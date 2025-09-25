/*
 * gui_menu_mng.c
 *
 *  Created on: 20-Sep-2025
 *      Author: abc@xyz
 */

#include "ui.h"
#include "lvgl.h"
#include "widgets.h"
#include "gui_menu_mng.h"

#include "nvs_app.h"
#include "esp_log.h"
#include "wifi_app.h"     // included to get some macros definition, but make sure to not call any function directly
#include "main.h"
#include "bsp/esp-bsp.h"  // for display brightness and contrast functions

// Private Macros
// un-comment this macro to use Menu in sidebar
#define GUI_MENU_AS_SIDEBAR

// Private Variables
static const char *TAG = "GUI_MENU";
static lv_obj_t * menu = NULL;                  // object pointer for the whole menu
static lv_obj_t * kb_wifi_pswd = NULL;          // object pointer for the keyboard
static lv_obj_t * txt_box_wifi_pswd = NULL;     // object pointer for the text area to enter WiFi password
static lv_obj_t * cb_wifi  = NULL;              // object pointer for drop down box
static w_button_t * connect_btn;                // object pointer for connect button
static w_button_t * disconnect_btn;             // object pointer for disconnect button
static w_button_t * rescan_btn;                 // object pointer for re-scan button
static char wifi_ssid[WIFI_MAX_SSID_LEN] = { 0 };
static uint8_t disp_brightness = 50;             // default brightness
static uint8_t disp_contrast = 50;               // default contrast

// Enumerations
typedef enum
{
  LV_MENU_ITEM_BUILDER_VARIANT_1,
  LV_MENU_ITEM_BUILDER_VARIANT_2
} lv_menu_builder_variant_t;

// Function pointer typedefs
typedef void (*slider_callback_t)( int32_t value );

// Private Function Prototypes
static void menu_back_btn_event_cb( lv_event_t * e );
static void settings_menu_event_cb( lv_event_t * e );
static void create_menu_settings( void );
static void create_wifi_settings_page( lv_obj_t * parent );
static void generic_slider_event_cb( lv_event_t * e );
static void txt_box_wifi_pswd_event_cb( lv_event_t * e );
static void btn_connect_event_cb( lv_event_t * e );
static void btn_disconnect_event_cb( lv_event_t * e );
static void btn_rescan_event_cb( lv_event_t * e );
static void check_box_show_pswd_event_cb( lv_event_t * e );
static void display_brightness_event( int32_t value );
static void display_contrast_event( int32_t value );
static lv_obj_t * create_text( lv_obj_t * parent, const char * icon, const char * txt, lv_menu_builder_variant_t builder_variant );
static lv_obj_t * create_slider( lv_obj_t * parent, const char * icon, const char * txt, int32_t min, int32_t max, int32_t val, slider_callback_t cb );

// Public Function Definitions
void gui_menu_mng_init( void )
{
  // register callback for settings image both settings-1 and settings-2
  lv_obj_add_event_cb( ui_imgSetting, settings_menu_event_cb, LV_EVENT_ALL, NULL );

  // create menu
  create_menu_settings();
}

void gui_menu_mng_wifi_ap_available( char * wifi_ap_list )
{
  // ESP_LOGE( TAG, "%s", (char*)data );
  // setting image should be visible only when this function is called
  lv_obj_remove_flag( ui_imgSetting, LV_OBJ_FLAG_HIDDEN );
  // updating the drop down list
  lv_dropdown_set_options( cb_wifi, wifi_ap_list );
}

void gui_menu_mng_wifi_connected( void )
{
  // enable the connect button and disable the disconnect button
  lv_obj_clear_flag( connect_btn->btn, LV_OBJ_FLAG_CLICKABLE );
  lv_obj_add_state( connect_btn->btn, LV_STATE_DISABLED );

  lv_obj_add_flag( disconnect_btn->btn, LV_OBJ_FLAG_CLICKABLE );
  lv_obj_remove_state( disconnect_btn->btn, LV_STATE_DISABLED );
}

void gui_menu_mng_wifi_disconnected( void )
{
  // disable the disconnect button and enable the connect button
  lv_obj_clear_flag( disconnect_btn->btn, LV_OBJ_FLAG_CLICKABLE );
  lv_obj_add_state( disconnect_btn->btn, LV_STATE_DISABLED );

  lv_obj_add_flag( connect_btn->btn, LV_OBJ_FLAG_CLICKABLE );
  lv_obj_remove_state( connect_btn->btn, LV_STATE_DISABLED );
}

