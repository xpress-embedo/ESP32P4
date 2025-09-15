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

// Public Function Definition
sensor_data_t * get_temperature_humidity( void );
long long get_time_ns( void );

#ifdef __cplusplus
}
#endif

#endif //