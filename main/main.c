#include "freertos/FreeRTOS.h"
#include "freertos/task.h"
#include "esp_log.h"

#include "mpu6050.h"
#include "gesture.h"
#include "oled_ui.h"
#include "button.h"
#include "command.h"
#include "json_builder.h"
#include "uart_sender.h"
#include "wifi_manager.h"
#include "mqtt_sender.h"

static const char *TAG = "day24_main";

#define DEVICE_NAME "smart_glasses_01"
#define JSON_BUF_SIZE 160

typedef enum
{
    APP_MODE_NORMAL = 0,
    APP_MODE_CONTROL
} app_mode_t;

static const char *mode_to_string(app_mode_t mode)
{
    switch (mode) {
    case APP_MODE_CONTROL:
        return "CONTROL";

    case APP_MODE_NORMAL:
    default:
        return "NORMAL";
    }
}

static void print_json_status(app_mode_t mode,
                              gesture_type_t gesture,
                              command_type_t command,
                              const char *status)
{
    char json_buf[JSON_BUF_SIZE];

    esp_err_t ret = json_builder_build(json_buf,
                                       sizeof(json_buf),
                                       DEVICE_NAME,
                                       mode_to_string(mode),
                                       gesture,
                                       command,
                                       status);

    if (ret == ESP_OK) {
        ESP_LOGI(TAG, "JSON=%s", json_buf);

        ret = uart_sender_send_json(json_buf);
        if (ret != ESP_OK) {
            ESP_LOGE(TAG, "UART send JSON failed");
        }
    } else {
        ESP_LOGE(TAG, "Build JSON failed");
    }
}


static void publish_mqtt_command(command_type_t command, int force)
{
    static command_type_t last_published_command = COMMAND_NONE;

    if (command == COMMAND_NONE) {
        last_published_command = COMMAND_NONE;
        return;
    }

    if (!force && command == last_published_command) {
        return;
    }

    mqtt_sender_publish_cmd(command_to_string(command));
    last_published_command = command;
}

void app_main(void)
{
    ESP_LOGI(TAG, "Day24 ESP32-S3 MQTT publisher start");

    app_mode_t mode = APP_MODE_NORMAL;
    command_type_t command = COMMAND_NONE;
    const char *status = "OK";

 
    wifi_manager_init_sta();

    mqtt_sender_start();

 
    vTaskDelay(pdMS_TO_TICKS(3000));

    
    esp_err_t ret = mpu6050_init();
    if (ret != ESP_OK) {
        ESP_LOGE(TAG, "MPU6050 init failed");
        return;
    }

    ret = oled_ui_init();
    if (ret != ESP_OK) {
        ESP_LOGE(TAG, "OLED init failed");
        return;
    }

   
    ret = button_init();
    if (ret != ESP_OK) {
        ESP_LOGE(TAG, "Button init failed");
        return;
    }

  
    ret = uart_sender_init();
    if (ret != ESP_OK) {
        ESP_LOGE(TAG, "UART sender init failed");
        return;
    }

    ESP_LOGI(TAG, "All modules init OK");

    while (1) {
        button_event_t event = button_get_event();

    
        if (event == BUTTON_EVENT_SHORT_PRESS) {
            if (mode == APP_MODE_NORMAL) {
                mode = APP_MODE_CONTROL;
            } else {
                mode = APP_MODE_NORMAL;
            }

            ESP_LOGI(TAG, "Short press, mode=%s", mode_to_string(mode));
        }

        if (event == BUTTON_EVENT_LONG_PRESS) {
            command = COMMAND_STOP;
            status = "STOP";

            ESP_LOGW(TAG, "Long press, command=%s", command_to_string(command));

            oled_ui_show_status(mode_to_string(mode),
                                GESTURE_NONE,
                                command_to_string(command),
                                status);

            print_json_status(mode, GESTURE_NONE, command, status);

            publish_mqtt_command(command, 1);

            vTaskDelay(pdMS_TO_TICKS(500));
            continue;
        }

        /*
         * 读取 MPU6050 原始数据
         */
        mpu6050_raw_data_t data;

        ret = mpu6050_read_raw(&data);
        if (ret == ESP_OK) {
            gesture_type_t gesture = gesture_detect(&data);

            if (mode == APP_MODE_CONTROL) {
                command = command_from_gesture(gesture);
            } else {
                command = COMMAND_NONE;
            }

            status = "OK";

            ESP_LOGI(TAG,
                     "Mode=%s Gesture=%s Command=%s | ACC_X=%d ACC_Y=%d ACC_Z=%d",
                     mode_to_string(mode),
                     gesture_to_string(gesture),
                     command_to_string(command),
                     data.acc_x,
                     data.acc_y,
                     data.acc_z);

            oled_ui_show_status(mode_to_string(mode),
                                gesture,
                                command_to_string(command),
                                status);

            print_json_status(mode, gesture, command, status);

          
            if (mode == APP_MODE_CONTROL) {
                publish_mqtt_command(command, 0);
            }
        } else {
            ESP_LOGE(TAG, "Read MPU6050 raw data failed");

            status = "ERR";
            command = COMMAND_NONE;

            oled_ui_show_status(mode_to_string(mode),
                                GESTURE_NONE,
                                command_to_string(COMMAND_NONE),
                                status);

            print_json_status(mode, GESTURE_NONE, command, status);
        }

        vTaskDelay(pdMS_TO_TICKS(500));
    }
}