// Private Function Definitions
/**
 * @brief Callback function when back button is pressed on menu
 *        Once clicked the Menu is hidden and Main Screen is Displayed
 * @param e pointer to event
 */
static void menu_back_btn_event_cb( lv_event_t * e )
{
  lv_event_code_t code = lv_event_get_code( e );
  if ( code == LV_EVENT_CLICKED )
  {
    // no need to check if hidden then show and vice-versa, because showing and
    // hiding is done using separate logics
    lv_obj_add_flag( menu, LV_OBJ_FLAG_HIDDEN );
    ESP_LOGI( TAG, "Menu Back Button Clicked" );
  }
}

/**
 * @brief Callback function when settings image is pressed
 *        Based on this the menu setting will be visible to the user
 * @param e pointer to event
 */
static void settings_menu_event_cb( lv_event_t * e )
{
  lv_event_code_t code = lv_event_get_code( e );
  if ( code == LV_EVENT_CLICKED )
  {
    // no need to check if hidden then show and vice-versa, because showing and
    // hiding is done using separate logics
    lv_obj_remove_flag( menu, LV_OBJ_FLAG_HIDDEN );
    ESP_LOGI( TAG, "Settings Image Clicked" );
  }
}

/**
 * Creates and displays a comprehensive menu on the LVGL interface.
 *
 * This function initializes a menu that allows users to control display settings
 * such as brightness and contrast, as well as select and switch between available WiFi networks.
 * It demonstrates how to set up menu components, add interactive items for display control,
 * and provide options for WiFi connectivity management.
 *
 * @param none
 */
