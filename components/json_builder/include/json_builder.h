#ifndef JSON_BUILDER_H
#define JSON_BUILDER_H

#include <stddef.h>

#include "esp_err.h"
#include "gesture.h"
#include "command.h"

esp_err_t json_builder_build(char*buf,
                            size_t len,
                            const char*device,
                            const char*mode,
                            gesture_type_t gesture,
                            command_type_t command,
                            const char*status
                        );
#endif
    