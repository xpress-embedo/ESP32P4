/*
 * influxDB.c
 *
 *  Created on: Jun 21, 2024
 *      Author: abc@xyz
 */
#include "esp_log.h"

#include "freertos/FreeRTOS.h"
#include "freertos/queue.h"
#include "freertos/task.h"

#include "esp_http_client.h"

#include "main.h"
#include "wifi_app.h"
#include "influxDB.h"


// Private Macros
#define INFLUXDB_EVENT_QUEUE_LEN            (5)
#define INFLUXDB_URL                        CONFIG_INFLUXDB_URL
#define INFLUXDB_ORG                        CONFIG_INFLUXDB_ORG
#define INFLUXDB_BUCKET                     CONFIG_INFLUXDB_BUCKET
#define INFLUXDB_TOKEN                      CONFIG_INFLUXDB_TOKEN

// Private Variables
static const char *TAG = "InfluxDB";
static const char *influxdb_url = INFLUXDB_URL;
static const char *influxdb_org = INFLUXDB_ORG;
static const char *influxdb_bucket = INFLUXDB_BUCKET;
static QueueHandle_t influxdb_q_event = NULL;
static bool influxdb_init = false;
static esp_http_client_handle_t client = NULL;

// Private Function Declaration
static void influxdb_task( void *pvParameters );
static void influxdb_send_temp_humidity( void );

// Public Function Definition
void influxdb_start( void )
{
  if ( false == influxdb_init )
  {
    ESP_LOGI( TAG, "Starting InfluxDB Task" );
    // create message queue with the length THINGSPEAK_EVENT_QUEUE_LEN
    influxdb_q_event = xQueueCreate( INFLUXDB_EVENT_QUEUE_LEN, sizeof(influxdb_q_msg_t) );
    if( influxdb_q_event == NULL )
    {
      ESP_LOGE(TAG, "Unable to Create Queue");
    }
    xTaskCreate(&influxdb_task, "InfluxDB Task", 4096*2, NULL, 6, NULL);
    influxdb_init = true;
  }
  else
  {
    ESP_LOGW( TAG, "InfludxDB was already initialized" );
    // we are here because of re-connection of WiFi
    // so we need to cleanup the previous client if any
    if ( client != NULL )
    {
      // cleanup the previous client
      esp_http_client_cleanup(client);
      // reset the client handle and then in send event function esp_http_client_init will create a new client
      client = NULL;
    }
  }
}

/**
 * @brief Send an event to influxdb queue
 * @param event event code
 * @param pData pointer to data if any (not used for future use)
 * @return pdTRUE if successful else pdFALSE
 */
BaseType_t influxdb_send_event( influxdb_event_t event, uint8_t *pData )
{
  BaseType_t status = pdFALSE;
  influxdb_q_msg_t msg;

  if( event < INFLUXDB_EV_MAX )
  {
    msg.event_id  = event;
    msg.data      = pData;
    if ( NULL == influxdb_q_event )
    {
      ESP_LOGE( TAG, "InfluxDB Queue Problem" );
    }
    else
    {
      status = xQueueSend( influxdb_q_event, &msg, portMAX_DELAY );
    }
  }
  return status;
}

// Private Function Definition

/**
 * @brief InfluxDB Task
 * The task waits if there is some event posted in queue, and then based on the
 * event it calls the appropriate function
 * @param pvParameters 
 */
static void influxdb_task( void *pvParameters )
{
  influxdb_q_msg_t msg;

  while( 1 )
  {
    // Wait for events posted in Queue
    if( xQueueReceive(influxdb_q_event, &msg, portMAX_DELAY) )
    {
      // the below is the code to handle the state machine
      if( INFLUXDB_EV_NONE != msg.event_id )
      {
        switch( msg.event_id )
        {
          case INFLUXDB_EV_TEMP_HUMID:
            influxdb_send_temp_humidity();
            break;
          default:
            break;
        } // switch case end
      }   // if event received in limit end
    }     // xQueueReceive end
  }
}

/**
 * @brief Send Temperature and Humidity data to InfluxDB Cloud
 * @param  none
 */
static void influxdb_send_temp_humidity( void )
{
  uint8_t temperature = 0;
  uint8_t humidity = 0;
  char data[150];
  char influxdb_full_url[200];
  char mac_addr[WIFI_MAC_ADDR_SIZE] = { 0 };

  sensor_data_t *sensor_data = get_temperature_humidity();
  temperature = sensor_data->temperature_current;
  humidity = sensor_data->humidity_current;
  wifi_app_get_mac_address( mac_addr );

  snprintf( data, sizeof(data), \
            "weather,device_id=%s temperature=%d,humidity=%d %lld", \
            mac_addr, temperature, humidity, get_time_ns() );
  // ESP_LOGI( TAG, "Data: %s", data );
  snprintf( influxdb_full_url, sizeof(influxdb_full_url),   \
            "%s/api/v2/write?org=%s&bucket=%s&precision=ns",\
            influxdb_url, influxdb_org, influxdb_bucket );

  
  if ( client == NULL )
  {
    esp_http_client_config_t config =
    {
      .url = influxdb_full_url,
      .method = HTTP_METHOD_POST,
      .timeout_ms = 5000,
      .transport_type = HTTP_TRANSPORT_OVER_SSL,   // Specify transport type
      // todo: maybe for future
      // .crt_bundle_attach = esp_crt_bundle_attach,  // Attach the certificate bundle
    };
    client = esp_http_client_init( &config );
    // set header
    esp_http_client_set_header( client, "Authorization", "Token " INFLUXDB_TOKEN );
    esp_http_client_set_header( client, "Content-Type", "text/plain" );
  }

  esp_http_client_set_post_field( client, data, strlen(data) );
  esp_err_t err = esp_http_client_perform(client);

  if (err == ESP_OK)
  {
    ESP_LOGI( TAG, "HTTP POST Status = %d", esp_http_client_get_status_code(client) );
    // ESP_LOGI( TAG, "content_length = %d", esp_http_client_get_content_length(client));
  }
  else
  {
    ESP_LOGE(TAG, "HTTP POST request failed: %s", esp_err_to_name(err));
  }

  // If you want to cleanup the client after each request, uncomment the below line
  // but it is better to keep the client initialized and reuse it for next request
  // esp_http_client_cleanup(client);
}
