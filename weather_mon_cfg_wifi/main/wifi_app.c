/*
 * wifi_app.c
 *
 *  Created on: Aug 8, 2025
 *      Author: xpress_embedo
 */

 #include <string.h>
#include "freertos/FreeRTOS.h"
#include "freertos/event_groups.h"
#include "freertos/task.h"

#include "esp_err.h"
#include "esp_log.h"
#include "esp_mac.h"
#include "lwip/netdb.h"

#include "wifi_app.h"
#include "http_server.h"
#include "nvs_app.h"
#include "main.h"
// #include "mqtt_app.h"

// Private Macros
#define WIFI_APP_QUEUE_SIZE                           (5)
#define WIFI_APP_TASK_SIZE                            (4*1024u)
#define WIFI_APP_TASK_PRIORITY                        (5u)

// Private Variables
static const char *TAG = "WiFi_APP";

// Queue handle used to manipulate the main queue events
static QueueHandle_t wifi_app_q_handle;

// used for returning the WiFi configuration
static wifi_config_t * wifi_config = NULL;
// used to track the number of retries when a connection attempt fails
static uint8_t wifi_connect_retry = 0;
static uint8_t wifi_mac_address[6] = { 0 };           // {AA,BB,CC,DD,EE,FF} MAC Address Format

// WiFi application Event group handle and status bits
static EventGroupHandle_t wifi_app_event_group;
const int WIFI_APP_CONNECTING_USING_SAVED_CREDS_BIT   = BIT0;
const int WIFI_APP_CONNECTING_FROM_HTTP_SERVER_BIT    = BIT1;
const int WIFI_APP_USER_REQUESTED_STA_DISCONNECT_BIT  = BIT2;
// This bit is used to keep track whether the STA is connected or not
const int WIFI_APP_STA_CONNECTED_GOT_IP_BIT           = BIT3;

// netif objects for the station mode and access point modes
esp_netif_t* esp_netif_sta = NULL;
esp_netif_t* esp_netif_ap = NULL;

// Private Function Prototypes
static void wifi_app_task(void *pvParameter);
static void wifi_app_event_handler_init( void );
static void wifi_app_default_wifi_init( void );
static void wifi_app_soft_ap_config( void );
static void wifi_app_connect_sta(void);
static void wifi_app_event_handler( void *arg, esp_event_base_t event_base, int32_t event_id, void * event_data );

#if 0
// Test Code to Log DNS Info
void log_dns_info()
{
  esp_netif_dns_info_t dns_info;
  esp_netif_t *netif = esp_netif_get_handle_from_ifkey("WIFI_STA_DEF");  // Default Wi-Fi interface

  if (netif == NULL)
  {
    ESP_LOGW(TAG, "Failed to get netif handle");
    return;
  }

  esp_err_t err = esp_netif_get_dns_info(netif, ESP_NETIF_DNS_MAIN, &dns_info);
  if (err == ESP_OK)
  {
    ESP_LOGI(TAG, "DNS IP: " IPSTR, IP2STR(&dns_info.ip.u_addr.ip4));
  }
  else
  {
    ESP_LOGW(TAG, "Failed to get DNS info: %s", esp_err_to_name(err));
  }
}
#endif


// Public Function Definitions
void wifi_app_start( void )
{
  ESP_LOGI( TAG, "Starting WiFi Application" );

  // Disable default logging messages
  esp_log_level_set("wifi", ESP_LOG_NONE);

  // Allocate memory for the WiFi Configuration
  wifi_config = (wifi_config_t*)malloc( sizeof(wifi_config_t) );
  memset( wifi_config, 0x00, sizeof(wifi_config_t) );

  // create a message queue
  wifi_app_q_handle = xQueueCreate( WIFI_APP_QUEUE_SIZE, sizeof(wifi_app_queue_msg_t) );

  // Create wifi application event group
  wifi_app_event_group = xEventGroupCreate();

  // start the WiFi application task
  xTaskCreate(&wifi_app_task, "wifi app task", WIFI_APP_TASK_SIZE, NULL, WIFI_APP_TASK_PRIORITY, NULL);
}

BaseType_t wifi_app_send_msg( wifi_app_msg_e msg_id )
{
  wifi_app_queue_msg_t msg;
  msg.msg_id = msg_id;
  return xQueueSend( wifi_app_q_handle, &msg, portMAX_DELAY );
}

/*
 * Get the WiFi Configuration
 */
wifi_config_t * wifi_app_get_wifi_config( void )
{
  return wifi_config;
}