static void create_menu_settings( void )
{
  lv_obj_t * header = NULL;
  lv_obj_t * title_label = NULL;
  
  // TODO: I think it will be better to use main screen instead of lv_screen_active
  menu = lv_menu_create( lv_screen_active() );
  lv_menu_set_mode_root_back_button( menu, LV_MENU_ROOT_BACK_BUTTON_ENABLED );
  // configure back button event handler later
  lv_obj_set_size( menu, lv_display_get_horizontal_resolution(NULL), lv_display_get_vertical_resolution(NULL) );
  lv_obj_center( menu );
  lv_obj_set_scrollbar_mode( menu, LV_SCROLLBAR_MODE_OFF );

  // Display Sub Page
  lv_obj_t * disp_page = lv_menu_page_create( menu, "Dislay Settings" );
  lv_menu_separator_create( disp_page );
  lv_obj_t * disp_section = lv_menu_section_create( disp_page );
  // before creating the slider, load the saved brightness and contrast values
  nvs_app_load_display_information( &disp_brightness, &disp_contrast );
  // if someone accidently saved wrong values, then reset to default
  if ( (disp_brightness > 100) || (disp_brightness < 10) )
  {
    disp_brightness = 50;
  }
  // set the display brightness to saved value
  bsp_display_brightness_set( disp_brightness );
  // contrast is not implemented in BSP, so just clamp the value
  if ( (disp_contrast > 100) || (disp_contrast < 10) )
  {
    disp_contrast = 50;
  }
  create_slider( disp_section, LV_SYMBOL_SETTINGS, "Brightness", 0, 100, disp_brightness, display_brightness_event );
  create_slider( disp_section, LV_SYMBOL_SETTINGS, "Contrast", 0, 100, disp_contrast, display_contrast_event );

  // WiFi Sub Page
  lv_obj_t * wifi_page = lv_menu_page_create( menu, "WiFi Settings" );
  create_wifi_settings_page( wifi_page );
  // lv_menu_separator_create( wifi_page );
  // lv_obj_t * wifi_section = lv_menu_section_create( wifi_page );
  // lv_obj_t * wifi_label = lv_label_create( wifi_section );
  // lv_label_set_text( wifi_label, "WiFi Setting Coming Soon..." );

  // Create Main Page
  lv_obj_t * main_page = lv_menu_page_create( menu, "Settings" );
  // creating sections instead of containers to have better padding and spacing
  lv_obj_t * btn_disp = lv_menu_section_create( main_page );
  // alternatively, create_text() can be used
  create_text( btn_disp, LV_SYMBOL_SETTINGS, "Display", LV_MENU_ITEM_BUILDER_VARIANT_1 );
  // lv_obj_t * label = lv_label_create( btn_disp );
  // lv_label_set_text( label, "Display" );
  lv_menu_set_load_page_event( menu, btn_disp, disp_page );

  // creating sections instead of containers to have better padding and spacing
  lv_obj_t * btn_wifi = lv_menu_section_create( main_page );
  create_text( btn_wifi, LV_SYMBOL_WIFI, "WiFi", LV_MENU_ITEM_BUILDER_VARIANT_1 );
  // lv_obj_t * lbl_wifi = lv_label_create( btn_wifi );
  // lv_label_set_text(lbl_wifi, "WiFi");
  lv_menu_set_load_page_event( menu, btn_wifi, wifi_page );

  // Increase the size of all headers
  // NOTE: This is increasing the size of all headers by same font, need to find
  // a way to control each menu header independently
  header = lv_menu_get_main_header( menu );
  // Assuming the title label is the first child of the header
  title_label = lv_obj_get_child_by_type( header, 0, &lv_label_class );
  // Apply font style
  lv_obj_set_style_text_font( title_label, &lv_font_montserrat_24, LV_PART_MAIN | LV_STATE_DEFAULT );

  // set the page
#ifdef GUI_MENU_AS_SIDEBAR
  lv_menu_set_sidebar_page( menu, main_page );
  lv_obj_t * sidebar_header = lv_menu_get_sidebar_header( menu );
  lv_obj_t * sidebar_title_label = lv_obj_get_child_by_type( sidebar_header, 0, &lv_label_class );
  lv_obj_set_style_text_font( sidebar_title_label, &lv_font_montserrat_24, LV_PART_MAIN | LV_STATE_DEFAULT );

  lv_obj_t * sidebar_back_button = lv_obj_get_child_by_type( sidebar_header, 0, &lv_button_class );
  if ( sidebar_back_button )
  {
    lv_obj_add_event_cb( sidebar_back_button, menu_back_btn_event_cb, LV_EVENT_CLICKED, NULL );
    ESP_LOGI( TAG, "Button Found and Event Registered SIDEBAR" );
    // Updating the image
    lv_obj_t * sidebar_back_btn_img = lv_obj_get_child_by_type( sidebar_back_button, 0, &lv_image_class );
    if ( sidebar_back_btn_img )
    {
      lv_image_set_src( sidebar_back_btn_img, &ui_img_setting_50px_png );
    }
  }
#else
  lv_menu_set_page( menu, main_page);
  // add callback for back button
  // NOTE: the issue with the below statement is that it is attaching the callback
  // to the entire menu object, including empty spaces, and this triggers the events
  // lv_obj_add_event_cb( menu, menu_back_btn_event_cb, LV_EVENT_CLICKED, NULL );
  // What I need to do is trigger callback event for "> Settings"
  header = lv_menu_get_main_header( menu );
  lv_obj_t * button = lv_obj_get_child_by_type( header, 0, &lv_button_class );
  if ( button )
  {
    // Found the back button
    lv_obj_add_event_cb( button, menu_back_btn_event_cb, LV_EVENT_CLICKED, NULL );
    ESP_LOGI( TAG, "Button Found and Event Registered" );
    break;
  }
#endif

  // menu is hidden by default, and will be visible when setting icon is clicked
  lv_obj_add_flag( menu, LV_OBJ_FLAG_HIDDEN );
  // in case we want to hide the page
  // lv_obj_add_flag( main_page, LV_OBJ_FLAG_HIDDEN );

  /* TODO: Styling for future
  static lv_style_t style_menu_dark;
  lv_style_init(&style_menu_dark);
  lv_style_set_bg_color(&style_menu_dark, lv_color_make(30, 30, 30));  // Dark gray
  lv_style_set_bg_opa(&style_menu_dark, LV_OPA_COVER);
  lv_style_set_border_width(&style_menu_dark, 0);
  lv_style_set_pad_all(&style_menu_dark, 8);

  lv_obj_add_style(menu, &style_menu_dark, LV_PART_MAIN | LV_STATE_DEFAULT);
  header = lv_menu_get_main_header(menu);
  lv_obj_add_style(header, &style_menu_dark, LV_PART_MAIN);
  
  lv_obj_t * sidebar = lv_menu_get_sidebar_header(menu);
  lv_obj_add_style(sidebar, &style_menu_dark, LV_PART_MAIN);

  lv_style_set_text_color(&style_menu_dark, lv_color_white());
  lv_style_set_text_font(&style_menu_dark, &lv_font_montserrat_20);
  */

  // this function will update the button styling for disconnected state
  gui_menu_mng_wifi_disconnected();
}

