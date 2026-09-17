#pragma once

#include "esp_err.h"
#include <stddef.h>
#include <stdint.h>

esp_err_t ota_writer_begin(void);

esp_err_t ota_writer_write(const uint8_t* data,size_t len);

esp_err_t ota_writer_finish(void);