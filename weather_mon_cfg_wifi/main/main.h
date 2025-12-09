/*
 * main.h
 *
 *  Created on: 10-Sep-2025
 *      Author: abc@xyz
 */

#ifndef MAIN_H
#define MAIN_H

#ifdef __cplusplus
extern "C" {
#endif

#include <stdio.h>
#include <stdbool.h>
#include <unistd.h>

#include "esp_log.h"
#include "freertos/idf_additions.h"

// Public Macros
#define SENSOR_BUFF_SIZE                        (100u)

// Data Structure
typedef struct _sensor_data_t
{
  uint8_t temperature_current;
  uint8_t humidity_current;
  uint8_t temperature[SENSOR_BUFF_SIZE];
  uint8_t humidity[SENSOR_BUFF_SIZE];
  size_t  sensor_idx;
} sensor_data_t;

// Enumeration
typedef enum 
{
  MAIN_EV_HTTP_SERVER_STARTED = 0,
  MAIN_EV_AP_LIST_AVAILABLE,
  MAIN_EV_AP_LIST_RESCAN,             // Request to re-scan for available APs
  MAIN_EV_STA_CONNECTED,              // Connected to WiFi Router
  MAIN_EV_START_INFLUXDB,
  MAIN_EV_GUI_REQ_USER_CONNECT,       // User has requested connect from GUI
  MAIN_EV_GUI_REQ_USER_DISCONNECT,    // User has requested disconnect from GUI
  MAIN_EV_STA_DISCONNECTED,           // Disconnected from WiFi Router
  MAIN_EV_DNS_REFRESH,                // Force DNS Refresh Timeout
  MAIN_EV_MAX,
} main_event_t;

// queue data structure
typedef struct _main_q_msg_t
{
  main_event_t  event_id;
  void          *data;
} main_q_msg_t;

// Public Function Definition
sensor_data_t * get_temperature_humidity( void );
long long get_time_ns( void );
BaseType_t main_send_event( main_event_t event, void *ptr_data );

#ifdef __cplusplus
}
#endif

#endif //