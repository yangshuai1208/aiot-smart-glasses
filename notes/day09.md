# Day09 学习记录

## 今日目标

- 确认 MPU6050 和 ESP32-S3 接线
- 跑通 I2C 通信
- 读取 MPU6050 WHO_AM_I 寄存器
- 唤醒 MPU6050
- 读取加速度和陀螺仪原始数据
- 串口打印 ACC_X、ACC_Y、ACC_Z、GYRO_X、GYRO_Y、GYRO_Z
- 完成 Git 提交

## 今日完成

- 新建 ESP-IDF 工程基础结构
- 新增 CMakeLists.txt
- 新增 main/CMakeLists.txt
- 编写 main/main.c
- 初始化 ESP32-S3 I2C 主机
- 读取 WHO_AM_I 寄存器
- 通过 PWR_MGMT_1 寄存器唤醒 MPU6050
- 从 ACCEL_XOUT_H 开始连续读取 14 字节原始数据
- 串口持续打印加速度、角速度和温度原始数据

## 今日代码提交

- commit: feat: read mpu6050 raw data over i2c

## 实验现象

- 串口显示 I2C init success
- 串口显示 WHO_AM_I = 0x68
- 串口持续输出 ACC_X / ACC_Y / ACC_Z
- 串口持续输出 GYRO_X / GYRO_Y / GYRO_Z
- 晃动 MPU6050 时，原始数据发生变化

## 遇到的问题

1. 现象：如果 WHO_AM_I 读取失败  
2. 可能原因：
   - SDA / SCL 接反
   - VCC / GND 没接好
   - MPU6050 地址不是 0x68
   - I2C 引脚宏定义和实际接线不一致
3. 解决方式：
   - 检查接线
   - 确认 VCC 使用 3.3V
   - 确认 GND 共地
   - 修改 I2C_MASTER_SDA_IO 和 I2C_MASTER_SCL_IO

## 今日知识点

- ESP32-S3 I2C 主机初始化
- MPU6050 I2C 地址
- WHO_AM_I 寄存器
- PWR_MGMT_1 唤醒寄存器
- ACCEL_XOUT_H 数据起始寄存器
- int16_t 高低字节合成
- 原始加速度和角速度数据读取
- FreeRTOS vTaskDelay 周期打印

## 面试可讲点

- MPU6050 通过 I2C 与 ESP32-S3 通信
- 先读取 WHO_AM_I 判断设备是否在线
- 通过 PWR_MGMT_1 将 MPU6050 从睡眠模式唤醒
- 从 0x3B 开始连续读取 14 字节数据，包含三轴加速度、温度和三轴角速度
- 后续可以基于 ACC_X、ACC_Y、ACC_Z 和 GYRO_X、GYRO_Y、GYRO_Z 进行姿态识别

## 明日计划

- 封装 mpu6050.c 和 mpu6050.h
- 将 MPU6050 初始化和读取逻辑从 main.c 中拆出来
- 为后续 gesture.c 姿态识别模块做准备