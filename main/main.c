#include "freertos/FreeRTOS.h"
#include "freertos/task.h"
#include "esp_log.h"

#include "mpu6050.h"
#include "gesture.h"
#include "oled_ui.h"

static const char *TAG = "APP_MAIN";

static const char*cmd_from_gesture(gesture_type_t gesture)
{
    switch(gesture){
        case GESTURE_LEFT:
        return "OPEN";

        case GESTURE_RIGHT:
        return "RELEASE";

        case GESTURE_NOD:
        return "GRAB";
        
        case GESTURE_SHAKE:
        return "NONE";

        case GESTURE_NONE:
        default:
        return "NONE";
        
    }
}

void app_main(void)
{
    ESP_LOGI(TAG, "AIoT Smart Glasses Day12 Start");

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

    while (1) {

        mpu6050_raw_data_t data;
        
        ret=mpu6050_read_raw(&data);
        if(ret==ESP_OK){
            gesture_type_t gesture=gesture_detect(&data);
            const char*cmd=cmd_from_gesture(gesture);

            ESP_LOGI(TAG,
                    "Gesture=%s Cmd=%s |ACC_X=%d ACC_Y=%d ACC_Z=%d",
                    gesture_to_string(gesture),
                    cmd,
                    data.acc_x,
                    data.acc_y,
                    data.acc_z
                    );
        oled_ui_show_status("CONTROL",gesture,cmd,"OK");

        }
        else{
            ESP_LOGE(TAG,"Read MPU6050 raw data failed");
            oled_ui_show_status("CONTROL",GESTURE_NONE,"NONE","ERR");
        }
        vTaskDelay(pdMS_TO_TICKS(500));
    }
}