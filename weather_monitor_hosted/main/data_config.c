/*
 * data_config.c
 *
 *  Created on: 10-Sep-2025
 *      Author: abc@xyz
 */

#include "data_config.h"

#include "esp_log.h"
#include "esp_partition.h"

// Private Macros
// Uncomment this line to print all the data read from configuration
#define DATA_CONFIG_DEBUG_EN

#define MAX_ACCESS_POINT_NAME_LEN         (32)
#define MAX_IP_ADDRESS_LEN                (16)
#define MAX_SERIAL_NUMBER_LEN             (32)
#define MAX_CONFIG_DATA_LEN               (MAX_ACCESS_POINT_NAME_LEN + MAX_IP_ADDRESS_LEN + MAX_SERIAL_NUMBER_LEN)

// Padding shouldn't be used for RAW data storage
typedef struct __attribute__((packed)) 
{
  char ap_name[MAX_ACCESS_POINT_NAME_LEN];
  char ap_ip[MAX_IP_ADDRESS_LEN];
  char serial_number[MAX_SERIAL_NUMBER_LEN];
} device_config_t;

// Private Variables
static const char *TAG = "CFG";
static bool is_initialized = false;
static device_config_t device_cfg;

// Public Function Definitions

/**
 * @brief This function initializes the configuration data by reading from the "config" partition.
 * @param  None
 */
void cfg_init( void )
{
  const esp_partition_t* config_partition = esp_partition_find_first( ESP_PARTITION_TYPE_DATA, 0x40, "config" );

  if ( !config_partition )
  {
    ESP_LOGE( TAG, "Config partition not found!" );
    return;
  }

  esp_err_t err = esp_partition_read( config_partition, 0, &device_cfg, sizeof(device_cfg) );
  if ( err != ESP_OK )
  {
    ESP_LOGE( TAG, "Failed to read config data: %s", esp_err_to_name(err) );
    return;
  }
  is_initialized = true;
  #ifdef DATA_CONFIG_DEBUG_EN
  ESP_LOGW(TAG, "Access Point Name: %s", device_cfg.ap_name);
  ESP_LOGW(TAG, "IP Address: %s", device_cfg.ap_ip);
  ESP_LOGW(TAG, "Serial Number: %s", device_cfg.serial_number);
  #endif
}

/**
 * @brief This function returns the device default access point name
 * @param access_point_name 
 */
void cfg_get_access_point( char *access_point_name )
{
  if ( is_initialized )
  {
    strncpy(access_point_name, device_cfg.ap_name, MAX_ACCESS_POINT_NAME_LEN);
  }
}

/**
 * @brief This function returns the device default access point IP address
 * @param ip_address 
 */
void cfg_get_ip_address( char *ip_address )
{
  if ( is_initialized )
  {
    strncpy(ip_address, device_cfg.ap_ip, MAX_IP_ADDRESS_LEN);
  }
}

/**
 * @brief This function returns the device serial number
 * @param serial_number 
 */
void cfg_get_serial_number( char *serial_number )
{
  if ( is_initialized )
  {
    strncpy(serial_number, device_cfg.serial_number, MAX_SERIAL_NUMBER_LEN);
  }
}

// Private Function Definitions
// todo
