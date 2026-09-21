#pragma once

#include "esp_err.h"
#include <stdbool.h>

esp_err_t ota_manager_print_partition_info(void);

esp_err_t ota_manager_check_and_confirm_app(bool self_test_ok);