bool wifi_app_is_connected( void )
{
  // todo
  return true;
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
void wifi_app_set_mac_address( void )
{
  // For ESP32P4 this function will work because it will query the MAC programmed in the ESP32C6 WiFi chip  
  esp_wifi_get_mac( WIFI_IF_STA, wifi_mac_address );
}

/**
 * @brief Get the MAC Address of the device
 * @param mac_str used to return the mac address as string
 */
void wifi_app_get_mac_address( char *mac_address )
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
  snprintf( mac_str, WIFI_MAC_ADDR_SIZE, "%02X:%02X:%02X:%02X:%02X:%02X", mac[0], mac[1], mac[2], mac[3], mac[4], mac[5] );
  ESP_LOGI( TAG, "MAC Address: %s", mac_str );
  #else
  // Call this function after set_mac_address function
  // because set_mac_address function will store the MAC address in wifi_mac_address variable
  snprintf( mac_address, WIFI_MAC_ADDR_SIZE, "%02X:%02X:%02X:%02X:%02X:%02X", \
            wifi_mac_address[0], wifi_mac_address[1], wifi_mac_address[2], \
            wifi_mac_address[3], wifi_mac_address[4], wifi_mac_address[5] );
  ESP_LOGI( TAG, "MAC Address: %s", mac_address );
  #endif
}

