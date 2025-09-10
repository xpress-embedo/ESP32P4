/*
 * data_config.h
 *
 *  Created on: 10-Sep-2025
 *      Author: abc@xyz
 */

#ifndef DATA_CONFIG_H
#define DATA_CONFIG_H

#ifdef __cplusplus
extern "C" {
#endif

// Public Function Prototypes
void cfg_init( void );
void cgf_get_access_point( char *access_point_name );
void cgf_get_ip_address( char *ip_address );
void cgf_get_serial_number( char *serial_number );

#ifdef __cplusplus
}
#endif

#endif //