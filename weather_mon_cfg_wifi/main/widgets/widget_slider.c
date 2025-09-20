#include "widgets.h"

// Private Function Prototypes
static void w_slider_event_cb( lv_event_t * e );

// Public Function Definitions

/**
 * @brief Create a slider widget with value labels
 * @param parent Parent Object. If NULL, the active screen will be used.
 * @param min Minimum slider value
 * @param max Maximum slider value
 * @param init Initial slider value
 * @param cb Callback function for slider events
 * @return Slider widget structure pointer. NULL on failure.
 */
w_slider_t * w_slider_create( lv_obj_t * parent,
                              int min, int max, int init,
                              w_slider_cb_t cb )
{
  if( parent == NULL )
  {
    // if no parent is provided, use the active screen
    parent = lv_screen_active();
  }

  // Allocate memory for the new slider widget
  w_slider_t * w_slider = (w_slider_t *) lv_malloc( sizeof(w_slider_t) );
  if( w_slider == NULL )
  {
    return NULL;
  }

  // styles
  static lv_style_t style_main;
  static lv_style_t style_indicator;
  static lv_style_t style_knob;
  static lv_style_t style_pressed_color;

  lv_style_init( &style_main );
  lv_style_set_bg_opa( &style_main, LV_OPA_COVER );
  lv_style_set_bg_color( &style_main, lv_color_hex3(0xbbb) );

  lv_style_init( &style_indicator );
  lv_style_set_bg_opa( &style_indicator, LV_OPA_COVER );
  lv_style_set_bg_color( &style_indicator, lv_palette_main(LV_PALETTE_CYAN) );

  lv_style_init( &style_knob );
  lv_style_set_bg_opa( &style_knob, LV_OPA_COVER );
  lv_style_set_bg_color( &style_knob, lv_palette_main(LV_PALETTE_CYAN) );
  lv_style_set_border_color( &style_knob, lv_palette_darken(LV_PALETTE_CYAN, 3) );
  lv_style_set_border_width( &style_knob, 2 );
  lv_style_set_pad_all( &style_knob, 5 );

  lv_style_init( &style_pressed_color );
  lv_style_set_bg_opa( &style_pressed_color, LV_OPA_COVER );
  lv_style_set_bg_color( &style_knob, lv_palette_darken(LV_PALETTE_CYAN, 2) );

  // assign callback function
  w_slider->callback = cb;

  // Create a container for the slider and its value label
  w_slider->cont = lv_obj_create( parent );
  lv_obj_set_size( w_slider->cont, 200, 50 );
  lv_obj_set_flex_flow( w_slider->cont, LV_FLEX_FLOW_ROW );
  lv_obj_set_flex_align(  w_slider->cont,
                          LV_FLEX_ALIGN_CENTER,
                          LV_FLEX_ALIGN_SPACE_EVENLY,
                          LV_FLEX_ALIGN_CENTER );
  lv_obj_set_style_pad_all( w_slider->cont, 5, 0 );

  lv_obj_center( w_slider->cont );

  // Create the slider
  w_slider->sld = lv_slider_create( w_slider->cont );
  lv_slider_set_range( w_slider->sld, min, max );
  lv_slider_set_value( w_slider->sld, init, LV_ANIM_OFF );
  lv_obj_set_width( w_slider->sld, LV_PCT(60) );
  lv_obj_add_event_cb( w_slider->sld, w_slider_event_cb, LV_EVENT_ALL, w_slider );
  
  // slider styles application is done below
  lv_obj_add_style( w_slider->sld, &style_main, LV_PART_MAIN );
  lv_obj_add_style( w_slider->sld, &style_indicator, LV_PART_INDICATOR );
  lv_obj_add_style( w_slider->sld, &style_knob, LV_PART_KNOB );
  lv_obj_add_style( w_slider->sld, &style_pressed_color, LV_PART_KNOB | LV_STATE_PRESSED );
  lv_obj_add_style( w_slider->sld, &style_pressed_color, LV_PART_INDICATOR | LV_STATE_PRESSED );

  // Create the value label
  w_slider->sld_val = lv_label_create( w_slider->cont );
  char buf[8];
  snprintf( buf, sizeof(buf), "%d", init );
  lv_label_set_text( w_slider->sld_val, buf );
  lv_obj_set_style_text_align( w_slider->sld_val, LV_TEXT_ALIGN_CENTER, 0 );
  lv_obj_set_width( w_slider->sld_val, LV_PCT(25) );

  return w_slider;
}

// Private Function Definitions

/**
 * @brief Callback function for slider events
 * @param e Event pointer
 */
static void w_slider_event_cb( lv_event_t * e )
{
  lv_event_code_t code = lv_event_get_code( e );
  lv_obj_t * sld = lv_event_get_target( e );
  w_slider_t * w_slider = (w_slider_t *) lv_event_get_user_data( e );

  if( code == LV_EVENT_VALUE_CHANGED )
  {
    // Update the value label
    int val = lv_slider_get_value( sld );
    char buf[8];
    snprintf( buf, sizeof(buf), "%d", val );
    lv_label_set_text( w_slider->sld_val, buf );
  }
  else if( code == LV_EVENT_DELETE )
  {
    // Free the memory when the slider is deleted
    lv_free( w_slider );
    w_slider = NULL;
  }

  // Call the user callback function if assigned
  if( w_slider->callback )
  {
    w_slider->callback( e );
  }
}
