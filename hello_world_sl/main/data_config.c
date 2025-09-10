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
#define MAX_ACCESS_POINT_NAME_LEN         (32)
#define MAX_IP_ADDRESS_LEN                (16)
#define MAX_SERIAL_NUMBER_LEN             (32)
#define MAX_CONFIG_DATA_LEN               (MAX_ACCESS_POINT_NAME_LEN + MAX_IP_ADDRESS_LEN + MAX_SERIAL_NUMBER_LEN)

// Padding shouldn't be used for RAW data storage
typedef struct __attribute__((packed)) 
{
  char access_point_name[MAX_ACCESS_POINT_NAME_LEN];
  char ip_address[MAX_IP_ADDRESS_LEN];
  char serial_number[MAX_SERIAL_NUMBER_LEN];
} device_config_t;

// Private Variables
static const char *TAG = "CFG";
static device_config_t device_cfg;

// Public Function Definitions
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
  ESP_LOGI(TAG, "Access Point Name: %s", device_cfg.access_point_name);
  ESP_LOGI(TAG, "IP Address: %s", device_cfg.ip_address);
  ESP_LOGI(TAG, "Serial Number: %s", device_cfg.serial_number);
}

void cgf_get_access_point( char *access_point_name )
{
}

void cgf_get_ip_address( char *ip_address )
{
}

void cgf_get_serial_number( char *serial_number )
{

}