/**
 * @file main_gen.c
 * @description Template source file for LVGL objects
 */

/*********************
 *      INCLUDES
 *********************/
#include "main_gen.h"
#include "ui.h"

/*********************
 *      DEFINES
 *********************/



/**********************
 *      TYPEDEFS
 **********************/

/***********************
 *  STATIC VARIABLES
 **********************/

/***********************
 *  STATIC PROTOTYPES
 **********************/

/**********************
 *   GLOBAL FUNCTIONS
 **********************/

lv_obj_t * main_create(void)
{
    LV_TRACE_OBJ_CREATE("begin");

    static lv_style_t main;

    static bool style_inited = false;

    if (!style_inited) {
        lv_style_init(&main);
        lv_style_set_text_font(&main, font_title);

        style_inited = true;
    }

    lv_obj_t * lv_obj_0 = lv_obj_create(NULL);
    lv_obj_set_width(lv_obj_0, lv_pct(100));
    lv_obj_set_height(lv_obj_0, lv_pct(100));
    lv_obj_add_style(lv_obj_0, &main, 0);

    lv_obj_t * lv_label_0 = lv_label_create(lv_obj_0);
    lv_label_set_text(lv_label_0, "Hello World Using LVGL Editor");
    lv_obj_set_align(lv_label_0, LV_ALIGN_CENTER);



    LV_TRACE_OBJ_CREATE("finished");

    // lv_obj_set_name(lv_obj_0, "main");

    return lv_obj_0;
}

/**********************
 *   STATIC FUNCTIONS
 **********************/