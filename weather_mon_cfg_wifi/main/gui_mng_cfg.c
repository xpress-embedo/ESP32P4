/*
 * gui_mng_cfg.c
 *
 *  Created on: 06-Sep-2025
 *      Author: abc@xyz
 */
 
#include "gui_mng.h"
#include "gui_mng_cfg.h"
#include "gui_menu_mng.h"

#include "esp_log.h"

#include "freertos/FreeRTOS.h"
#include "freertos/task.h"

#include "main.h"
#include "lvgl.h"
#include "ui.h"
#include "assets_mng.h"

// Private Macros
#define NUM_ELEMENTS(x)                 (sizeof(x)/sizeof(x[0]))
#define LOAD_SENSOR_SCREEN_TIMER        ( 2000/ GUI_MNG_REFRESH_TIME )
#define SENSOR_ARC_ANIMATION_TIMER      ( 1000/ GUI_MNG_REFRESH_TIME )

// function template for callback function
typedef void (*gui_mng_callback)(void * data);

// structure
typedef struct _gui_mng_event_cb_t
{
  gui_mng_event_t   event;
  gui_mng_callback  callback;
} gui_mng_event_cb_t;

// Private Function Prototypes
// Private Function Prototypes
static void gui_wifi_connecting( void *data );
static void gui_wifi_connected( void *data );
static void gui_wifi_internet_connected( void *data );
static void gui_wifi_disconnected( void *data );
static void gui_load_sensor_screen( void *data );
static void gui_update_sensor_data( void *data );
static void gui_sensor_arc_animation( void *data );

// Private Variables
static const char *TAG = "GUI_CFG";
static uint32_t load_sensor_screen_timer = 0;
static uint32_t sensor_arc_animation_timer = 0;
static bool temp_ovf = false;
static bool humidity_ovf = false;

// This will be loaded from the littlefatfs file system
LV_IMG_DECLARE(ui_img_logo1_png);
static lv_img_dsc_t img_logo_dsc;       // main logo image descriptor

// Events and Callback Mapping Table
static const gui_mng_event_cb_t gui_mng_event_cb[] =
{
  { GUI_MNG_EV_WIFI_CONNECTING,         gui_wifi_connecting           },
  { GUI_MNG_EV_WIFI_CONNECTED,          gui_wifi_connected            },
  { GUI_MNG_EV_WIFI_DISCONNECTED,       gui_wifi_disconnected         },
  { GUI_MNG_EV_WIFI_INTERNET_CONNECTED, gui_wifi_internet_connected   },
  { GUI_MNG_EV_LOAD_SENSOR_SCREEN,      gui_load_sensor_screen        },
  { GUI_MNG_EV_TEMP_HUMID,              gui_update_sensor_data        },
  { GUI_MNG_EV_TEMP_HUMID_ARC_ANIM,     gui_sensor_arc_animation      },
};

// Public Function Definitions
/**
 * @brief GUI Configurable Initialization Function
 * @param  None
 */
void gui_cfg_init( void )
{
  // mount the littlefs file system which contains project assets
  assets_mng_init();

  ESP_LOGI( TAG, "UI Init. Starts" );
  GUI_LOCK();
  ui_init();
  if ( assets_mng_load_image("/assets/logo.bin", &img_logo_dsc) )
  {
    lv_img_set_src( ui_imgLogo, &img_logo_dsc);
  }
  else
  {
    ESP_LOGE(TAG, "Failed to load image");
  }
  // Menu related code can't be generated using Square Line Studio, this is written
  // by me by referencing LVGL examples, and it starts from here
  gui_menu_mng_init();  
  GUI_UNLOCK();
  ESP_LOGI( TAG, "UI Init. Ends" );
}

/**
 * @brief Process the events posted to GUI manager module
 *        This function calls the dedicated function based on the event posted
 *        to GUI manager queue, I will think of moving this function to GUI manager
 * @param event event name
 * @param data event data pointer
 */
