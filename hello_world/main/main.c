#include <stdio.h>
#include <stdbool.h>
#include <unistd.h>

#include "esp_log.h"
#include "freertos/idf_additions.h"

#include "lvgl.h"
#include "bsp/esp-bsp.h"
#include "bsp/display.h"

// Private Macros
#define MAIN_TASK_PERIOD                    (5000)

// Private Variables
static const char *TAG = "MAIN";

void lv_example_get_started_1(void)
{
    /*Change the active screen's background color*/
    lv_obj_set_style_bg_color(lv_screen_active(), lv_color_hex(0x003a57), LV_PART_MAIN);

    /*Create a white label, set its text and align it to the center*/
    lv_obj_t * label = lv_label_create(lv_screen_active());
    lv_label_set_text(label, "Hello world");
    lv_obj_set_style_text_color(lv_screen_active(), lv_color_hex(0xffffff), LV_PART_MAIN);
    lv_obj_align(label, LV_ALIGN_CENTER, 0, 0);
}

// Private Function Declarations
void app_main(void)
{
  ESP_LOGI( TAG, "Starting Program" );
  
  bsp_display_cfg_t cfg = 
  {
    .lvgl_port_cfg = ESP_LVGL_PORT_INIT_CONFIG(),
    .buffer_size = BSP_LCD_DRAW_BUFF_SIZE,
    .double_buffer = BSP_LCD_DRAW_BUFF_DOUBLE,
    .flags = 
    {
      .buff_dma = true,
      .buff_spiram = false,
      .sw_rotate = false,
    }
  };
  bsp_display_start_with_config(&cfg);
  bsp_display_backlight_on();

  bsp_display_lock(0);
  lv_example_get_started_1();
  bsp_display_unlock();
    
  while (true)
  {
    ESP_LOGI( TAG, "Periodic Print" );
    vTaskDelay(MAIN_TASK_PERIOD / portTICK_PERIOD_MS);
  }
}

// Private Function Definitions
