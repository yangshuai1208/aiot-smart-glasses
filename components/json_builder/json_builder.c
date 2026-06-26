#include "json_builder.h"

#include <stdio.h>

#include "gesture.h"
#include "command.h"

esp_err_t json_builder_build(char*buf,
                            size_t len,
                            const char*device,
                            const char*mode,
                            gesture_type_t gesture,
                            command_type_t command,
                            const char*status
                        )
{
    if(buf==NULL||device==NULL||mode==NULL||status==NULL){
        return ESP_ERR_INVALID_ARG;
    }

    int written=snprintf(buf,
                         len,
                        "{\"device\":\"%s\",\"mode\":\"%s\",\"gesture\":\"%s\",\"cmd\":\"%s\",\"status\":\"%s\"}",
                        device,
                        mode,
                        gesture_to_string(gesture),
                        command_to_string(command),
                        status   );
if(written<0){

    return ESP_FAIL;
}
if((size_t)written>=len){
    return ESP_ERR_NO_MEM;
}

return ESP_OK;
}