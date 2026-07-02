Day25 学习记录：Linux Gateway MQTT 订阅与命令解析

1. 使用 libmosquitto 编写 Linux MQTT Gateway
2. 订阅 Topic：aiot/glasses/cmd
3. 接收 ESP32-S3 发布的 JSON 命令
4. 从 JSON 中提取 cmd 字段
5. 将 OPEN / GRAB / RELEASE / STOP 映射为 STM32 协议
6. 当前只打印 HAND_OPEN 等命令
7. Day26 将把 HAND_xxx 通过串口发送给 STM32F407