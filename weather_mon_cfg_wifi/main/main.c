#include <nvs_flash.h>
#include <driver/gpio.h>
#include <time.h>
#include <sys/time.h>

#include "esp_log.h"
#include "freertos/idf_additions.h"

// #include "wifi_sta_mng.h"

#include "esp_sntp.h"

#include "main.h"
#include "dht11.h"
#include "influxDB.h"
#include "gui_mng.h"
#include "data_config.h"
#include "wifi_app.h"

// Test Code
#include "esp_random.h"

// Private Macros
#define MAIN_TASK_PERIOD                    (1000)
#define DHT11_PIN                           (GPIO_NUM_17)
#define MAIN_EVENT_QUEUE_LEN                (5)

// Private Variables
static const char *TAG = "MAIN";
static sensor_data_t sensor_data = { .sensor_idx = 0 };
static bool sntp_connect_status = false;
static QueueHandle_t main_q_event = NULL;

// Private Function Declarations
static void log_app_heap( void );
static void measure_temp_humidity( void );
static void app_sntp_init( void );
static bool app_sntp_get_time( void );

// Application Main Entry Point
void app_main(void)
{
  main_q_msg_t main_msg;

  // Disable default gpio logging messages
  esp_log_level_set("gpio", ESP_LOG_NONE);
  // disable default wifi logging messages
  esp_log_level_set("wifi", ESP_LOG_NONE);
  esp_log_level_set("wifi_init", ESP_LOG_NONE);
  esp_log_level_set("sleep", ESP_LOG_NONE);
  esp_log_level_set("spi_flash", ESP_LOG_NONE);
  vTaskDelay(1000 / portTICK_PERIOD_MS);

  // Initialize NVS
  esp_err_t ret = nvs_flash_init();
  if ( (ret == ESP_ERR_NVS_NO_FREE_PAGES) || (ret == ESP_ERR_NVS_NEW_VERSION_FOUND) )
  {
    ESP_ERROR_CHECK(nvs_flash_erase());
    ret = nvs_flash_init();
  }
  ESP_ERROR_CHECK(ret);

  // print available heap
  log_app_heap();
  ESP_LOGI( TAG, "IDF version: %s", esp_get_idf_version() );

  // create message queue with the length MAIN_EVENT_QUEUE_LEN
  main_q_event = xQueueCreate( MAIN_EVENT_QUEUE_LEN, sizeof(main_q_msg_t) );
  if( main_q_event == NULL )
  {
    ESP_LOGE(TAG, "Unable to Create Main Event Queue");
  }

  // initialize the configuration data
  cfg_init();

  // start the gui task
  gui_start();

  // start wifi application (soft access point and HTTP web server)
  wifi_app_start();

  // initialize dht sensor library
  // dht11_init(DHT11_PIN, true);
    
  while(1)
  {
    static uint8_t measure_counter = 0;
    measure_counter++;
    // 1 min per measurement
    if ( measure_counter >= 10u )
    {
      // Get DHT11 Temperature and Humidity Values
      measure_temp_humidity();
      // print available heap
      log_app_heap();
      measure_counter = 0u;
    }
    // Wait for events posted in Queue
    if( xQueueReceive( main_q_event, &main_msg, pdMS_TO_TICKS(MAIN_TASK_PERIOD) ) )
    {
      // below is the code to handle the state machine
      switch ( main_msg.event_id )
      {
        case MAIN_EV_HTTP_SERVER_STARTED:
          // send event to gui manager
          gui_send_event( GUI_MNG_EV_WIFI_CONNECTING, NULL );
          break;
        case MAIN_EV_AP_LIST_AVAILABLE:
          gui_send_event( GUI_MNG_EV_WIFI_AP_LIST_AVAILABLE, main_msg.data );
          break;
        case MAIN_EV_STA_CONNECTED:
          ESP_LOGI( TAG, "WiFi Connected, now synchronizing with NTP server." );
          gui_send_event( GUI_MNG_EV_WIFI_CONNECTED, NULL );
          app_sntp_init();
          sntp_connect_status = app_sntp_get_time();
          // if time fetched then only start the influxDB server
          if ( sntp_connect_status )
          {
            // now send the influxDB task
            main_send_event( MAIN_EV_START_INFLUXDB, NULL );
            gui_send_event( GUI_MNG_EV_WIFI_INTERNET_CONNECTED, NULL );
          }
          break;
        case MAIN_EV_START_INFLUXDB:
          // need to add a check inside the module, to not start again if already started
          influxdb_start();
          break;
        default:
          ESP_LOGE( TAG, "Invalid Event Received" );
          break;
      }
    }
  }
}

// Public Function Definition
/**
 * @brief Get the Pointer to the Sensor Data Structure to get the temperature
 *        and Humidity values
 * @param  None
 * @return sensor_data data structure pointer
 */
sensor_data_t * get_temperature_humidity( void )
{
  return &sensor_data;
}

/**
 * @brief Get the current time in nanoseconds
 * @param  None
 * @return current time in nanoseconds
 */
long long get_time_ns( void )
{
  struct timeval now;
  gettimeofday( &now, NULL );
  long long time_ns = (long long)now.tv_sec * 1000000000LL + now.tv_usec * 1000LL;

  return time_ns;
}

