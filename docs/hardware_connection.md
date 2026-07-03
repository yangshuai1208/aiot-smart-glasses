# 硬件连接说明

## 1. 硬件清单

| 硬件 | 作用 |
|---|---|
| ESP32-S3 N16R8 | 智能眼镜控制端 |
| STM32F407VET6 | 灵动手执行端主控 |
| PCA9685 | 16 路 PWM 舵机驱动模块 |
| MG90S ×5 | 五指舵机 |
| MPU6050 | 姿态检测模块 |
| OLED | 状态显示 |
| USB-TTL | Linux Gateway 到 STM32 串口通信 |
| 5V 3A 电源 | 舵机独立供电 |
| 杜邦线 | 模块连接 |

---

## 2. ESP32-S3 连接

```text
ESP32-S3 USB → 电脑
ESP32-S3 连接 WiFi / 手机热点
电脑和 ESP32-S3 处于同一局域网
```

注意：

```text
ESP32-S3 不能使用 127.0.0.1 作为 MQTT Broker 地址
ESP32-S3 应该填写电脑 WLAN / 热点 IPv4 地址
```

正确示例：

```c
#define MQTT_BROKER_URI "mqtt://192.168.136.1:1883"
```

错误示例：

```c
#define MQTT_BROKER_URI "mqtt:// 192.168.136.1"
```

错误原因：

```text
mqtt:// 后面不能有空格
IP 后面建议加 :1883
```

---

## 3. Linux Gateway 到 STM32F407

USB-TTL 连接 STM32 USART1：

```text
USB-TTL TX  → STM32 PA10 USART1_RX
USB-TTL RX  → STM32 PA9  USART1_TX
USB-TTL GND → STM32 GND
```

注意：

```text
TX 接 RX
RX 接 TX
GND 必须共地
不要把 USB-TTL 的 5V 乱接到 STM32
```

串口参数：

```text
波特率：115200
数据位：8
校验位：None
停止位：1
流控：None
```

---

## 4. STM32F407 到 PCA9685

```text
STM32 PB6  → PCA9685 SCL
STM32 PB7  → PCA9685 SDA
STM32 3.3V → PCA9685 VCC
STM32 GND  → PCA9685 GND
```

说明：

```text
PB6 / PB7 使用 I2C1
PCA9685 VCC 是逻辑电源，接 3.3V
```

---

## 5. PCA9685 到舵机电源

```text
5V 3A 电源 +  → PCA9685 V+
5V 3A 电源 -  → PCA9685 GND
STM32 GND     → PCA9685 GND
```

注意：

```text
PCA9685 V+ 是舵机电源输入
舵机不能从 STM32 板子取电
STM32、PCA9685、舵机电源必须共地
```

---

## 6. PCA9685 到 MG90S 五舵机

```text
PCA9685 CH0 → 大拇指舵机
PCA9685 CH1 → 食指舵机
PCA9685 CH2 → 中指舵机
PCA9685 CH3 → 无名指舵机
PCA9685 CH4 → 小拇指舵机
```

每个 MG90S 舵机接线：

```text
信号线     → PCA9685 对应通道 Signal
红色线     → PCA9685 V+
棕/黑色线  → PCA9685 GND
```

---

## 7. 常见硬件问题

### 舵机不动

检查：

```text
1. PCA9685 V+ 是否接 5V 外部电源
2. PCA9685 VCC 是否接 STM32 3.3V
3. STM32、PCA9685、舵机电源是否共地
4. I2C 接线 PB6 / PB7 是否正确
5. 舵机是否插在 CH0 ~ CH4
```

### STM32 收不到命令

检查：

```text
1. USB-TTL TX 是否接 PA10
2. USB-TTL RX 是否接 PA9
3. GND 是否共地
4. 波特率是否 115200
5. Linux Gateway 是否发送 HAND_xxx\n
```

### 舵机抖动

检查：

```text
1. 舵机电源电流是否足够
2. 五个舵机是否同时堵转
3. 角度范围是否过大
4. 是否所有 GND 共地
```