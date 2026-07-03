# 系统架构设计

## 项目名称

AIoT 智能眼镜 + 具身智能灵动手控制系统

## 总体架构

```text
ESP32-S3 智能眼镜控制端
        ↓ WiFi / MQTT
Linux Gateway 网关
        ↓ UART
STM32F407VET6 灵动手执行端
        ↓ I2C
PCA9685 16 路 PWM 驱动
        ↓ PWM
五个 MG90S 舵机