/*
 * gui_mng.h
 *
 *  Created on: 06-Sep-2025
 *      Author: abc@xyz
 */

#ifndef MAIN_GUI_MNG_H_
#define MAIN_GUI_MNG_H_

// Include Header Files
#include "freertos/FreeRTOS.h"
#include "gui_mng_cfg.h"

#include "bsp/esp-bsp.h"

// Public Macros

// GUI_LOCK shouldn't be called from ISR, use this macro to assert if called from ISR context
// #define GUI_LOCK()                               do { \
//                                                       assert(!xPortInIsrContext()); \
//                                                       bsp_display_lock(0); \
//                                                     } while(0)

#define GUI_LOCK()                            bsp_display_lock(0)
#define GUI_UNLOCK()                          bsp_display_unlock()

typedef struct _gui_q_msg_t {
  gui_mng_event_t   event_id;
  void              *data;
} gui_q_msg_t;

// Public Function Prototypes
void gui_start( void );
BaseType_t gui_send_event( gui_mng_event_t event, void *pData );

#endif /* MAIN_GUI_MNG_H_ */
