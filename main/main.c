
#include "freertos/FreeRTOS.h"
#include "freertos/task.h"

#include "esp_log.h"
#include "esp_app_desc.h"
#include "esp_system.h"

#include <stdbool.h>
#include <string.h>

#include "mpu6050.h"
#include "gesture.h"
#include "oled_ui.h"
#include "button.h"
#include "command.h"
#include "json_builder.h"
#include "uart_sender.h"
#include "wifi_manager.h"
#include "mqtt_sender.h"

#include "ota_manager.h"
#include "ota_downloader.h"

/* ===================== 宏定义 ===================== */

static const char *TAG = "day24_main";

#define DEVICE_NAME "smart_glasses_01"
#define JSON_BUF_SIZE 160

/*
 * Day08 OTA测试地址。
 *
 * 注意：这里必须是真实HTTPS URL，
 * 不能使用Markdown链接格式。
 */
#define OTA_FIRMWARE_URL \
    "https://github.com/yangshuai1208/aiot-smart-glasses/releases/download/v1.0.1/aiot-smart-glasses-v1.0.1.bin"

/* ===================== 工作模式 ===================== */

typedef enum
{
    APP_MODE_NORMAL = 0,
    APP_MODE_CONTROL
} app_mode_t;

static const char *mode_to_string(app_mode_t mode)
{
    switch (mode)
    {
    case APP_MODE_CONTROL:
        return "CONTROL";

    case APP_MODE_NORMAL:
    default:
        return "NORMAL";
    }
}

/* ===================== JSON状态上报 ===================== */

static void print_json_status(
    app_mode_t mode,
    gesture_type_t gesture,
    command_type_t command,
    const char *status)
{
    char json_buf[JSON_BUF_SIZE];

    esp_err_t ret =
        json_builder_build(
            json_buf,
            sizeof(json_buf),
            DEVICE_NAME,
            mode_to_string(mode),
            gesture,
            command,
            status);

    if (ret == ESP_OK)
    {
        ESP_LOGI(TAG, "JSON=%s", json_buf);

        ret = uart_sender_send_json(json_buf);

        if (ret != ESP_OK)
        {
            ESP_LOGE(
                TAG,
                "UART send JSON failed");
        }
    }
    else
    {
        ESP_LOGE(
            TAG,
            "Build JSON failed");
    }
}

/* ===================== MQTT命令发布 ===================== */

static void publish_mqtt_command(
    command_type_t command,
    int force)
{
    static command_type_t last_published_command =
        COMMAND_NONE;

    if (command == COMMAND_NONE)
    {
        last_published_command = COMMAND_NONE;
        return;
    }

    if (!force &&
        command == last_published_command)
    {
        return;
    }

    mqtt_sender_publish_cmd(
        command_to_string(command));

    last_published_command = command;
}

/* ===================== 主程序 ===================== */

