/**
 * @file hello_world_lvgl_gen.c
 */

/*********************
 *      INCLUDES
 *********************/
#include "hello_world_lvgl_gen.h"

#if LV_USE_XML
#endif

/*********************
 *      DEFINES
 *********************/

/**********************
 *      TYPEDEFS
 **********************/

/**********************
 *  STATIC PROTOTYPES
 **********************/

/**********************
 *  STATIC VARIABLES
 **********************/

/*----------------
 * Translations
 *----------------*/

/**********************
 *  GLOBAL VARIABLES
 **********************/

/*--------------------
 *  Permanent screens
 *-------------------*/

/*----------------
 * Global styles
 *----------------*/

/*----------------
 * Fonts
 *----------------*/
lv_font_t * font_title;
extern uint8_t Inter_SemiBold_ttf_data[];
extern size_t Inter_SemiBold_ttf_data_size;

/*----------------
 * Images
 *----------------*/

/*----------------
 * Subjects
 *----------------*/

/**********************
 *      MACROS
 **********************/

/**********************
 *   GLOBAL FUNCTIONS
 **********************/

void hello_world_lvgl_init_gen(const char * asset_path)
{
    char buf[256];

    /*----------------
     * Global styles
     *----------------*/

    /*----------------
     * Fonts
     *----------------*/
    /* create tiny ttf font 'font_title' from C array */
    font_title = lv_tiny_ttf_create_data(Inter_SemiBold_ttf_data, Inter_SemiBold_ttf_data_size, 42);

    /*----------------
     * Images
     *----------------*/


    /*----------------
     * Subjects
     *----------------*/

    /*----------------
     * Translations
     *----------------*/


#if LV_USE_XML
    /*Register widgets*/

    /* Register fonts */
    lv_xml_register_font(NULL, "font_title", font_title);

    /* Register subjects */

    /* Register callbacks */
#endif

    /* Register all the global assets so that they won't be created again when globals.xml is parsed.
     * While running in the editor skip this step to update the preview when the XML changes */
#if LV_USE_XML && !defined(LV_EDITOR_PREVIEW)

    /* Register images */
#endif

#if LV_USE_XML == 0
    /*--------------------
    *  Permanent screens
    *-------------------*/

    /*If XML is enabled it's assumed that the permanent screens are created
     *manaully from XML using lv_xml_create()*/

#endif
}

/* callbacks */

/**********************
 *   STATIC FUNCTIONS
 **********************/