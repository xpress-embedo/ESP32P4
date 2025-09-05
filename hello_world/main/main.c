#include <stdio.h>
#include <stdbool.h>
#include <unistd.h>

#include "esp_log.h"
#include "freertos/idf_additions.h"

// Private Macros
#define MAIN_TASK_PERIOD                    (5000)

// Private Variables
static const char *TAG = "MAIN";


// Private Function Declarations
void app_main(void)
{
  ESP_LOGI( TAG, "Starting Program" );
  while (true)
  {
    ESP_LOGI( TAG, "Periodic Print" );
    vTaskDelay(MAIN_TASK_PERIOD / portTICK_PERIOD_MS);
  }
}

// Private Function Definitions
