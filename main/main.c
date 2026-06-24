#include "freertos/FreeRTOS.h"
#include "freertos/task.h"
#include "esp_log.h"

#include "mpu6050.h"
#include "gesture.h"

static const char *TAG = "APP_MAIN";

void app_main(void)
{
    ESP_LOGI(TAG, "AIoT Smart Glasses Day11 Start");

    esp_err_t ret = mpu6050_init();
    if (ret != ESP_OK) {
        ESP_LOGE(TAG, "MPU6050 failed");
        return;
    }

    while (1) {

        mpu6050_raw_data_t data;
        
        ret=mpu6050_read_raw(&data);
        if(ret==ESP_OK){
            gesture_type_t gesture=gesture_detect(&data);
        
            ESP_LOGI(TAG,
                    "ACC_X=%d ACC_Y=%d ACC_Z=%d | GYRO_X=%d GYRO_Y=%d GYRO_Z=%d| TEMP=%.2f|Gesture=%s",
                    data.acc_x,
                    data.acc_y,
                    data.acc_z,
                    data.gyro_x,
                    data.gyro_y,
                    data.gyro_z,
                    data.temperature,
                    gesture_to_string(gesture));
                    
        }
        else{
            ESP_LOGE(TAG,"Read MPU6050 raw data failed");
        }
        vTaskDelay(pdMS_TO_TICKS(500));
    }
}