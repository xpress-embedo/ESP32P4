/*
 * gui_mng.c
 *
 *  Created on: 06-Sep-2025
 *      Author: abc@xyz
 */

#include "gui_mng.h"

#include "esp_log.h"

#include "freertos/FreeRTOS.h"
#include "freertos/task.h"
#include "freertos/semphr.h"

// Private Macros
#define GUI_EVENT_QUEUE_LEN                   (5)

// Private Variables
static const char *TAG = "GUI";
static QueueHandle_t gui_q_event = NULL;
static TimerHandle_t gui_refresh_timer = NULL;

// Private Function Declaration
static void gui_init( void );
static void gui_task(void *pvParameter);
static void gui_refresh_cb( TimerHandle_t xTimer );

// Public Function Definition
/**
 * @brief GUI Start Function, this function will start the gui manager task
 * @param  none
 */
void gui_start( void )
{
  gui_init();

  // create a periodic timer for GUI refresh
  gui_refresh_timer = xTimerCreate("GUI Refresh", pdMS_TO_TICKS(GUI_MNG_REFRESH_TIME), pdTRUE, NULL, gui_refresh_cb );
  // start the timer
  xTimerStart(gui_refresh_timer, 0);
    
  // callback function, task name, stack size, parameters, priority, task handle
  xTaskCreate(&gui_task, "gui task", 4096*8, NULL, 5, NULL);
}

/**
 * @brief Send GUI Event
 * @param event Event Code
 * @param pData Pointer to Data if Any
 * @return BaseType_t pdTRUE if successful else pdFALSE
 */
BaseType_t gui_send_event( gui_mng_event_t event, void *pData )
{
  BaseType_t status = pdFALSE;
  gui_q_msg_t msg;

  if( event < GUI_MNG_EV_MAX )
  {
    msg.event_id  = event;
    msg.data      = pData;
    status = xQueueSend( gui_q_event, &msg, portMAX_DELAY );
  }
  return status;
}

// Private Function Definitions

/**
 * @brief gui initialization task, this will initialize the semaphire and display
 * @param  none
 */
static void gui_init( void )
{
  // create message queue with the length GUI_EVENT_QUEUE_LEN
  gui_q_event = xQueueCreate( GUI_EVENT_QUEUE_LEN, sizeof(gui_q_msg_t) );
  if( gui_q_event == NULL )
  {
    ESP_LOGE(TAG, "Unable to Create Queue");
  }
  
  bsp_display_start();
  bsp_display_backlight_on();

  // main user interface
  gui_cfg_init();
}

/**
 * @brief gui task Function which calls the lvgl timer handler function
 *        and other updates on the user interface based on the events received
 * @param *pvParameter  task parameter
 */
static void gui_task(void *pvParameter)
{
  gui_q_msg_t msg;
  msg.event_id = GUI_MNG_EV_NONE;

  while(1)
  {
    // wait only GUI_MNG_REFRESH_TIME ms and then proceed
    if( xQueueReceive(gui_q_event, &msg, pdMS_TO_TICKS(GUI_MNG_REFRESH_TIME)) )
    {
      // the below is the code to handle the state machine
      if( GUI_MNG_EV_NONE != msg.event_id )
      {
        gui_cfg_mng_process(msg.event_id, msg.data);
      }   // if event received in limit end
    }     // xQueueReceive end
  }
}


/**
 * @brief GUI Refresh Timer Callback Function
 * This function will be called periodically based on the timer set
 * @param xTimer Timer Handle
 */
void gui_refresh_cb(TimerHandle_t xTimer)
{
  gui_cfg_refresh();
}

