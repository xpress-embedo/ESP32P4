#include <stdio.h>
#include <stdbool.h>
#include <unistd.h>

#include "esp_log.h"
#include "esp_partition.h"

#include "freertos/idf_additions.h"

#include "gui_mng.h"

// Private Macros
#define MAIN_TASK_PERIOD                    (5000)

// Private Variables
static const char *TAG = "MAIN";


void read_config()
{
  const esp_partition_t* config_partition = esp_partition_find_first( ESP_PARTITION_TYPE_DATA, 0x40, "config" );

  if ( config_partition )
  {
    uint8_t buffer[80]; // Adjust size as needed
    esp_partition_read(config_partition, 0, buffer, sizeof(buffer));
    ESP_LOGI( TAG, "Read config data: %s", buffer );
  }
  else
  {
    ESP_LOGE( TAG, "Config partition not found!" );
  }
}

// Private Function Declarations
void app_main(void)
{
  ESP_LOGI( TAG, "Starting Program" );

  read_config();
  
  // start the gui task
  gui_start();
  
  gui_send_event( GUI_MNG_EV_HELLO_WORLD, NULL );
    
  while (true)
  {
    ESP_LOGI( TAG, "Periodic Print" );
    vTaskDelay(MAIN_TASK_PERIOD / portTICK_PERIOD_MS);
  }
}


// Private Function Definitions