BaseType_t main_send_event( main_event_t event, void *ptr_data )
{
  BaseType_t status = pdFALSE;
  main_q_msg_t msg;

  if( event < MAIN_EV_MAX )
  {
    msg.event_id  = event;
    msg.data      = ptr_data;
    status = xQueueSend( main_q_event, &msg, portMAX_DELAY );
  }
  return status;
}

// Private Function Definitions

/**
 * @brief This function logs the current free heap size
 * @param  None
 */
static void log_app_heap( void )
{
  // ESP_LOGW( TAG, "Free memory: %" PRIu32 " bytes", esp_get_free_heap_size() );
  size_t internal_free = heap_caps_get_free_size( MALLOC_CAP_INTERNAL );
  size_t external_free = heap_caps_get_free_size( MALLOC_CAP_SPIRAM );
  size_t largest_internal = heap_caps_get_largest_free_block( MALLOC_CAP_INTERNAL );
  size_t largest_external = heap_caps_get_largest_free_block( MALLOC_CAP_SPIRAM );

  ESP_LOGW( TAG, "Internal RAM: Free = %u bytes, Largest block = %u bytes", internal_free, largest_internal );
  ESP_LOGW( TAG, "External PSRAM: Free = %u bytes, Largest block = %u bytes", external_free, largest_external );
}

static void measure_temp_humidity( void )
{
  // if( dht11_read().status == DHT11_OK )
  if ( 1 )
  {
    // uint8_t temp = (uint8_t)dht11_read().humidity;
    uint8_t temp = 50 + (uint8_t)(esp_random() % 10);
    // humidity can't be greater than 100%, that means invalid data
    if( temp < 100 )
    {
      if( sensor_data.sensor_idx < SENSOR_BUFF_SIZE )
      {
        sensor_data.humidity[sensor_data.sensor_idx] = temp;
        sensor_data.humidity_current = temp;
        // temp = (uint8_t)dht11_read().temperature;
        temp = 20 + (uint8_t)(esp_random() % 5);
        sensor_data.temperature[sensor_data.sensor_idx] = temp;
        sensor_data.temperature_current = temp;
        ESP_LOGI(TAG, "Temperature: %d", sensor_data.temperature_current);
        ESP_LOGI(TAG, "Humidity: %d", sensor_data.humidity_current);
        sensor_data.sensor_idx++;
        // trigger event to display temperature and humidity
        gui_send_event(GUI_MNG_EV_TEMP_HUMID, (void*)(&sensor_data) );
        // if wifi is connected, trigger event to send data to ThingSpeak
        if( wifi_app_is_connected() && sntp_connect_status )
        {
          influxdb_send_event(INFLUXDB_EV_TEMP_HUMID, NULL);
        }
        // reset the index
        if( sensor_data.sensor_idx >= SENSOR_BUFF_SIZE )
        {
          sensor_data.sensor_idx = 0;
        }
      }
    }
    else
    {
      ESP_LOGE(TAG, "In-correct data received from DHT11 -> %u", temp);
    }
  }
  else
  {
    ESP_LOGE(TAG, "Unable to Read DHT11 Status");
  }
}

/**
 * @brief Initialize the SNTP
 * @param  None
 */
static void app_sntp_init( void )
{
  ESP_LOGI( TAG, "Initializing SNTP" );
  esp_sntp_setoperatingmode( SNTP_OPMODE_POLL );
  esp_sntp_setservername( 0, "pool.ntp.org" );  // set the SNTP server
  esp_sntp_init();
}

/**
 * @brief Synchronize the time from the SNTP server
 * @param  None
 * @return true if synchronization is successful else false
 */
static bool app_sntp_get_time( void )
{
  #define MAX_RETRY_COUNT_SNTP    (20)
  bool status = false;
  char time_buffer[50] = { 0 };   // temporary: only for printing/debugging

  // set the time zone to India Standard Time (IST)
  setenv( "TZ", "IST-5:30", 1);
  tzset();

  // wait for the time to be set
  time_t now = 0;
  struct tm time_info = { 0 };
  uint8_t retry = 0;

  time(&now);
  localtime_r( &now, &time_info );
  // the function is similar to snprintf, but is used to format the time
  strftime(time_buffer, sizeof(time_buffer), "%d.%m.%Y %H:%M:%S", &time_info);
  ESP_LOGI(TAG, "Sync Current Time: %s", time_buffer);

  while( (time_info.tm_year < (2020-1900)) && (retry < MAX_RETRY_COUNT_SNTP) )
  {
    ESP_LOGI( TAG, "Synchronizing the time.....%d", retry );
    retry++;
    vTaskDelay( (retry*3000)/portTICK_PERIOD_MS );
    time(&now);
    // The localtime_r() function converts the calendar time pointed to by clock
    // into a broken-down time stored in the structure to which result points.
    // The localtime_r() function also returns a pointer to that same structure.
    localtime_r( &now, &time_info );

    memset(time_buffer, 0x00, sizeof(time_buffer) );
    // the function is similar to snprintf, but is used to format the time
    strftime(time_buffer, sizeof(time_buffer), "%d.%m.%Y %H:%M:%S", &time_info);
    ESP_LOGI(TAG, "Sync Current Time: %s", time_buffer);
  }

  if( retry < MAX_RETRY_COUNT_SNTP )
  {
    // it means time is synchronized with SNTP server
    status = true;
  }
  return status;
}

