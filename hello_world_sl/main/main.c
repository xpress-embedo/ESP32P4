#include <stdio.h>
#include <stdbool.h>
#include <unistd.h>

#include "esp_log.h"

#include "freertos/idf_additions.h"

#include "gui_mng.h"
#include "data_config.h"
#include "assets_mng.h"

// Private Macros
#define MAIN_TASK_PERIOD                    (5000)

// Private Function Prototypes
static void log_heap( void );

// Private Variables
static const char *TAG = "MAIN";

// Application Main Entry Point
void app_main(void)
{
  ESP_LOGI( TAG, "Starting Program" );
  
  // start the gui task
  gui_start();

  // initialize the configuration data
  cfg_init();

  // print available heap
  log_heap();

  // mount the littlefs file system which contains project assets
  assets_mng_init();
  
  gui_send_event( GUI_MNG_EV_HELLO_WORLD, NULL );
    
  while (true)
  {
    vTaskDelay(MAIN_TASK_PERIOD / portTICK_PERIOD_MS);
    log_heap();
  }
}


// Private Function Definitions

/**
 * @brief This function logs the current free heap size
 * @param  None
 */
static void log_heap( void )
{
  size_t free_heap = heap_caps_get_free_size(MALLOC_CAP_DEFAULT);
  ESP_LOGW(TAG, "Free heap: %d bytes", free_heap);
}

