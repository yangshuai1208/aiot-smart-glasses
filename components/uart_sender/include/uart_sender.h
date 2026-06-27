#ifndef UART_SENDER_H
#define UART_SENDER_H

#include "esp_err.h"

esp_err_t uart_sender_init(void);
esp_err_t uart_sender_send_json(const char*json);

#endif