/**
 * @brief Create WiFi Settings Page
 * @param parent Parent Pointer to this Page
 */
static void create_wifi_settings_page( lv_obj_t * parent )
{
  // create main container
  lv_obj_t * main_cont = lv_obj_create( parent );
  // remove default styles
  lv_obj_remove_style_all( main_cont );
  lv_obj_set_width( main_cont, LV_PCT(100) );
  lv_obj_set_height( main_cont, LV_PCT(100) );
  lv_obj_set_align( main_cont, LV_ALIGN_CENTER );
  lv_obj_set_flex_flow( main_cont, LV_FLEX_FLOW_COLUMN );
  lv_obj_set_flex_align( main_cont, LV_FLEX_ALIGN_CENTER, LV_FLEX_ALIGN_CENTER, LV_FLEX_ALIGN_CENTER );
  lv_obj_remove_flag( main_cont, LV_OBJ_FLAG_CLICKABLE | LV_OBJ_FLAG_SCROLLABLE );
  lv_obj_set_style_pad_left( main_cont, 5, LV_PART_MAIN | LV_STATE_DEFAULT );
  lv_obj_set_style_pad_right( main_cont, 5, LV_PART_MAIN | LV_STATE_DEFAULT );
  lv_obj_set_style_pad_top( main_cont, 5, LV_PART_MAIN | LV_STATE_DEFAULT );
  lv_obj_set_style_pad_bottom( main_cont, 5, LV_PART_MAIN | LV_STATE_DEFAULT );

  // create 4 containers inside main container
  // 1. WiFi SSID Text and WiFi List
  lv_obj_t * cont_ssid = lv_obj_create( main_cont );
  lv_obj_remove_style_all( cont_ssid );
  lv_obj_set_width( cont_ssid, LV_PCT(100) );
  lv_obj_set_height( cont_ssid, LV_PCT(20) );
  lv_obj_set_align( cont_ssid, LV_ALIGN_TOP_MID );
  lv_obj_set_flex_flow( cont_ssid, LV_FLEX_FLOW_ROW_WRAP );
  lv_obj_set_flex_align( cont_ssid, LV_FLEX_ALIGN_SPACE_EVENLY, LV_FLEX_ALIGN_CENTER, LV_FLEX_ALIGN_SPACE_EVENLY );
  lv_obj_remove_flag( cont_ssid, LV_OBJ_FLAG_CLICKABLE | LV_OBJ_FLAG_SCROLLABLE );

  lv_obj_t * lbl_wifi = lv_label_create( cont_ssid );
  lv_obj_set_width( lbl_wifi, LV_SIZE_CONTENT );
  lv_obj_set_height( lbl_wifi, LV_SIZE_CONTENT );
  lv_obj_set_align( lbl_wifi, LV_ALIGN_CENTER );
  lv_label_set_text( lbl_wifi, "WiFi SSID" );
  lv_obj_set_style_text_font( lbl_wifi, &lv_font_montserrat_18, LV_PART_MAIN | LV_STATE_DEFAULT );

  cb_wifi = lv_dropdown_create( cont_ssid );
  lv_dropdown_set_options( cb_wifi, "FIRST\nSECOND\nGROUND\nFIRST_EXT\n" );
  lv_obj_set_width( cb_wifi, 250 );
  lv_obj_set_height( cb_wifi, 50 );
  lv_obj_set_align( cb_wifi, LV_ALIGN_CENTER );
  lv_obj_add_flag( cb_wifi, LV_OBJ_FLAG_SCROLL_ON_FOCUS );
  lv_obj_set_style_text_font( cb_wifi, &lv_font_montserrat_18, LV_PART_MAIN | LV_STATE_DEFAULT );

  // 2. WiFi Password Test and Password Input
  lv_obj_t * cont_pswd = lv_obj_create( main_cont );
  lv_obj_remove_style_all( cont_pswd );
  lv_obj_set_width( cont_pswd, LV_PCT(100) );
  lv_obj_set_height( cont_pswd, LV_PCT(20) );
  lv_obj_set_align( cont_pswd, LV_ALIGN_TOP_MID );
  lv_obj_set_flex_flow( cont_pswd, LV_FLEX_FLOW_ROW_WRAP );
  lv_obj_set_flex_align( cont_pswd, LV_FLEX_ALIGN_SPACE_EVENLY, LV_FLEX_ALIGN_CENTER, LV_FLEX_ALIGN_SPACE_EVENLY );
  lv_obj_remove_flag( cont_pswd, LV_OBJ_FLAG_CLICKABLE | LV_OBJ_FLAG_SCROLLABLE );

  lv_obj_t * lbl_wifi_pswd = lv_label_create( cont_pswd );
  lv_obj_set_width( lbl_wifi_pswd, LV_SIZE_CONTENT );
  lv_obj_set_height( lbl_wifi_pswd, LV_SIZE_CONTENT );
  lv_obj_set_align( lbl_wifi_pswd, LV_ALIGN_CENTER );
  lv_label_set_text( lbl_wifi_pswd, "Password" );
  lv_obj_set_style_text_font( lbl_wifi_pswd, &lv_font_montserrat_18, LV_PART_MAIN | LV_STATE_DEFAULT );

  txt_box_wifi_pswd = lv_textarea_create( cont_pswd );
  lv_obj_set_width( txt_box_wifi_pswd, 250 );
  lv_obj_set_height( txt_box_wifi_pswd, 50 );
  lv_obj_set_align( txt_box_wifi_pswd, LV_ALIGN_CENTER );
  lv_textarea_set_placeholder_text( txt_box_wifi_pswd, "Enter Password" );
  lv_textarea_set_password_mode( txt_box_wifi_pswd, true );
  lv_textarea_set_max_length( txt_box_wifi_pswd, 64 );

  lv_obj_t * check_box_show_pswd = lv_checkbox_create( cont_pswd );
  lv_checkbox_set_text( check_box_show_pswd, "Show Password" );
  lv_obj_set_width( check_box_show_pswd, LV_SIZE_CONTENT );
  lv_obj_set_height( check_box_show_pswd, LV_SIZE_CONTENT );
  lv_obj_set_align( check_box_show_pswd, LV_ALIGN_CENTER );
  // we may need to tune this again, based on the display size
  #ifdef GUI_MENU_AS_SIDEBAR
  lv_obj_set_x( check_box_show_pswd, 35 );
  lv_obj_set_y( check_box_show_pswd, 35 );
  #else
  lv_obj_set_x( check_box_show_pswd, 10 );
  lv_obj_set_y( check_box_show_pswd, 35 );
  #endif
  // here the flag LV_OBJ_FLAG_IGNORE_LAYOUT is added to avoid the checkbox being squished in the flex layout
  // else whole layout will be broken
  lv_obj_add_flag( check_box_show_pswd, LV_OBJ_FLAG_IGNORE_LAYOUT | LV_OBJ_FLAG_OVERFLOW_VISIBLE | LV_OBJ_FLAG_SCROLL_ON_FOCUS );
  lv_obj_set_style_text_font( check_box_show_pswd, &lv_font_montserrat_10, LV_PART_MAIN | LV_STATE_DEFAULT );
  lv_obj_set_style_border_color( check_box_show_pswd, lv_color_hex(0x808080), LV_PART_INDICATOR | LV_STATE_DEFAULT );
  lv_obj_set_style_border_color( check_box_show_pswd, lv_color_hex(0x808080), LV_PART_INDICATOR | LV_STATE_CHECKED );
  lv_obj_set_style_bg_color( check_box_show_pswd, lv_color_hex(0x808080), LV_PART_INDICATOR | LV_STATE_CHECKED );
  lv_obj_set_style_border_opa( check_box_show_pswd, 255, LV_PART_INDICATOR | LV_STATE_DEFAULT );

  // add event callback to the checkbox
  lv_obj_add_event_cb( check_box_show_pswd, check_box_show_pswd_event_cb, LV_EVENT_ALL, NULL );

  // 3. Button Row (Connect, Disconnect, Re-Scan)
  lv_obj_t * cont_btn = lv_obj_create( main_cont );
  lv_obj_remove_style_all( cont_btn );
  lv_obj_set_width( cont_btn, LV_PCT(100) );
  lv_obj_set_height( cont_btn, LV_PCT(20) );
  lv_obj_set_align( cont_btn, LV_ALIGN_TOP_MID );
  lv_obj_set_flex_flow( cont_btn, LV_FLEX_FLOW_ROW_WRAP );
  lv_obj_set_flex_align( cont_btn, LV_FLEX_ALIGN_SPACE_EVENLY, LV_FLEX_ALIGN_CENTER, LV_FLEX_ALIGN_SPACE_EVENLY );
  lv_obj_remove_flag( cont_btn, LV_OBJ_FLAG_CLICKABLE | LV_OBJ_FLAG_SCROLLABLE );

  connect_btn = w_button_create( cont_btn, "Connect", btn_connect_event_cb );
  disconnect_btn = w_button_create( cont_btn, "Disconnect", btn_disconnect_event_cb );
  rescan_btn = w_button_create( cont_btn, "Re Scan", btn_rescan_event_cb );

  // 4. Keyboard (hidden by default, shown when text area is focused)
  kb_wifi_pswd = lv_keyboard_create( main_cont );
  lv_obj_set_width( kb_wifi_pswd, LV_PCT(100) );
  lv_obj_set_height( kb_wifi_pswd, LV_PCT(40) );
  lv_obj_set_align( kb_wifi_pswd, LV_ALIGN_CENTER );

  // Assign a Text Area to the Keyboard. The pressed characters will be put there.
  lv_keyboard_set_textarea( kb_wifi_pswd, txt_box_wifi_pswd );
  lv_obj_add_event_cb( txt_box_wifi_pswd, txt_box_wifi_pswd_event_cb, LV_EVENT_ALL, NULL );
  // Hide the keyboard by default
  // lv_obj_add_flag( kb_wifi_pswd, LV_OBJ_FLAG_HIDDEN );     // I don't like the idea of hiding in my use-case so commented for now
}

