/*
 * wifi_sta_mng.h
 *
 *  Created on: Sep 15, 2025
 *      Author: abc@xyz
 */

#ifndef WIFI_STA_MNG_H_
#define WIFI_STA_MNG_H_

// Public Macros
#define MAC_ADDR_SIZE                           (18u)


// Public Function Prototypes
void wifi_sta_connect( void );
bool wifi_sta_is_connected( void );
void wifi_sta_get_mac_address( char *mac_address );

#endif /* WIFI_STA_MNG_H_ */