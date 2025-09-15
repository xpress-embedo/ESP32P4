/*
 * app_mng.h
 *
 *  Created on: Aug 8, 2025
 *      Author: abc@xyz
 */

#ifndef NVS_MNG_H_
#define NVS_MNG_H_

#include <stdio.h>
#include <string.h>
#include <stdio.h>

// Public Function Prototypes
esp_err_t nvs_save_sta_creds( void );
bool nvs_load_sta_creds( void );
esp_err_t nvs_clear_sta_creds( void );

#endif /* NVS_MNG_H_ */
