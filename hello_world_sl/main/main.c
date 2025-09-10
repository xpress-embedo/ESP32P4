#include <stdio.h>
#include <stdbool.h>
#include <unistd.h>

#include "esp_log.h"
#include "esp_littlefs.h"

#include "freertos/idf_additions.h"

#include "gui_mng.h"
#include "data_config.h"

// Private Macros
#define MAIN_TASK_PERIOD                    (5000)

// Private Variables
static const char *TAG = "MAIN";

// Private Function Declarations
void mount_assets_fs( void );

// Application Main Entry Point
void app_main(void)
{
  ESP_LOGI( TAG, "Starting Program" );
  
  // start the gui task
  gui_start();

  // initialize the configuration data
  cfg_init();

  // mount the littlefs file system
  mount_assets_fs();
  
  gui_send_event( GUI_MNG_EV_HELLO_WORLD, NULL );
    
  while (true)
  {
    ESP_LOGI( TAG, "Periodic Print" );
    vTaskDelay(MAIN_TASK_PERIOD / portTICK_PERIOD_MS);
  }
}


// Private Function Definitions

/**
 * @brief Mount the LittleFS filesystem for assets
 * @param  none
 * @return none
 */
void mount_assets_fs( void )
{
  const esp_vfs_littlefs_conf_t conf = 
  {
    .base_path = "/assets",
    .partition_label = "assets",
    .format_if_mount_failed = false,  // Set to true if you want auto-format on failure
    .dont_mount = false,
  };

  esp_err_t err = esp_vfs_littlefs_register( &conf );
  if ( err != ESP_OK )
  {
    if ( err == ESP_FAIL )
    {
      ESP_LOGE(TAG, "Failed to mount or format filesystem");
    } 
    else if ( err == ESP_ERR_NOT_FOUND )
    {
      ESP_LOGE(TAG, "Failed to find LittleFS partition");
    } 
    else 
    {
      ESP_LOGE(TAG, "Failed to initialize LittleFS (%s)", esp_err_to_name(err));
    }
    return;
  }

  size_t total = 0, used = 0;
  err = esp_littlefs_info( conf.partition_label, &total, &used );
  if ( err == ESP_OK )
  {
    ESP_LOGI(TAG, "LittleFS mounted: total=%d bytes, used=%d bytes", total, used);
  }
  else 
  {
    ESP_LOGW(TAG, "Failed to get LittleFS info (%s)", esp_err_to_name(err));
    return;
  }

  FILE *f = fopen("/assets/hello.txt", "r");
  if ( f )
  {
    char buf[64];
    fgets(buf, sizeof(buf), f);
    fclose(f);
    ESP_LOGW(TAG, "Read from hello.txt: %s", buf);
  }
  else 
  {
    ESP_LOGE(TAG, "Failed to open hello.txt");
  }
}
