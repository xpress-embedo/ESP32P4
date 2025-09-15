/*
 * wifi_sta_mng.h
 *
 *  Created on: Sep 15, 2025
 *      Author: abc@xyz
 */

#include "esp_log.h"
#include "freertos/idf_additions.h"
#include "freertos/event_groups.h"
#include "esp_system.h"
#include "esp_event.h"
#include "esp_netif.h"
#include "esp_wifi.h"
#include "esp_mac.h"

#include "wifi_sta_mng.h"

// Private Macros
#define APP_WIFI_SSID                       CONFIG_ESP_WIFI_SSID
#define APP_WIFI_PSWD                       CONFIG_ESP_WIFI_PASSWORD
#define WIFI_MAX_RETRY                      (6)
#define WIFI_CONNECT_DELAY                  (500)     // Initial delay in milliseconds
#define WIFI_MAX_DELAY                      (60000)   // Maximum delay in milliseconds
// The following are the bits/flags for event group
#define WIFI_CONNECTED_BIT                  BIT0      // connected to the access point with an IP
#define WIFI_FAIL_BIT                       BIT1      // failed to connect after the max. amount of retries

// Private Variables
static const char *TAG = "WIFI_STA_MNG";

/* WiFi Connection Related Variables */
static EventGroupHandle_t wifi_event_group;           // FreeRTOS event group to signal when we are connected
static uint8_t wifi_connect_retry = 0;
static bool wifi_connect_status = false;
static uint8_t wifi_mac_address[6] = { 0 };              // {AA,BB,CC,DD,EE,FF} MAC Address Format

// Private Function Declarations
static void app_connect_wifi( void );
static void set_mac_address( void );
static void get_mac_address( char *mac_address );
static void wifi_event_handler( void *arg, esp_event_base_t event_base, int32_t event_id, void * event_data );


// Public Function Definitions
/**
 * @brief Connect with the WiFi Router
 * @note  in future this function can be moved to a commom place.
 * @param  none
 */
void wifi_sta_connect( void )
{
  app_connect_wifi();
}

/**
 * @brief Check whether WiFi is connected or not
 * @param  none
 * @return true if connected else false
 */
bool wifi_sta_is_connected( void )
{
  return wifi_connect_status;
}

/**
 * @brief Get the MAC Address of the device
 * @param mac_address used to return the mac address as string
 */
void wifi_sta_get_mac_address( char *mac_address )
{
  get_mac_address( mac_address );
}


// Private Function Definitions
/**
 * @brief Connect with the WiFi Router
 * @note  in future this function can be moved to a commom place.
 * @param  none
 */