void app_main(void)
{
    /* 1. 获取当前运行固件版本 */

    const esp_app_desc_t *app_desc =
        esp_app_get_description();

    ESP_LOGI(
        TAG,
        "Firmware version=%s",
        app_desc->version);

    ESP_LOGI(
        TAG,
        "ESP32-S3 AIoT application start");

    app_mode_t mode = APP_MODE_NORMAL;

    command_type_t command = COMMAND_NONE;

    const char *status = "OK";

    /* 2. 初始化Wi-Fi，等待成功连接 */

    wifi_manager_init_sta();

    /* 3. 启动MQTT客户端 */

    mqtt_sender_start();

    vTaskDelay(pdMS_TO_TICKS(3000));

    /* 4. 检查OTA分区 */

    esp_err_t ota_ret =
        ota_manager_print_partition_info();

    if (ota_ret != ESP_OK)
    {
        ESP_LOGE(
            TAG,
            "OTA partition check failed, stop upgrade");

        return;
    }

    /*
     * ==================================================
     * 5. Day08 OTA专项测试入口
     *
     * 只有V1.0.0执行本次升级。
     *
     * OTA测试不依赖MPU6050、OLED等外设初始化，
     * 避免传感器接线问题阻断固件下载实验。
     *
     * V1.0.1不会进入这个分支。
     * ==================================================
     */

    if (strcmp(app_desc->version, "1.0.0") == 0)
    {
        ESP_LOGW(
            TAG,
            "OTA test mode: skip peripheral init");

        ESP_LOGI(
            TAG,
            "Start OTA: V1.0.0 -> V1.0.1");

        esp_err_t upgrade_ret =
            ota_downloader_start(
                OTA_FIRMWARE_URL);

        if (upgrade_ret == ESP_OK)
        {
            ESP_LOGI(
                TAG,
                "OTA completed, restarting");

            esp_restart();
        }

        /*
         * OTA失败：
         * 不切换启动分区，不把失败当作成功。
         */
        ESP_LOGE(
            TAG,
            "OTA failed: %s",
            esp_err_to_name(upgrade_ret));

        return;
    }

    /*
     * ==================================================
     * 6. 正常固件初始化
     *
     * V1.0.1及其他版本进入这里。
     * ==================================================
     */

    esp_err_t ret = mpu6050_init();

    if (ret != ESP_OK)
    {
        ESP_LOGE(
            TAG,
            "MPU6050 init failed");

        ota_manager_check_and_confirm_app(false);

        return;
    }

    /* 7. OLED初始化 */

    bool oled_ready = false;

    ret = oled_ui_init();

    if (ret == ESP_OK)
    {
        oled_ready = true;
    }
    else
    {
        ESP_LOGW(
            TAG,
            "OLED init failed; bypassed for OTA test");
    }

    /* 8. 按键初始化 */

    ret = button_init();

    if (ret != ESP_OK)
    {
        ESP_LOGE(
            TAG,
            "Button init failed");

        ota_manager_check_and_confirm_app(false);

        return;
    }

    /* 9. UART发送模块初始化 */

    ret = uart_sender_init();

    if (ret != ESP_OK)
    {
        ESP_LOGE(
            TAG,
            "UART sender init failed");

        ota_manager_check_and_confirm_app(false);

        return;
    }

    ESP_LOGI(
        TAG,
        "Critical modules initialized, OLED ready=%d",
        (int)oled_ready);

    /*
     * 10. 固件自检通过后，确认当前OTA应用有效。
     *
     * OLED是本次实验允许的临时豁免项。
     * 不能将这一策略直接作为正式产品自检标准。
     */

    ret = ota_manager_check_and_confirm_app(true);

    if (ret != ESP_OK)
    {
        ESP_LOGE(
            TAG,
            "OTA app confirmation failed: %s",
            esp_err_to_name(ret));

        return;
    }

    /* ===================== 主循环 ===================== */

    while (1)
    {
        button_event_t event =
            button_get_event();

        /* 11. 短按：切换工作模式 */

        if (event == BUTTON_EVENT_SHORT_PRESS)
        {
            if (mode == APP_MODE_NORMAL)
            {
                mode = APP_MODE_CONTROL;
            }
            else
            {
                mode = APP_MODE_NORMAL;
            }

            ESP_LOGI(
                TAG,
                "Short press, mode=%s",
                mode_to_string(mode));
        }

        /* 12. 长按：发送STOP命令 */

        if (event == BUTTON_EVENT_LONG_PRESS)
        {
            command = COMMAND_STOP;
            status = "STOP";

            ESP_LOGW(
                TAG,
                "Long press, command=%s",
                command_to_string(command));

            /* OLED初始化成功才显示 */

            if (oled_ready)
            {
                oled_ui_show_status(
                    mode_to_string(mode),
                    GESTURE_NONE,
                    command_to_string(command),
                    status);
            }

            print_json_status(
                mode,
                GESTURE_NONE,
                command,
                status);

            publish_mqtt_command(
                command,
                1);

            vTaskDelay(pdMS_TO_TICKS(500));

            continue;
        }

        /* 13. 读取MPU6050原始数据 */

        mpu6050_raw_data_t data;

        ret = mpu6050_read_raw(&data);

        if (ret == ESP_OK)
        {
            gesture_type_t gesture =
                gesture_detect(&data);

            /* 14. 控制模式下生成动作命令 */

            if (mode == APP_MODE_CONTROL)
            {
                command =
                    command_from_gesture(gesture);
            }
            else
            {
                command = COMMAND_NONE;
            }

            status = "OK";

            ESP_LOGI(
                TAG,
                "Mode=%s Gesture=%s Command=%s | ACC_X=%d ACC_Y=%d ACC_Z=%d",
                mode_to_string(mode),
                gesture_to_string(gesture),
                command_to_string(command),
                data.acc_x,
                data.acc_y,
                data.acc_z);

            /* 15. OLED状态显示 */

            if (oled_ready)
            {
                oled_ui_show_status(
                    mode_to_string(mode),
                    gesture,
                    command_to_string(command),
                    status);
            }

            /* 16. JSON状态上报 */

            print_json_status(
                mode,
                gesture,
                command,
                status);

            /* 17. MQTT命令发布 */

            if (mode == APP_MODE_CONTROL)
            {
                publish_mqtt_command(
                    command,
                    0);
            }
        }
        else
        {
            /* 18. 传感器读取失败 */

            ESP_LOGE(
                TAG,
                "Read MPU6050 raw data failed");

            status = "ERR";

            command = COMMAND_NONE;

            if (oled_ready)
            {
                oled_ui_show_status(
                    mode_to_string(mode),
                    GESTURE_NONE,
                    command_to_string(COMMAND_NONE),
                    status);
            }

            print_json_status(
                mode,
                GESTURE_NONE,
                command,
                status);
        }

        vTaskDelay(pdMS_TO_TICKS(500));
    }
}
