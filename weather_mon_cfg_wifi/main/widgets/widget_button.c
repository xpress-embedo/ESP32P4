#include "widgets.h"

// Private Function Prototypes
static void w_button_event_cb( lv_event_t * e );

// Public Function Definitions
w_button_t * w_button_create( lv_obj_t * parent, const char * label, w_button_cb_t cb )
{
  if( parent == NULL )
  {
    // if no parent is provided, use the active screen
    parent = lv_screen_active();
  }

  // Allocate memory for the new button widget
  w_button_t * w_button = (w_button_t *) lv_malloc( sizeof(w_button_t) );
  if( w_button == NULL )
  {
    return NULL;
  }

  // assign callback function
  w_button->callback = cb;

  // Create the button
  w_button->btn = lv_btn_create( parent );
  lv_obj_set_size( w_button->btn, 100, 50 );
  lv_obj_set_align( w_button->btn, LV_ALIGN_CENTER );
  lv_obj_add_flag( w_button->btn, LV_OBJ_FLAG_SCROLL_ON_FOCUS );
  lv_obj_clear_flag( w_button->btn, LV_OBJ_FLAG_SCROLLABLE );
  lv_obj_set_style_radius( w_button->btn, 0, LV_PART_MAIN | LV_STATE_DEFAULT );
  lv_obj_set_style_bg_color( w_button->btn, lv_color_hex(0x808080), LV_PART_MAIN | LV_STATE_DEFAULT );
  lv_obj_set_style_bg_opa( w_button->btn, 255, LV_PART_MAIN | LV_STATE_DEFAULT );
  lv_obj_add_event_cb( w_button->btn, w_button_event_cb, LV_EVENT_ALL, w_button );

  // Create the button label
  w_button->btn_lbl = lv_label_create( w_button->btn );
  lv_label_set_text( w_button->btn_lbl, label );
  lv_obj_center( w_button->btn_lbl );

  return w_button;
}

// Private Function Definitions
static void w_button_event_cb( lv_event_t * e )
{
  lv_event_code_t code = lv_event_get_code( e );
  w_button_t * w_button = (w_button_t *)lv_event_get_user_data( e );

  // change button color when pressed
  if( code == LV_EVENT_PRESSED )
  {
    lv_obj_set_style_bg_color( w_button->btn, lv_color_hex(0x404040), LV_PART_MAIN | LV_STATE_DEFAULT );
  }
  else if( code == LV_EVENT_RELEASED || code == LV_EVENT_PRESS_LOST )
  {
    lv_obj_set_style_bg_color( w_button->btn, lv_color_hex(0x808080), LV_PART_MAIN | LV_STATE_DEFAULT );
  }

  // call user callback function if assigned
  if( w_button->callback != NULL )
  {
    w_button->callback( e );
  }
}


