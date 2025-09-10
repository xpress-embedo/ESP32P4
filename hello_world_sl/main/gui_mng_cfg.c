/*
 * gui_mng_cfg.c
 *
 *  Created on: 06-Sep-2025
 *      Author: xpress_embedo
 */
 
#include "gui_mng.h"
#include "gui_mng_cfg.h"

#include "esp_log.h"
#include "esp_heap_caps.h"

#include "freertos/FreeRTOS.h"
#include "freertos/task.h"

#include "lvgl.h"
#include "ui.h"

// Private Macros
#define NUM_ELEMENTS(x)                 (sizeof(x)/sizeof(x[0]))

// function template for callback function
typedef void (*gui_mng_callback)(void * data);

// structure
typedef struct _gui_mng_event_cb_t
{
  gui_mng_event_t   event;
  gui_mng_callback  callback;
} gui_mng_event_cb_t;

// Private Function Prototypes
static void gui_hello_world( void *data );

// Private Variables
LV_IMG_DECLARE(ui_img_logo1_png);

static lv_img_dsc_t img_logo_dsc;       // main logo image descriptor
static const char *TAG = "GUI_CFG";

static const gui_mng_event_cb_t gui_mng_event_cb[] =
{
  { GUI_MNG_EV_HELLO_WORLD,             gui_hello_world               },
};


// Public Function Definitions
/**
 * @brief GUI Configurable Initialization Function
 * @param  None
 */
void gui_cfg_init( void )
{
  ESP_LOGI( TAG, "UI Init. Starts" );
  GUI_LOCK();
  ui_init();
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
  // use this for periodic usage
}

// Private Function Definitions
/**
 * @brief 
 * @param path 
 * @param img_dsc 
 * @return 
 */
static bool load_lvgl_image( const char *path, lv_img_dsc_t *img_dsc )
{
  // opens the image file in binary mode for reading
  FILE *f = fopen(path, "rb");
  if ( !f )
  {
    ESP_LOGE("LVGL", "Failed to open image file: %s", path);
    return false;
  }

  // calculates the file size by seeking to the end and rewinding
  fseek( f, 0, SEEK_END );
  size_t size = ftell( f );
  rewind(f);

  // allocates memory for the image data
  uint8_t *buffer = malloc(size);
  if ( !buffer )
  {
    fclose(f);
    ESP_LOGE("LVGL", "Failed to allocate memory");
    return false;
  }

  // reads the file data into the buffer and closes the file
  fread( buffer, 1, size, f );
  fclose(f);

  // copies the image descriptor (metadata like width, height, color format) 
  // from the start of the buffer
  memcpy( img_dsc, buffer, sizeof(lv_img_dsc_t) );
  // Points img_dsc->data to the pixel data portion of the buffer
  img_dsc->data = buffer + sizeof(lv_img_dsc_t);
  return true;
}

static void unload_lvgl_image ( lv_obj_t *img, lv_img_dsc_t *img_dsc )
{
  lv_obj_del( img );
  free( (void *)img_dsc->data - sizeof(lv_img_dsc_t) );
  memset( img_dsc, 0, sizeof(lv_img_dsc_t) );
}


/**
 * @brief Callback function when hello world event is received
 * @param data 
 */
static void gui_hello_world( void *data )
{
  ESP_LOGI( TAG, "Hello World Event Received" );
  size_t free_heap = heap_caps_get_free_size(MALLOC_CAP_DEFAULT);
  ESP_LOGW(TAG, "Free heap: %d bytes", free_heap);

  if ( load_lvgl_image("/assets/logo.bin", &img_logo_dsc) )
  {
    GUI_LOCK();
    lv_img_set_src( ui_imgLogo, &img_logo_dsc);
    GUI_UNLOCK();
    free_heap = heap_caps_get_free_size(MALLOC_CAP_DEFAULT);
    ESP_LOGW(TAG, "Free heap: %d bytes", free_heap);
  }
  else
  {
    ESP_LOGE(TAG, "Failed to load image");
  }
}

