#ifndef OLED_UI_H
#define OLED_UI_H

#include "esp_err.h"
#include "gesture.h"

esp_err_t oled_ui_init(void);

esp_err_t oled_ui_show_status(const char*mode,
                            gesture_type_t gesture,
                            const char*cmd,
                            const char*status);
#endif    