void gui_cfg_mng_process( gui_mng_event_t event, void *data )
{
  uint8_t idx = 0;
  for( idx=0; idx < NUM_ELEMENTS(gui_mng_event_cb); idx++ )
  {
    // check if event matches the table
    if( event == gui_mng_event_cb[idx].event )
    {
      // call the callback function with arguments, if not NULL
      if( gui_mng_event_cb[idx].callback != NULL )
      {
        gui_mng_event_cb[idx].callback(data);
      }
    }
  }
}

/**
 * @brief this is a custom refresh function called periodically by GUI manager
 *        in this function we can write our code which can be called periodically
 */
void gui_cfg_refresh( void )
{
  if( (load_sensor_screen_timer > 0) && (--load_sensor_screen_timer == 0) )
  {
    ESP_LOGI( TAG, "Loading Sensor Screen Now" );
    gui_send_event( GUI_MNG_EV_LOAD_SENSOR_SCREEN, NULL );
  }
  
  if( (sensor_arc_animation_timer > 0) && (--sensor_arc_animation_timer == 0) )
  {
    // ESP_LOGI( TAG, "Reloading ARC Animation Timer" );
    sensor_arc_animation_timer = SENSOR_ARC_ANIMATION_TIMER;
    if( temp_ovf || humidity_ovf )
    {
      // send event
      gui_send_event( GUI_MNG_EV_TEMP_HUMID_ARC_ANIM, NULL );
      // ESP_LOGI( TAG, "Sending Event: GUI_MNG_EV_TEMP_HUMID_ARC_ANIM" );
    }
  }
}

// Private Function Definitions
/**
 * @brief Callback function when ESP32 is connecting to WiFi router
 * @param data 
 */
static void gui_wifi_connecting( void *data )
{
  GUI_LOCK();
  // update the connect icon status to disconnected
  lv_img_set_src( ui_imgWiFiStatus1,  &ui_img_wifi_disconnected_png );
  lv_img_set_src( ui_imgWiFiStatus2,  &ui_img_wifi_disconnected_png );
  GUI_UNLOCK();
  ESP_LOGI( TAG, "gui_wifi_connecting" );
}

/**
 * @brief Callback function when ESP32 is connected to Router
 * @param data 
 */
static void gui_wifi_connected( void *data )
{
  GUI_LOCK();
  // update the connect icon status to wifi connected with no internet
  lv_img_set_src( ui_imgWiFiStatus1,  &ui_img_wifi_png );
  lv_img_set_src( ui_imgWiFiStatus2,  &ui_img_wifi_png );
  GUI_UNLOCK();
  ESP_LOGI( TAG, "gui_wifi_connected" );
}

/**
 * @brief Callback function when ESP32 is connected to router and also get time 
          from SNTP
 * @param data
 */
static void gui_wifi_internet_connected( void *data )
{
  GUI_LOCK();
  // update the connect icon status to connected
  lv_img_set_src( ui_imgWiFiStatus1,  &ui_img_wifi_connected_png );
  lv_img_set_src( ui_imgWiFiStatus2,  &ui_img_wifi_connected_png );
  GUI_UNLOCK();

  // this will be used to post another event to load sensor screen
  // load_sensor_screen_timer = LOAD_SENSOR_SCREEN_TIMER;
  ESP_LOGI( TAG, "gui_wifi_internet_connected" );
  ESP_LOGW( TAG, "Time Starts to Load Next Screen ");
}

/**
 * @brief Callback function when WiFi is disconnected (Disconnect from Router & Influx Server)
 * @param data
 */
static void gui_wifi_disconnected( void *data )
{
  GUI_LOCK();
  // update the connect icon status to disconnected
  lv_img_set_src( ui_imgWiFiStatus1,  &ui_img_wifi_disconnected_png );
  lv_img_set_src( ui_imgWiFiStatus2,  &ui_img_wifi_disconnected_png );
  GUI_UNLOCK();
  ESP_LOGI( TAG, "gui_wifi_disconnected" );
}

