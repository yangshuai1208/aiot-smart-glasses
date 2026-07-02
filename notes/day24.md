1. 使用 ESP32-S3 作为智能眼镜控制端
2. ESP32-S3 通过 WiFi 连接到局域网
3. 使用 ESP-MQTT 连接 MQTT Broker
4. 发布 Topic：aiot/glasses/cmd
5. 发布 JSON 命令：OPEN / GRAB / RELEASE / STOP
6. Linux 端通过 mosquitto_sub 成功接收 MQTT 消息
7. Day25 将实现 Linux Gateway MQTT 订阅并转发到 STM32