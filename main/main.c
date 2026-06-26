#include "freertos/FreeRTOS.h"
#include "freertos/task.h"
#include "esp_log.h"

#include "mpu6050.h"
#include "gesture.h"
#include "oled_ui.h"
#include "button.h"
#include "command.h"

static const char *TAG = "APP_MAIN";

typedef enum
{
    APP_MODE_NORMAL=0,
    APP_MODE_CONTROL
}app_mode_t;

static const char*mode_to_string(app_mode_t mode)
{
    switch(mode){
        case APP_MODE_CONTROL:
         return "CONTROL";

        case APP_MODE_NORMAL:
        default:
         return "NORMAL";
     }
}


void app_main(void)
{
    ESP_LOGI(TAG, "AIoT Smart Glasses Day14 Start");

    app_mode_t mode=APP_MODE_NORMAL;
    command_type_t command=COMMAND_NONE;
    const char*status="OK";


    esp_err_t ret = mpu6050_init();
    if (ret != ESP_OK) {
        ESP_LOGE(TAG, "MPU6050 failed");
        return;
    }

    ret=oled_ui_init();
    if(ret!=ESP_OK){
        ESP_LOGE(TAG,"OLED init failed");
        return;
    }


    ret = button_init();
    if (ret != ESP_OK) {
    ESP_LOGE(TAG, "Button init failed");
    return;
     }

    while (1) {
        button_event_t event=button_get_event();

        if(event==BUTTON_EVENT_SHORT_PRESS){
            if(mode==APP_MODE_NORMAL){
                mode=APP_MODE_CONTROL;
            }else{
                mode=APP_MODE_NORMAL;
            }
            ESP_LOGI(TAG,"Short press,mode=%s",mode_to_string(mode));
        }

        if(event==BUTTON_EVENT_LONG_PRESS)
        {
            command=COMMAND_STOP;
            status="STOP";

            ESP_LOGW(TAG,"Long press,command=%s",command_to_string(command));

            oled_ui_show_status(mode_to_string(mode),GESTURE_NONE,command_to_string(command),status);

            vTaskDelay(pdMS_TO_TICKS(500));
            continue;
        }

        mpu6050_raw_data_t data;
        
        ret=mpu6050_read_raw(&data);
        if(ret==ESP_OK){
            gesture_type_t gesture=gesture_detect(&data);

            if(mode==APP_MODE_CONTROL){
                command=command_from_gesture(gesture);
            }else{
                command=COMMAND_NONE;
            }
            status="OK";
            

            ESP_LOGI(TAG,
                    "Mode=%s Gesture=%s Command=%s |ACC_X=%d ACC_Y=%d ACC_Z=%d",
                    mode_to_string(mode),
                    gesture_to_string(gesture),
                    command_to_string(command),
                    data.acc_x,
                    data.acc_y,
                    data.acc_z
                    );
        oled_ui_show_status(mode_to_string(mode),gesture,command_to_string(command),status);

        }
        else{
            ESP_LOGE(TAG,"Read MPU6050 raw data failed");
            oled_ui_show_status(mode_to_string(mode),GESTURE_NONE,command_to_string(COMMAND_NONE),"ERR");
        }
        vTaskDelay(pdMS_TO_TICKS(500));
    }
}