// Private Function Definitions
static void wifi_app_task(void *pvParameter)
{
  wifi_app_queue_msg_t msg;
  EventBits_t event_bits;

  // initialize the event handler
  wifi_app_event_handler_init();

  // initialize the TCP/IP stack and wifi config
  wifi_app_default_wifi_init();

  // SoftAP Config
  wifi_app_soft_ap_config();

  // start wifi
  ESP_ERROR_CHECK( esp_wifi_start() );

  // send the first message
  wifi_app_send_msg( WIFI_APP_MSG_LOAD_SAVED_CREDENTIALS );
  // wifi_app_send_msg( WIFI_APP_MSG_START_HTTP_SERVER );

  for( ;; )
  {
    if( xQueueReceive(wifi_app_q_handle, &msg, portMAX_DELAY) )
    {
      switch( msg.msg_id )
      {
        case WIFI_APP_MSG_START_HTTP_SERVER:
          ESP_LOGI( TAG, "WIFI_APP_MSG_START_HTTP_SERVER" );
          http_server_start();
          main_send_event( MAIN_EV_HTTP_SERVER_STARTED, NULL );
          break;
        case WIFI_APP_MSG_CONNECTING_FROM_HTTP_SERVER:
          ESP_LOGI( TAG, "WIFI_APP_MSG_CONNECTING_FROM_HTTP_SERVER" );

          xEventGroupSetBits(wifi_app_event_group, WIFI_APP_CONNECTING_FROM_HTTP_SERVER_BIT);

          // Attempt a Connection
          wifi_app_connect_sta();

          // set the current number of retries to zero
          wifi_connect_retry = 0;

          // Let the HTTP Server knows about the connection attempt
          http_server_monitor_send_msg( HTTP_MSG_WIFI_CONNECT_INIT );
          break;
        case WIFI_APP_MSG_STA_CONNECTED_GOT_IP:
          ESP_LOGI( TAG, "WIFI_APP_MSG_STA_CONNECTED_GOT_IP" );

          xEventGroupSetBits(wifi_app_event_group, WIFI_APP_STA_CONNECTED_GOT_IP_BIT);

          // send message to http server that esp32 is connected as station
          http_server_monitor_send_msg( HTTP_MSG_WIFI_CONNECT_SUCCESS );

          // send message to mqtt application that esp32 is connected and u can connect with mqtt server
          // mqtt_app_send_msg( MQTT_APP_MSG_START_CONNECTION );

          // send message to gui manager that mqtt is starting
          // gui_send_event( GUI_MNG_EV_MQTT_CONNECTING, NULL );

          // Send the event to main module, mentioning that connected to STA
          main_send_event( MAIN_EV_STA_CONNECTED, NULL );

          // here we got the IP and hence we need to save the credentials in the flash
          event_bits = xEventGroupGetBits( wifi_app_event_group );

          // save Station credentials only when saving from the HTTP server
          // because in another case it is already loaded from nvs and no need to save again
          if( event_bits & WIFI_APP_CONNECTING_USING_SAVED_CREDS_BIT )
          {
            // clearing this bit again here, in case we want to disconnect and connect again
            // in this case we need to save the new credentials again
            xEventGroupClearBits(wifi_app_event_group, WIFI_APP_CONNECTING_USING_SAVED_CREDS_BIT);
          }
          else
          {
            // save credentials
            nvs_app_save_sta_creds();
          }

          if( event_bits & WIFI_APP_CONNECTING_FROM_HTTP_SERVER_BIT )
          {
            xEventGroupClearBits(wifi_app_event_group, WIFI_APP_CONNECTING_FROM_HTTP_SERVER_BIT);
          }
          break;
        case WIFI_APP_MSG_USR_REQUESTED_STA_DISCONNECT:
          ESP_LOGI(TAG, "WIFI_APP_MSG_USR_REQUESTED_STA_DISCONNECT");

          event_bits = xEventGroupGetBits(wifi_app_event_group);
          // If connected, then disconnect and clear the credentials
          // NOTE: Disconnection User Request means pressing "Disconnect" button
          if( event_bits & WIFI_APP_STA_CONNECTED_GOT_IP_BIT )
          {
            xEventGroupSetBits(wifi_app_event_group, WIFI_APP_USER_REQUESTED_STA_DISCONNECT_BIT);

            wifi_connect_retry = WIFI_MAX_CONNECTION_RETRIES;
            ESP_ERROR_CHECK( esp_wifi_disconnect() );

            // since user requested the disconnection, it's better to clear the credentials
            nvs_app_clear_sta_creds();
          }

          break;
        case WIFI_APP_MSG_STA_DISCONNECTED:
          ESP_LOGI(TAG,"WIFI_APP_MSG_STA_DISCONNECTED");

          // send message to mqtt application regarding disconnection
          // mqtt_app_send_msg( MQTT_APP_MSG_STOP_CONNECTION );

          event_bits = xEventGroupGetBits(wifi_app_event_group);

          if( event_bits & WIFI_APP_CONNECTING_USING_SAVED_CREDS_BIT )
          {
            ESP_LOGI(TAG, "WIFI_APP_MSG_STA_DISCONNECTED: Attempt Using Saved Credentials");
            xEventGroupClearBits(wifi_app_event_group, WIFI_APP_CONNECTING_USING_SAVED_CREDS_BIT);
            // At startup, the maximum retries is reached and the connection can't be established
            // hence we will clear the flash
            nvs_app_clear_sta_creds();
          }
          else if( event_bits & WIFI_APP_CONNECTING_FROM_HTTP_SERVER_BIT )
          {
            ESP_LOGI(TAG, "WIFI_APP_MSG_STA_DISCONNECTED: Attempt Using HTTP Server");
            xEventGroupClearBits(wifi_app_event_group, WIFI_APP_CONNECTING_FROM_HTTP_SERVER_BIT);
            // send message to http server that esp32 is disconnected as station
            http_server_monitor_send_msg( HTTP_MSG_WIFI_CONNECT_FAIL );
          }
          else if( event_bits & WIFI_APP_USER_REQUESTED_STA_DISCONNECT_BIT )
          {
            ESP_LOGI(TAG, "WIFI_APP_MSG_STA_DISCONNECTED: User Requested Disconnection");
            xEventGroupClearBits(wifi_app_event_group, WIFI_APP_USER_REQUESTED_STA_DISCONNECT_BIT);
            // send message to http server that esp32 is disconnected as station
            http_server_monitor_send_msg( HTTP_MSG_WIFI_USER_DISCONNECT );
            // send message to gui manager to update the icons
            // gui_send_event( GUI_MNG_EV_WIFI_DISCONNECTED, NULL );
          }
          else
          {
            ESP_LOGI(TAG, "WIFI_APP_MSG_STA_DISCONNECTED: Attempt Failed check WiFi Access Point availability");
            // Adjust this case according to our needs (let's say retrying etc)
          }

          break;
        case WIFI_APP_MSG_LOAD_SAVED_CREDENTIALS:
          ESP_LOGI(TAG,"WIFI_APP_MSG_LOAD_SAVED_CREDENTIALS");

          if( nvs_app_load_sta_creds() )
          {
            ESP_LOGI(TAG, "Loaded Station Credentials");
            wifi_app_connect_sta();
            xEventGroupSetBits( wifi_app_event_group, WIFI_APP_CONNECTING_USING_SAVED_CREDS_BIT );
          }
          else
          {
            ESP_LOGW( TAG, "Unable to Load the Saved Credential" );
            // don't attempt further connections
            ESP_ERROR_CHECK( esp_wifi_disconnect() );
          }
          // Next step is to start the http web server
          // earlier this msg was sent first but now, it is triggered after checking the saved credentials
          wifi_app_send_msg( WIFI_APP_MSG_START_HTTP_SERVER );

          break;
        default:
          break;
      }
    }
  }
}