/**
 * @brief Slider Callback Function
 *        This function will call the respective configured callback function
 * @param e Pointer to LVGL Event
 */
static void generic_slider_event_cb( lv_event_t * e )
{
  lv_event_code_t code = lv_event_get_code( e );
  if ( code == LV_EVENT_VALUE_CHANGED )
  {
    // handle value change
    lv_obj_t * slider = lv_event_get_target( e );
    int32_t val = lv_slider_get_value( slider );
    // retrieve the callback function pointer
    slider_callback_t cb = (slider_callback_t)lv_event_get_user_data( e );
    if ( cb )
    {
      cb( val );
    }
  }
  else if ( code == LV_EVENT_RELEASED )
  {
    // TODO: not the correct way to do this, need to find a better way
    // save the brightness value to NVS
    ESP_LOGW( TAG, "Saving Brightness: %d, Contrast: %d to NVS", disp_brightness, disp_contrast );
    nvs_app_save_display_information( disp_brightness, disp_contrast );
  }
}

/**
 * @brief WiFi Password Text Area Callback Function
 *        This is used to show and hide the keyboard
 * @param e Pointer to LVGL Event
 */
static void txt_box_wifi_pswd_event_cb( lv_event_t * e )
{
  #if 0
  NOTE: I did not liked the idea of hiding Keyboard in my use case, hence 
  commenting this function for now
  lv_event_code_t code = lv_event_get_code( e );
  lv_obj_t * target = lv_event_get_target( e );
  switch ( code )
  {
  case LV_EVENT_FOCUSED:
  case LV_EVENT_CLICKED:
    // show the keyboard
    // link the keyboard to the text area (in case not already done)
    lv_keyboard_set_textarea( kb_wifi_pswd, target );
    lv_obj_remove_flag( kb_wifi_pswd, LV_OBJ_FLAG_HIDDEN );
    break;
  case LV_EVENT_DEFOCUSED:
  case LV_EVENT_READY:
    // hide the keyboard
    // Unlink the keyboard from the text area to avoid further input
    lv_keyboard_set_textarea( kb_wifi_pswd, NULL );
    lv_obj_add_flag( kb_wifi_pswd, LV_OBJ_FLAG_HIDDEN );
    break;
  default:
    break;
  }
  #endif
}

