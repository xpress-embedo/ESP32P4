#ifndef WIDGET_SLIDER_H_INCLUDED
#define WIDGET_SLIDER_H_INCLUDED

#include "widgets.h"

// Public Structures

// function pointer for slider callback function
typedef void (*w_slider_cb_t) ( lv_event_t * e );

typedef struct 
{
  lv_obj_t * cont;
  lv_obj_t * sld;
  lv_obj_t * sld_dsc;
  lv_obj_t * sld_val;
  w_slider_cb_t callback;
} w_slider_t;

// Public Function Prototypes
w_slider_t * w_slider_create( lv_obj_t * parent, 
                              int min, int max, int init, 
                              w_slider_cb_t cb );

#endif // WIDGET_SLIDER_H_INCLUDED