// initialize the wifi application event handler for WiFo and IP Events
static void wifi_app_event_handler_init( void )
{
  ESP_ERROR_CHECK( esp_event_loop_create_default() );

  esp_event_handler_instance_t instance_wifi_event;
  esp_event_handler_instance_t instance_ip_event;

  ESP_ERROR_CHECK( esp_event_handler_instance_register( WIFI_EVENT,               \
                                                        ESP_EVENT_ANY_ID,         \
                                                        &wifi_app_event_handler,  \
                                                        NULL,                     \
                                                        &instance_wifi_event) );
  ESP_ERROR_CHECK( esp_event_handler_instance_register( IP_EVENT,                 \
                                                        ESP_EVENT_ANY_ID,         \
                                                        &wifi_app_event_handler,  \
                                                        NULL,                     \
                                                        &instance_ip_event) );
}

static void wifi_app_default_wifi_init( void )
{
  // initialize TCP/IP stack
  ESP_ERROR_CHECK( esp_netif_init() );

  // default wifi config - operations must be in this order
  wifi_init_config_t cfg = WIFI_INIT_CONFIG_DEFAULT();

  ESP_ERROR_CHECK( esp_wifi_init(&cfg) );
  // NOTE: I am not sure if this is really needed
  // ESP_ERROR_CHECK( esp_wifi_set_storage( WIFI_STORAGE_RAM) );

  esp_netif_sta = esp_netif_create_default_wifi_sta();
  esp_netif_ap = esp_netif_create_default_wifi_ap();

  // logic for setting MAC address as hostname (starts)

  // This is an ESP32P4 device which doesn't have EFUSE, so esp_read_mac 
  // function will not work while esp_wifi_get_mac function will work because 
  // it will query the MAC programmed in the ESP32C6 WiFi chip
  // so call this function to get the MAC address programmed in the WiFi chip
  // and store it in wifi_mac_address variable, as we don't want to query again and again
  // make sure to call this after esp_wifi_init function and only once
  wifi_app_set_mac_address();

  esp_netif_t *netif = esp_netif_get_handle_from_ifkey("WIFI_STA_DEF");
  char mac_str[WIFI_MAC_ADDR_SIZE];
  char hostname[32];
  wifi_app_get_mac_address( mac_str );
  snprintf(hostname, sizeof(hostname), "E_%s", mac_str);
  ESP_ERROR_CHECK( esp_netif_set_hostname(netif, hostname) );
  ESP_LOGW( TAG, "Hostname set to MAC: %s", hostname );
  // logic for setting MAC address as hostname (ends)
}

static void wifi_app_soft_ap_config( void )
{
  // SoftAP- wifi access point configuration
  wifi_config_t ap_config =
  {
    .ap =
    {
      .ssid = WIFI_AP_SSID,
      .ssid_len = strlen(WIFI_AP_SSID),
      .password = WIFI_AP_PASSWORD,
      .max_connection = WIFI_AP_MAX_CONNECTIONS,
      .channel = WIFI_AP_CHANNEL,
      .ssid_hidden = WIFI_AP_SSID_HIDDEN,
      .authmode = WIFI_AUTH_WPA2_PSK,
      .beacon_interval = WIFI_AP_BEACON_INTERVAL,
    },
  };

  // Configure the DHCP for the AP
  esp_netif_ip_info_t ap_ip_info;
  memset(&ap_ip_info, 0x00, sizeof(ap_ip_info));

  // Stop the DHCP Server, this must be called first
  esp_netif_dhcps_stop(esp_netif_ap);

  // this function inet_pton converts the ip address in standard numeric form
  inet_pton(AF_INET, WIFI_AP_IP, &ap_ip_info.ip);       // Assign Access Point's Static IP, GW and NetMask
  inet_pton(AF_INET, WIFI_AP_GATEWAY, &ap_ip_info.gw);
  inet_pton(AF_INET, WIFI_AP_NETMASK, &ap_ip_info.netmask);

  // Statically Configures the network interface
  ESP_ERROR_CHECK(esp_netif_set_ip_info(esp_netif_ap, &ap_ip_info));
  // Start the AP DHCP Server (for connecting stations i.e. our mobile devices)
  ESP_ERROR_CHECK(esp_netif_dhcps_start(esp_netif_ap));

  // Set the mode as Access Point and Station Mode
  ESP_ERROR_CHECK(esp_wifi_set_mode(WIFI_MODE_APSTA));
  // Set our configuration
  ESP_ERROR_CHECK(esp_wifi_set_config(ESP_IF_WIFI_AP, &ap_config));
  // our Default bandwidth is 20MHz
  ESP_ERROR_CHECK(esp_wifi_set_bandwidth(WIFI_IF_AP, WIFI_AP_BANDWIDTH));
  ESP_ERROR_CHECK(esp_wifi_set_ps(WIFI_STA_POWER_SAVE));
}