static void app_connect_wifi( void )
{
  wifi_event_group = xEventGroupCreate();

  ESP_LOGI( TAG, "Connecting with WiFi Router" );

  ESP_ERROR_CHECK( esp_netif_init() );

  ESP_ERROR_CHECK( esp_event_loop_create_default() );
  esp_netif_create_default_wifi_sta();

  wifi_init_config_t cfg = WIFI_INIT_CONFIG_DEFAULT();

  ESP_ERROR_CHECK( esp_wifi_init(&cfg) );

  // This is an ESP32P4 device which doesn't have EFUSE, so esp_read_mac 
  // function will not work while esp_wifi_get_mac function will work because 
  // it will query the MAC programmed in the ESP32C6 WiFi chip
  // so call this function to get the MAC address programmed in the WiFi chip
  // and store it in wifi_mac_address variable, as we don't want to query again and again
  // make sure to call this after esp_wifi_init function and only once
  set_mac_address();
  
  // setting MAC address as hostname
  esp_netif_t *netif = esp_netif_get_handle_from_ifkey("WIFI_STA_DEF");

  /*
  uint8_t mac[6];
  ESP_ERROR_CHECK(esp_wifi_get_mac(WIFI_IF_STA, mac));
  char hostname[32];
  snprintf( hostname, sizeof(hostname), "ESP_%02X:%02X:%02X:%02X:%02X:%02X", mac[0], mac[1], mac[2], mac[3], mac[4], mac[5]);
  */
  // Another way of doing things, since I want MAC address programmed in EFUSE of ESP32S3
  char mac_str[MAC_ADDR_SIZE];
  char hostname[32];
  get_mac_address( mac_str );
  snprintf(hostname, sizeof(hostname), "ESP32_%s", mac_str);
  
  ESP_ERROR_CHECK( esp_netif_set_hostname(netif, hostname) );
  ESP_LOGW( TAG, "Hostname set to MAC: %s", hostname );

  esp_event_handler_instance_t instance_any_id;
  esp_event_handler_instance_t instance_got_ip;
  ESP_ERROR_CHECK( esp_event_handler_instance_register( WIFI_EVENT,           \
                                                        ESP_EVENT_ANY_ID,     \
                                                        &wifi_event_handler,  \
                                                        NULL,                 \
                                                        &instance_any_id) );
  ESP_ERROR_CHECK( esp_event_handler_instance_register( IP_EVENT,             \
                                                        IP_EVENT_STA_GOT_IP,  \
                                                        &wifi_event_handler,  \
                                                        NULL,                 \
                                                        &instance_got_ip) );

  wifi_config_t wifi_config =
  {
    .sta =
    {
      .ssid = APP_WIFI_SSID,
      .password = APP_WIFI_PSWD,
      .threshold.authmode = WIFI_AUTH_WPA2_PSK,
    },
  };

  ESP_ERROR_CHECK( esp_wifi_set_mode(WIFI_MODE_STA) );
  ESP_ERROR_CHECK( esp_wifi_set_config(WIFI_IF_STA, &wifi_config) );
  ESP_ERROR_CHECK( esp_wifi_start() );

  ESP_LOGI(TAG, "WiFi Initialized in Station Mode Finished.");

  /*
   * Wait until either the connection is established (WIFI_CONNECTED_BIT) or
   * connection failed for the maximum number of re-tries (WIFI_FAIL_BIT).
   * The bits are set by event_handler() (see above) */
  EventBits_t bits = xEventGroupWaitBits( wifi_event_group,                   \
                                          WIFI_CONNECTED_BIT | WIFI_FAIL_BIT, \
                                          pdFALSE,                            \
                                          pdFALSE,                            \
                                          portMAX_DELAY);

  /* xEventGroupWaitBits() returns the bits before the call returned, hence we
   * can test which event actually happened. */
  if( bits & WIFI_CONNECTED_BIT )
  {
    ESP_LOGI(TAG, "Connected to Access Point %s", APP_WIFI_SSID );
  }
  else if( bits & WIFI_FAIL_BIT )
  {
    ESP_LOGE(TAG, "Failed to Connect to Access Point %s", APP_WIFI_SSID );
  }
  else
  {
    ESP_LOGE(TAG, "Unexpected Event" );
  }
  vEventGroupDelete(wifi_event_group);
}

/**
 * @brief WiFi Event Handler Function
 * @param arg
 * @param event_base Event Base whether WIFI Event or IP Event
 * @param event_id   Event ID
 * @param event_data Data with Event
 */
