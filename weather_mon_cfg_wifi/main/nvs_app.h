/*
 * nvs_app.h
 *
 *  Created on: Aug 8, 2025
 *      Author: abc@xyz
 */

#ifndef NVS_APP_H_
#define NVS_APP_H_

#include <stdio.h>
#include <string.h>
#include <stdio.h>

#include "esp_err.h"

// Public Function Prototypes
esp_err_t nvs_app_save_sta_creds( void );
bool nvs_app_load_sta_creds( void );
esp_err_t nvs_app_clear_sta_creds( void );
esp_err_t nvs_app_save_display_information( uint8_t brightness, uint8_t contrast );
bool nvs_app_load_display_information( uint8_t *brightness, uint8_t *contrast );
esp_err_t nvs_app_clear_display_information( void );

#endif /* NVS_MNG_H_ */
