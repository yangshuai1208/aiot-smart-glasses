#ifndef MPU6050_H
#define MPU6050_H

#include <stdint.h>
#include "esp_err.h"

typedef struct{
    int16_t acc_x;
    int16_t acc_y;
    int16_t acc_z;

    int16_t gyro_x;
    int16_t gyro_y;
    int16_t gyro_z;

    float temperature;
}mpu6050_raw_data_t;

esp_err_t mpu6050_init(void);

esp_err_t mpu6050_read_raw(mpu6050_raw_data_t *data);

#endif