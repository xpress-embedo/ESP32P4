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
#define MAIN_TASK_PERIOD                    (10000)
#define DHT11_PIN                           (GPIO_NUM_17)

// Private Variables
static const char *TAG = "MAIN";
static sensor_data_t sensor_data = { .sensor_idx = 0 };
static bool sntp_connect_status = false;

// Private Function Declarations
static void log_app_heap( void );
static void app_sntp_init( void );
static bool app_sntp_get_time( void );

// Application Main Entry Point
void app_main(void)
{
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

  // initialize the configuration data
  cfg_init();

  // start the gui task
  gui_start();
  gui_send_event( GUI_MNG_EV_WIFI_CONNECTING, NULL );

  // start wifi application (soft access point and HTTP web server)
  wifi_app_start();

  // connect with WiFi (it will take some time) (will be controlled using wifi_app.c module)
  // wifi_sta_connect_init();
  
  // if( wifi_sta_is_connected() )
  // {
  //   ESP_LOGI( TAG, "WiFi Connected, now synchronizing with NTP server." );
  //   gui_send_event( GUI_MNG_EV_WIFI_CONNECTED, NULL );
  //   app_sntp_init();
  //   sntp_connect_status = app_sntp_get_time();
  //   // if time fetched then only start the influxDB server
  //   if( sntp_connect_status )
  //   {
  //     // now start the influxDB task
  //     influxdb_start();
  //     // gui_send_event( GUI_MNG_EV_WIFI_INTERNET_CONNECTED, NULL );
  //   }
  // }

  // initialize dht sensor library
  // dht11_init(DHT11_PIN, true);
    
  while(1)
  {
    // Get DHT11 Temperature and Humidity Values
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
          // if( wifi_sta_is_connected() && sntp_connect_status )
          // {
          //   influxdb_send_event(INFLUXDB_EV_TEMP_HUMID, NULL);
          // }
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
    // print available heap
    log_app_heap();
    // Wait before next measurement
    vTaskDelay(MAIN_TASK_PERIOD / portTICK_PERIOD_MS);
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

// Private Function Definitions

/**
 * @brief This function logs the current free heap size
 * @param  None
 */
static void log_app_heap( void )
{
  ESP_LOGW( TAG, "Free memory: %" PRIu32 " bytes", esp_get_free_heap_size() );
  // size_t free_heap = heap_caps_get_free_size(MALLOC_CAP_DEFAULT);
  // ESP_LOGW(TAG, "Free heap: %d bytes", free_heap);
}

// Private Function Definitions
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

