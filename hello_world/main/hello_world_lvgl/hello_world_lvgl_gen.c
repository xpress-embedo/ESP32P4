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
lv_obj_t * main_screen;

/*----------------
 * Global styles
 *----------------*/

/*----------------
 * Fonts
 *----------------*/


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

    main_screen = main_screen_create();
#endif
}

/* callbacks */

/**********************
 *   STATIC FUNCTIONS
 **********************/