/*
 * assets_mng.h
 *
 *  Created on: 10-Sep-2025
 *      Author: abc@xyz
 */

#ifndef ASSETS_MNG_H
#define ASSETS_MNG_H

#include "lvgl.h"

#ifdef __cplusplus
extern "C" {
#endif

// Public Function Prototypes
void assets_mng_init( void );
bool assets_mng_load_image( const char *path, lv_img_dsc_t *img_dsc );
void assets_mng_unload_image( lv_obj_t *img, lv_img_dsc_t *img_dsc );

#ifdef __cplusplus
}
#endif

#endif //