static void wifi_event_handler( void *arg, esp_event_base_t event_base, int32_t event_id, void * event_data )
{
  if( WIFI_EVENT == event_base )
  {
    if( WIFI_EVENT_STA_START == event_id )
    {
      esp_wifi_connect();
    }
    else if( WIFI_EVENT_STA_DISCONNECTED == event_id )
    {
      if( wifi_connect_retry < WIFI_MAX_RETRY )
      {
        uint32_t delay = (1 << wifi_connect_retry) * WIFI_CONNECT_DELAY;
        delay = (delay > WIFI_MAX_DELAY) ? WIFI_MAX_DELAY : delay;
        // waiting for some time before retrying again
        vTaskDelay(delay / portTICK_PERIOD_MS);
        esp_wifi_connect();
        wifi_connect_retry++;
        ESP_LOGI(TAG, "Retry Wi-Fi connection (%d/%d)...", wifi_connect_retry, WIFI_MAX_RETRY);
      }
      else
      {
        wifi_connect_status = false;
        xEventGroupSetBits(wifi_event_group, WIFI_FAIL_BIT);
        ESP_LOGE(TAG, "Failed to connect to Access Point.");
      }
    }
  } // if( WIFI_EVENT = event_base )
  else if( IP_EVENT == event_base )
  {
    ip_event_got_ip_t *event = (ip_event_got_ip_t *)event_data;
    ESP_LOGI(TAG, "Got IP:" IPSTR, IP2STR(&event->ip_info.ip));
    wifi_connect_retry = 0;
    wifi_connect_status = true;
    xEventGroupSetBits(wifi_event_group, WIFI_CONNECTED_BIT);
    // checking the set HostName
    esp_netif_t *netif = esp_netif_get_handle_from_ifkey("WIFI_STA_DEF");
    const char *hostname = NULL;
    esp_err_t err = esp_netif_get_hostname(netif, &hostname);
    if (err == ESP_OK && hostname != NULL) 
    {
      ESP_LOGI(TAG, "Current Hostname: %s", hostname);
    }
    else
    {
      ESP_LOGW(TAG, "Failed to get hostname: %s", esp_err_to_name(err));
    }
  }
}


/**
 * @brief Get the MAC Address of the device and set it to wifi_mac_address variable
 *        esp_read_mac function will not work for ESP32P4 because it doesn't have EFUSE
 *        and will not read anything from there, while this function will work because it
 *        will query the MAC programmed in the ESP32C6 WiFi chip.
 *        But make sure to call this function after esp_wifi_init function
 *        This function stores the MAC address in wifi_mac_address variable, so
 *        we have to not query again and again.
 * @param  none
 */
static void set_mac_address( void )
{
  // For ESP32P4 this function will work because it will query the MAC programmed in the ESP32C6 WiFi chip  
  esp_wifi_get_mac( WIFI_IF_STA, wifi_mac_address );
}

/**
 * @brief Get the MAC Address of the device
 * @param mac_str used to return the mac address as string
 */
static void get_mac_address( char *mac_str )
{
  #if 0
  // This function will not work for ESP32P4 because it doesn't have EFUSE and will not read anything from there
  uint8_t mac[6];
  /* NOTE: there is an another function named esp_wifi_get_mac and the below function.
  Both function are used to get the mac address but there is a difference.
  esp_wifi_get_mac: Returns the currently MAC used by WiFi, i.e. after calling function esp_wifi_init
  This function may reflect the overridden MAC is esp_wifi_set_mac function was used
  while esp_read_mac returns the Factory Programmed MAC directly from EFUSE, doesn't
  require WiFi drivers to be running */
  // esp_read_mac( mac, ESP_MAC_WIFI_STA );
  snprintf( mac_str, MAC_ADDR_SIZE, "%02X:%02X:%02X:%02X:%02X:%02X", mac[0], mac[1], mac[2], mac[3], mac[4], mac[5] );
  ESP_LOGI( TAG, "MAC Address: %s", mac_str );
  #else
  // Call this function after set_mac_address function
  // because set_mac_address function will store the MAC address in wifi_mac_address variable
  snprintf( mac_str, MAC_ADDR_SIZE, "%02X:%02X:%02X:%02X:%02X:%02X", \
            wifi_mac_address[0], wifi_mac_address[1], wifi_mac_address[2], \
            wifi_mac_address[3], wifi_mac_address[4], wifi_mac_address[5] );
  ESP_LOGI( TAG, "MAC Address: %s", mac_str );
  #endif
}
