# Day26 学习记录：MQTT 到 STM32 串口转发

今天完成 Linux Gateway 的 MQTT 到串口转发功能。

## 完成内容

1. 新增 serial_sender 模块
2. 使用 termios 配置 Linux 串口
3. 串口参数为 115200 8N1
4. MQTT 收到 OPEN 后发送 HAND_OPEN\n
5. MQTT 收到 GRAB 后发送 HAND_GRAB\n
6. MQTT 收到 RELEASE 后发送 HAND_RELEASE\n
7. MQTT 收到 STOP 后发送 HAND_STOP\n
8. STM32F407 接收串口命令后控制 PCA9685 和五个 MG90S 舵机动作

## 今日链路

ESP32-S3 / mosquitto_pub
        ↓ MQTT
Linux Gateway
        ↓ UART
STM32F407
        ↓ I2C
PCA9685
        ↓ PWM
MG90S 舵机