/*
 * gui_menu_mng.c
 *
 *  Created on: 20-Sep-2025
 *      Author: abc@xyz
 */

#include "ui.h"
#include "lvgl.h"
#include "gui_menu_mng.h"

#include "esp_log.h"

// Private Macros

// Private Variables
static const char *TAG = "GUI_MENU";

// Private Function Prototypes
static void settings_event_cb( lv_event_t * e );

// Public Function Definitions
void gui_menu_mng_init( void )
{
  // register callback for settings image both settings-1 and settings-2
  lv_obj_add_event_cb( ui_imgSetting1, settings_event_cb, LV_EVENT_ALL, NULL );
  lv_obj_add_event_cb( ui_imgSetting1, NULL, LV_EVENT_ALL, NULL );
}

// Private Function Definitions
static void settings_event_cb( lv_event_t * e )
{
  lv_event_code_t code = lv_event_get_code( e );
  if ( code == LV_EVENT_CLICKED )
  {
    LV_LOG_USER( "Settings Image Clicked" );
    ESP_LOGI( TAG, "Settings Image Clicked" );
  }
}

