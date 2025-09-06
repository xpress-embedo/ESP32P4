/*
 * gui_mng_cfg.h
 *
 *  Created on: 06-Sep-2025
 *      Author: abc@xyz
 */

#ifndef MAIN_GUI_MNG_CFG_H_
#define MAIN_GUI_MNG_CFG_H_

#include <stdint.h>

// Public Macros
#define GUI_MNG_REFRESH_TIME            (20)    // in milliseconds

// Enumeration
typedef enum {
  GUI_MNG_EV_NONE = 0,
  GUI_MNG_EV_HELLO_WORLD,               // Event for Hello World LVGL
  GUI_MNG_EV_MAX,
} gui_mng_event_t;


// Public Function Prototypes
void gui_cfg_init( void );
void gui_cfg_mng_process( gui_mng_event_t event, void *data );
void gui_cfg_refresh( void );

#endif /* MAIN_GUI_MNG_CFG_H_ */
