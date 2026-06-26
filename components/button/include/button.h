#ifndef BUTTON_H
#define BUTTON_H

#include "esp_err.h"

typedef enum
{
    BUTTON_EVENT_NONE=0,
    BUTTON_EVENT_SHORT_PRESS,
    BUTTON_EVENT_LONG_PRESS
}button_event_t;

esp_err_t button_init(void);
button_event_t button_get_event(void);

#endif