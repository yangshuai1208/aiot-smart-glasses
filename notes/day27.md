# Day27 学习记录：完整链路联调

## 今日目标

完成 ESP32-S3 智能眼镜控制端到 STM32F407 灵动手执行端的完整链路联调。

## 完整链路

ESP32-S3 手势识别
        ↓ MQTT publish
Linux Gateway MQTT subscribe
        ↓ JSON 解析
OPEN / GRAB / RELEASE / STOP
        ↓ 协议转换
HAND_OPEN / HAND_GRAB / HAND_RELEASE / HAND_STOP
        ↓ UART
STM32F407 USART1 接收
        ↓ I2C
PCA9685
        ↓ PWM
MG90S 五舵机动作

## 今日完成内容

1. 启动 Mosquitto Broker
2. 启动 Linux MQTT Gateway
3. 打开 STM32 串口转发
4. 启动 ESP32-S3 MQTT 发布端
5. ESP32-S3 短按进入 CONTROL 模式
6. 手势触发 OPEN / GRAB / RELEASE / STOP
7. Linux Gateway 成功收到 MQTT JSON
8. Gateway 成功通过 UART 发送 HAND_xxx
9. STM32F407 成功接收命令
10. PCA9685 成功驱动五个 MG90S 舵机动作

## 今日问题记录

- 若 ESP32 MQTT disconnected，优先检查 Broker IP 和网络。
- 若 Gateway 收不到，检查 Topic 和 Broker 是否一致。
- 若 UART SEND 有但 STM32 不动，检查 TX/RX、GND、串口号、波特率和 \n。
- 若舵机抖动，检查外部 5V 电源和共地。