/*
 * Connects the ESP32 to an external access point using the updated station
 * configuration
 */
static void wifi_app_connect_sta(void)
{
  wifi_config_t * wifi_config_ptr = wifi_app_get_wifi_config();
  if ( !wifi_config_ptr )
  {
    ESP_LOGE( TAG, "STA Connection Failed, as wifi_config is NULL!" );
    return;
  }
  
  ESP_LOGI( TAG, "STA Connection Starting" );
  ESP_ERROR_CHECK( esp_wifi_set_config(ESP_IF_WIFI_STA, wifi_config_ptr ) );
  // ESP_ERROR_CHECK( esp_wifi_connect() );   // This is too harsh and can crash the system
  esp_err_t err = esp_wifi_connect();
  if ( err != ESP_OK )
  {
    ESP_LOGE( TAG, "esp_wifi_connect: Failed" );
  }
}

/**
 * @brief WiFi Event Handler Function
 * @param arg data, aside from event data, that is passed to the handler when it is called
 * @param event_base Event Base whether WIFI Event or IP Event
 * @param event_id   Event ID
 * @param event_data Data with Event
 */
static void wifi_app_event_handler( void *arg, esp_event_base_t event_base, int32_t event_id, void * event_data )
{
  if( WIFI_EVENT == event_base )
  {
    switch( event_id )
    {
      case WIFI_EVENT_AP_START:
        ESP_LOGI( TAG, "WIFI_EVENT_AP_START");
        break;
      case WIFI_EVENT_AP_STOP:
        ESP_LOGI( TAG, "WIFI_EVENT_AP_STOP");
        break;
      case WIFI_EVENT_AP_STACONNECTED:
        ESP_LOGI( TAG, "WIFI_EVENT_AP_STACONNECTED");
        break;
      case WIFI_EVENT_AP_STADISCONNECTED:
        ESP_LOGI( TAG, "WIFI_EVENT_AP_STADISCONNECTED");
        break;
      case WIFI_EVENT_STA_START:
        ESP_LOGI( TAG, "WIFI_EVENT_STA_START");
        break;
      case WIFI_EVENT_STA_CONNECTED:
        ESP_LOGI( TAG, "WIFI_EVENT_STA_CONNECTED");
        break;
      case WIFI_EVENT_STA_DISCONNECTED:
        ESP_LOGI( TAG, "WIFI_EVENT_STA_DISCONNECTED");
        wifi_event_sta_disconnected_t *event = (wifi_event_sta_disconnected_t*)malloc(sizeof(wifi_event_sta_disconnected_t));
        *event = *((wifi_event_sta_disconnected_t*)event_data);
        ESP_LOGI(TAG, "WIFI_EVENT_STA_DISCONNECTED, Reason Code %d", event->reason);

        if( wifi_connect_retry < WIFI_MAX_CONNECTION_RETRIES )
        {
          esp_wifi_connect();
          wifi_connect_retry++;
        }
        else
        {
          wifi_app_send_msg( WIFI_APP_MSG_STA_DISCONNECTED );
        }
        break;
    }
  } // if( WIFI_EVENT = event_base )
  else if( IP_EVENT == event_base )
  {
    switch( event_id )
    {
      case IP_EVENT_STA_GOT_IP:
        ESP_LOGI( TAG, "IP_EVENT_STA_GOT_IP");
        /*
         * Need to change DNS because MQTT was not working, i.e. not able to connect
         * Some points/reasons:
         * - Router DNS works for some devices, not all
         * - ESP32's DNS Query Format or Timing wasn't compatible
         * - No Fallback DNS behavior
         */
        // log_dns_info();
        esp_netif_dns_info_t new_dns =
        {
            .ip.u_addr.ip4.addr = ipaddr_addr("8.8.8.8"),
            .ip.type = IPADDR_TYPE_V4,
        };

        esp_netif_t *netif = esp_netif_get_handle_from_ifkey("WIFI_STA_DEF");
        esp_netif_set_dns_info(netif, ESP_NETIF_DNS_MAIN, &new_dns);
        ESP_LOGI(TAG, "DNS manually set to 8.8.8.8");

        wifi_app_send_msg( WIFI_APP_MSG_STA_CONNECTED_GOT_IP );
        break;
    }
  }
}

