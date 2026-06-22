#include <stdio.h>
#include <stdint.h>
#include "freertos/FreeRTOS.h"
#include "freertos/task.h"
#include "driver/i2c.h"
#include "driver/gpio.h"
#include "esp_log.h"

#define I2C_MASTER_SCL_IO           9
#define I2C_MASTER_SDA_IO           8
#define I2C_MASTER_NUM              I2C_NUM_0
#define I2C_MASTER_FREQ_HZ          100000
#define I2C_MASTER_TX_BUF_DISABLE   0
#define I2C_MASTER_RX_BUF_DISABLE   0

#define MPU6050_ADDR                0x68
#define MPU6050_WHO_AM_I_REG        0x75
#define MPU6050_PWR_MGMT_1_REG      0x6B
#define MPU6050_ACCEL_XOUT_H_REG    0x3B

static const char *TAG = "MPU6050";

static esp_err_t i2c_master_init(void)
{
    i2c_config_t conf = {
        .mode = I2C_MODE_MASTER,
        .sda_io_num = I2C_MASTER_SDA_IO,
        .scl_io_num = I2C_MASTER_SCL_IO,
        .sda_pullup_en = GPIO_PULLUP_ENABLE,
        .scl_pullup_en = GPIO_PULLUP_ENABLE,
        .master.clk_speed = I2C_MASTER_FREQ_HZ,
    };

    esp_err_t ret = i2c_param_config(I2C_MASTER_NUM, &conf);
    if (ret != ESP_OK) {
        return ret;
    }

    return i2c_driver_install(I2C_MASTER_NUM,
                              conf.mode,
                              I2C_MASTER_RX_BUF_DISABLE,
                              I2C_MASTER_TX_BUF_DISABLE,
                              0);
}

static esp_err_t mpu6050_write_byte(uint8_t reg_addr, uint8_t data)
{
    uint8_t write_buf[2] = {reg_addr, data};

    return i2c_master_write_to_device(I2C_MASTER_NUM,
                                      MPU6050_ADDR,
                                      write_buf,
                                      sizeof(write_buf),
                                      pdMS_TO_TICKS(1000));
}

static esp_err_t mpu6050_read_bytes(uint8_t reg_addr, uint8_t *data, size_t len)
{
    return i2c_master_write_read_device(I2C_MASTER_NUM,
                                        MPU6050_ADDR,
                                        &reg_addr,
                                        1,
                                        data,
                                        len,
                                        pdMS_TO_TICKS(1000));
}

static esp_err_t mpu6050_init(void)
{
    uint8_t who_am_i = 0;

    esp_err_t ret = mpu6050_read_bytes(MPU6050_WHO_AM_I_REG, &who_am_i, 1);
    if (ret != ESP_OK) {
        ESP_LOGE(TAG, "Failed to read WHO_AM_I");
        return ret;
    }

    ESP_LOGI(TAG, "WHO_AM_I = 0x%02X", who_am_i);

    if (who_am_i != 0x68) {
        ESP_LOGW(TAG, "Unexpected WHO_AM_I value, check wiring or address");
    }

    ret = mpu6050_write_byte(MPU6050_PWR_MGMT_1_REG, 0x00);
    if (ret != ESP_OK) {
        ESP_LOGE(TAG, "Failed to wake up MPU6050");
        return ret;
    }

    ESP_LOGI(TAG, "MPU6050 wake up success");

    return ESP_OK;
}

static void mpu6050_read_raw_data(void)
{
    uint8_t raw_data[14];

    esp_err_t ret = mpu6050_read_bytes(MPU6050_ACCEL_XOUT_H_REG,
                                       raw_data,
                                       14);
    if (ret != ESP_OK) {
        ESP_LOGE(TAG, "Failed to read raw data");
        return;
    }

    int16_t acc_x = (int16_t)((raw_data[0] << 8) | raw_data[1]);
    int16_t acc_y = (int16_t)((raw_data[2] << 8) | raw_data[3]);
    int16_t acc_z = (int16_t)((raw_data[4] << 8) | raw_data[5]);

    int16_t temp_raw = (int16_t)((raw_data[6] << 8) | raw_data[7]);

    int16_t gyro_x = (int16_t)((raw_data[8] << 8) | raw_data[9]);
    int16_t gyro_y = (int16_t)((raw_data[10] << 8) | raw_data[11]);
    int16_t gyro_z = (int16_t)((raw_data[12] << 8) | raw_data[13]);

    float temp = (temp_raw / 340.0f) + 36.53f;

    ESP_LOGI(TAG,
             "ACC_X=%d ACC_Y=%d ACC_Z=%d | GYRO_X=%d GYRO_Y=%d GYRO_Z=%d | TEMP=%.2f",
             acc_x,
             acc_y,
             acc_z,
             gyro_x,
             gyro_y,
             gyro_z,
             temp);
}

void app_main(void)
{
    ESP_LOGI(TAG, "AIoT Smart Glasses Day09 Start");

    esp_err_t ret = i2c_master_init();
    if (ret != ESP_OK) {
        ESP_LOGE(TAG, "I2C init failed");
        return;
    }

    ESP_LOGI(TAG, "I2C init success");

    ret = mpu6050_init();
    if (ret != ESP_OK) {
        ESP_LOGE(TAG, "MPU6050 init failed");
        return;
    }

    while (1) {
        mpu6050_read_raw_data();
        vTaskDelay(pdMS_TO_TICKS(500));
    }
}