/**
 * @brief Connect Button Callback
 * @param e Pointer to LVGL Event
 */
static void btn_connect_event_cb( lv_event_t * e )
{
  lv_event_code_t code = lv_event_get_code( e );
  if ( code == LV_EVENT_CLICKED )
  {
    // lv_obj_t * btn = lv_event_get_target( e );
    ESP_LOGI( TAG, "Connect Button Clicked" );
    lv_dropdown_get_selected_str( cb_wifi, wifi_ssid, WIFI_MAX_SSID_LEN );
    // wifi_sta_start_connecting( wifi_ssid, lv_textarea_get_text(txt_box_wifi_pswd) );
    // Update the WiFi Network Configuration and let the WiFi Application Know
    wifi_config_t * wifi_config_ptr = wifi_app_get_wifi_config();
    // If no memory is allocated to wifi_config then we can Store Access Fault
    if ( !wifi_config_ptr )
    {
      ESP_LOGE(TAG, "wifi_config is NULL!");
    }
    else
    {
      memset( wifi_config_ptr, 0x00, sizeof(wifi_config_t) );
      strncpy( (char*)wifi_config_ptr->sta.ssid, wifi_ssid, sizeof(wifi_config_ptr->sta.ssid)-1 );
      strncpy( (char*)wifi_config_ptr->sta.password, lv_textarea_get_text(txt_box_wifi_pswd), sizeof(wifi_config_ptr->sta.password)-1 );
      ESP_LOGI( TAG, "WiFi Config Updated: SSID: %s, Password: %s", wifi_config_ptr->sta.ssid, wifi_config_ptr->sta.password );
      // send this event to main module and it will handle the connection
      main_send_event( MAIN_EV_GUI_REQ_USER_CONNECT, NULL );
    }
  }
}

