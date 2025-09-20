#ifndef WIDGET_BUTTON_H_INCLUDED
#define WIDGET_BUTTON_H_INCLUDED

#include "widgets.h"

// Public Structures

// function pointer for slider callback function
typedef void (*w_button_cb_t) ( lv_event_t * e );

typedef struct 
{
  lv_obj_t * btn;
  lv_obj_t * btn_lbl;
  w_button_cb_t callback;
} w_button_t;

// Public Function Prototypes
w_button_t * w_button_create( lv_obj_t * parent, const char * label, w_button_cb_t cb );

#endif // WIDGET_BUTTON_H_INCLUDED