/**
 * @brief Load the Sensor screen where temperature and humidity will be displayed
 * @param data 
 */
static void gui_load_sensor_screen( void *data )
{
  ESP_LOGI( TAG, "gui_load_sensor_screen" );
  // ui_Screen2_screen_init();
  if( ui_Screen2 == NULL )
  {
    ESP_LOGE( TAG, "Sensor Screen Is Null" );
  }
  else
  {
    ESP_LOGW( TAG, "Loading Sensor Screen" );
    GUI_LOCK();
    lv_disp_load_scr( ui_Screen2 );
    GUI_UNLOCK();
  }
}

/**
 * @brief Update the Temperature and Humidity data on display
 * @param data pointer to sensor data
 */
static void gui_update_sensor_data( void *data )
{
  #define TEMPERATURE_OVERFLOW_VALUE            (30u)
  #define HUMIDITY_OVERFLOW_VALUE               (60u)
  bool overflow = false;
  sensor_data_t *sensor_data;
  sensor_data = (sensor_data_t*)data;
  ESP_LOGI( TAG, "gui_update_sensor_data" );
  uint8_t temperature = sensor_data->temperature_current;
  uint8_t humidity = sensor_data->humidity_current;
  GUI_LOCK();
  lv_label_set_text_fmt( ui_lblSensor1, "%d°C", temperature );
  lv_arc_set_value( ui_arcSensor1, temperature );
  lv_label_set_text_fmt( ui_lblSensor2, "%d%%", humidity );
  lv_arc_set_value( ui_arcSensor2, humidity );
  
  // overflow logic and start of animation
  if ( temperature > TEMPERATURE_OVERFLOW_VALUE )
  {
    overflow = true;
    temp_ovf = true;
  }
  else
  {
    temp_ovf = false;
    // restore the color
    lv_obj_set_style_arc_color(ui_arcSensor1, lv_color_hex(0x36B9F6), LV_PART_INDICATOR | LV_STATE_DEFAULT);
  }
  
  if ( humidity > HUMIDITY_OVERFLOW_VALUE )
  {
    overflow = true;
    humidity_ovf = true;
  }
  else
  {
    humidity_ovf = false;
    // restore the color
    lv_obj_set_style_arc_color(ui_arcSensor2, lv_color_hex(0x36B9F6), LV_PART_INDICATOR | LV_STATE_DEFAULT);
  }
  
  if( overflow )
  {
    ESP_LOGI( TAG, "OverFlow Detected" );
    sensor_arc_animation_timer = SENSOR_ARC_ANIMATION_TIMER;
  }
  else
  {
    sensor_arc_animation_timer = 0;
  }
  GUI_UNLOCK();
}

/**
 * @brief Callback function to update the arc animation
 * @param data pointer to sensor data
 */
static void gui_sensor_arc_animation( void *data )
{
  static bool toggle_animation = false;
  GUI_LOCK();
  if( temp_ovf )
  {
    if ( toggle_animation )
    {
      lv_obj_set_style_arc_color(ui_arcSensor1, lv_color_hex(0x36B9F6), LV_PART_INDICATOR | LV_STATE_DEFAULT);
    }
    else
    {
      lv_obj_set_style_arc_color(ui_arcSensor1, lv_color_hex(0xF63653), LV_PART_INDICATOR | LV_STATE_DEFAULT);
    }
  }
  
  if( humidity_ovf )
  {
    if ( toggle_animation )
    {
      lv_obj_set_style_arc_color(ui_arcSensor2, lv_color_hex(0x36B9F6), LV_PART_INDICATOR | LV_STATE_DEFAULT);
    }
    else
    {
      lv_obj_set_style_arc_color(ui_arcSensor2, lv_color_hex(0xF63653), LV_PART_INDICATOR | LV_STATE_DEFAULT);
    }
  }
  GUI_UNLOCK();
  
  if ( toggle_animation )
  {
    toggle_animation = false;
  }
  else 
  {
    toggle_animation = true;
  }
}