/**
 * @brief Diconnect Button Callback
 * @param e Pointer to LVGL Event
 */
static void btn_disconnect_event_cb( lv_event_t * e )
{
  lv_event_code_t code = lv_event_get_code( e );
  if ( code == LV_EVENT_CLICKED )
  {
    // lv_obj_t * btn = lv_event_get_target( e );
    ESP_LOGI( TAG, "Disconnect Button Clicked" );
    // send this event to main module and it will handle the disconnection
    main_send_event( MAIN_EV_GUI_REQ_USER_DISCONNECT, NULL );
  }
}

/**
 * @brief Diconnect Button Callback
 * @param e Pointer to LVGL Event
 */
static void btn_rescan_event_cb( lv_event_t * e )
{
  lv_event_code_t code = lv_event_get_code( e );
  if ( code == LV_EVENT_CLICKED )
  {
    // lv_obj_t * btn = lv_event_get_target( e );
    ESP_LOGI( TAG, "Re-Scan Button Clicked" );
    // handle re-scan button click
  }
}

/**
 * @brief Rescan Button Callback
 * @param e Pointer to LVGL Event
 */
static void check_box_show_pswd_event_cb( lv_event_t * e )
{
  lv_event_code_t code = lv_event_get_code( e );
  if ( code == LV_EVENT_VALUE_CHANGED )
  {
    lv_obj_t * cb = lv_event_get_target( e );
    bool checked = lv_obj_has_state( cb, LV_STATE_CHECKED );
    ESP_LOGI( TAG, "Show Password Checkbox: %s", checked ? "Checked" : "Unchecked" );
    if ( txt_box_wifi_pswd != NULL )
    {
      lv_textarea_set_password_mode( txt_box_wifi_pswd, !checked );
    }
  }
}

/**
 * @brief Change Display Brightness Callback Function
 * @param value 
 */
static void display_brightness_event( int32_t value )
{
  ESP_LOGI( TAG, "Display Brightness: %d", value);
  // clamp the value between 10 and 100
  value = value < 10 ? 10 : (value > 100 ? 100 : value);
  disp_brightness = (uint8_t)value;
  bsp_display_brightness_set( value );
}

/**
 * @brief Change Display Contrast Callback Function
 * @param value 
 */
static void display_contrast_event( int32_t value )
{
  ESP_LOGI( TAG, "Display Contrast: %d", value);
  // clamp the value between 10 and 100
  value = value < 10 ? 10 : (value > 100 ? 100 : value);
  disp_contrast = (uint8_t)value;
}

/**
 * @brief Create Text and Layout for Menu Screen
 * @param parent parent of this child
 * @param icon icon to display
 * @param txt text to display
 * @param builder_variant select this LV_MENU_ITEM_BUILDER_VARIANT_1 or LV_MENU_ITEM_BUILDER_VARIANT_2
 * @return container object
 */
static lv_obj_t * create_text( lv_obj_t * parent, const char * icon, const char * txt, lv_menu_builder_variant_t builder_variant )
{
  lv_obj_t * obj = lv_menu_cont_create( parent );
  lv_obj_t * img = NULL;
  lv_obj_t * label = NULL;

  // if icon is provided, create and set it
  if ( icon )
  {
    img = lv_image_create( obj );
    lv_image_set_src( img, icon );
  }

  // if text is provided, create and set it
  if ( txt )
  {
    label = lv_label_create( obj );
    lv_label_set_text( label, txt );
    lv_label_set_long_mode( label, LV_LABEL_LONG_SCROLL_CIRCULAR );
    if ( builder_variant == LV_MENU_ITEM_BUILDER_VARIANT_1 )
    {
      // Need to increase the font size
      lv_obj_set_style_text_font( label, &lv_font_montserrat_20, LV_PART_MAIN | LV_STATE_DEFAULT );
    }
    lv_obj_set_flex_grow( label, 1 );
  }

  // if both icon and text are provided, make the icon start a new row and swap their order
  // so that the text appears first followed by the icon
  if ( (builder_variant == LV_MENU_ITEM_BUILDER_VARIANT_2) && icon && txt )
  {
    lv_obj_add_flag( img, LV_OBJ_FLAG_FLEX_IN_NEW_TRACK );
    lv_obj_swap( img, label );
  }

  return obj;
}

/**
 * @brief Generate Slider for Menu Screen
 * @param parent Parent of this child
 * @param icon Icon to display
 * @param txt Text to display
 * @param min Minimum Value of Slider
 * @param max Maximum Value of Slider
 * @param val Initial Value of Slider
 * @param cb Callback function when Slider value is changed
 * @return container object
 */
static lv_obj_t * create_slider( lv_obj_t * parent, const char * icon, const char * txt, int32_t min, int32_t max, int32_t val, slider_callback_t cb )
{
  lv_obj_t * obj = create_text( parent, icon, txt, LV_MENU_ITEM_BUILDER_VARIANT_2 );

  lv_obj_t * slider = lv_slider_create( obj );
  lv_obj_set_flex_grow( slider, 1 );
  lv_slider_set_range( slider, min, max );
  lv_slider_set_value( slider, val, LV_ANIM_OFF );

  lv_obj_set_style_radius( slider, 0, LV_PART_MAIN | LV_STATE_DEFAULT );
  lv_obj_set_style_bg_color( slider, lv_color_hex(0xD3D3D3), LV_PART_MAIN | LV_STATE_DEFAULT );
  lv_obj_set_style_bg_opa( slider, 255, LV_PART_MAIN | LV_STATE_DEFAULT );

  lv_obj_set_style_bg_color( slider, lv_color_hex(0x696969), LV_PART_INDICATOR | LV_STATE_DEFAULT );
  lv_obj_set_style_bg_opa( slider, 255, LV_PART_INDICATOR | LV_STATE_DEFAULT );

  lv_obj_set_style_bg_color( slider, lv_color_hex(0x708090), LV_PART_KNOB | LV_STATE_DEFAULT );
  lv_obj_set_style_bg_opa( slider, 255, LV_PART_KNOB | LV_STATE_DEFAULT );

  lv_obj_add_event_cb( slider, generic_slider_event_cb, LV_EVENT_ALL, (void *)cb );

  lv_obj_clear_flag( slider, LV_OBJ_FLAG_GESTURE_BUBBLE );

  // if no icon is provided, make the slider start in a new row
  if ( icon == NULL )
  {
    lv_obj_add_flag( slider, LV_OBJ_FLAG_FLEX_IN_NEW_TRACK );
  }

  return obj;
}
