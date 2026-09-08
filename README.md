# AIoT 智能眼镜姿态控制端

## 1. 项目简介

本项目是 AIoT 智能眼镜与灵动手控制系统中的可穿戴控制端，基于 ESP32-S3 开发。

ESP32-S3 通过 MPU6050 采集头部姿态数据，识别 LEFT、RIGHT、NOD、SHAKE 等动作，并映射为 OPEN、RELEASE、GRAB 等控制命令。

系统支持 NORMAL / CONTROL 两种工作模式：

- NORMAL：仅进行姿态识别和本地状态显示
- CONTROL：将有效姿态转换为控制命令并发送
- 长按按键：生成 STOP 命令

当前 ESP32 端同时实现：

1. WiFi + MQTT 命令发布
2. UART JSON 调试/备用输出
3. OLED 本地状态反馈

---

## 2. 系统定位

完整系统由三个主要节点组成：

```text
ESP32-S3 智能眼镜
        ↓
   WiFi / MQTT
        ↓
Linux IoT Gateway
        ↓
      UART
        ↓
STM32F407 灵动手执行端
        ↓
      I2C
        ↓
     PCA9685
        ↓
    MG90S × 5
```

本仓库只负责 **ESP32-S3 智能眼镜控制端**。

STM32 执行端和 Linux 网关分别维护在独立仓库中。

---

## 3. 硬件组成

| 硬件 | 作用 |
|---|---|
| ESP32-S3 | 主控制器、WiFi、MQTT |
| MPU6050 | 加速度与角速度采集 |
| 0.96 寸 OLED | 显示模式、姿态和命令 |
| 按键 | 模式切换及 STOP |
| USB / 串口 | 日志和 UART JSON 输出 |

### I2C

MPU6050 与 OLED 共用 I2C 总线，通过不同从机地址区分设备。

```text
MPU6050 → 0x68
OLED    → 0x3C
```

### UART1

```text
TX       GPIO17
RX       GPIO18
Baudrate 115200
Data     8 bit
Parity   None
Stop     1 bit
```

---

## 4. 软件架构

主要组件：

```text
components/
├── mpu6050/       # MPU6050 驱动
├── gesture/       # 姿态识别
├── command/       # gesture → command
├── oled_ui/       # OLED 状态显示
├── button/        # 按键事件
├── json_builder/  # UART JSON 构造
├── uart_sender/   # UART JSON 输出
├── wifi_manager/  # WiFi STA
└── mqtt_sender/   # MQTT 命令发布
```

主程序：

```text
main/main.c
```

核心处理流程：

```text
MPU6050
   ↓
读取加速度 / 角速度
   ↓
gesture_detect()
   ↓
LEFT / RIGHT / NOD / SHAKE
   ↓
command_from_gesture()
   ↓
OPEN / RELEASE / GRAB / NONE
   ↓
OLED 显示
   ├──────────────→ UART JSON
   │
   └── CONTROL模式 → MQTT 发布
```

---

## 5. 姿态与命令映射

| Gesture | Command | 含义 |
|---|---|---|
| LEFT | OPEN | 张开灵动手 |
| RIGHT | RELEASE | 释放 |
| NOD | GRAB | 抓取 |
| SHAKE | NONE | 当前不执行 |
| NONE | NONE | 无动作 |
| BUTTON_LONG | STOP | 生成停止命令 |

姿态识别与控制命令分层处理：

```text
gesture
↓
command
```

避免在主程序中直接耦合具体动作逻辑。

---

## 6. 工作模式

### NORMAL

```text
读取姿态
→ OLED 显示
→ 不生成机械手控制动作
```

### CONTROL

```text
读取姿态
→ gesture识别
→ command映射
→ MQTT发布控制命令
```

短按按键切换：

```text
NORMAL ↔ CONTROL
```

长按按键：

```text
STOP
```

并强制发布 STOP 命令。

---

## 7. MQTT 通信

ESP32 使用 ESP-IDF MQTT Client。

Broker 地址由当前实验网络配置决定。

控制 Topic：

```text
aiot/glasses/cmd
```

MQTT Payload 示例：

```json
{
  "device": "glasses01",
  "cmd": "GRAB",
  "seq": 15
}
```

当前发布参数：

```text
QoS    = 1
Retain = 0
```

其中：

- `cmd`：控制命令
- `seq`：ESP32 侧递增业务序号
- QoS1：至少一次交付，因此接收端仍需要考虑重复消息
- Retain=0：避免旧控制命令在重新订阅后被再次执行

为了减少相同姿态持续触发造成的重复发布，程序记录上一次发送的命令；命令没有变化时不重复发送。

STOP 属于安全控制命令，因此允许强制发送。

---

## 8. UART JSON 输出

除 MQTT 外，当前程序还保留 UART1 JSON 输出。

UART JSON 主要用于：

- 开发调试
- 观察姿态识别结果
- 验证协议字段
- 无 MQTT 环境下的链路实验

示例：

```json
{
  "device": "smart_glasses_01",
  "mode": "CONTROL",
  "gesture": "NOD",
  "cmd": "GRAB",
  "status": "OK"
}
```

UART1 参数：

```text
115200 8N1
TX = GPIO17
RX = GPIO18
```

因此需要区分：

```text
MQTT
→ 当前无线控制命令发布方式

UART JSON
→ 调试 / 备用通信方式
```

---

## 9. 关键代码

### 姿态转命令

```c
command_type_t command_from_gesture(gesture_type_t gesture);
```

例如：

```text
GESTURE_LEFT  → COMMAND_OPEN
GESTURE_RIGHT → COMMAND_RELEASE
GESTURE_NOD   → COMMAND_GRAB
```

### MQTT 发布

```c
mqtt_sender_publish_cmd(command_to_string(command));
```

### UART JSON

```c
uart_sender_send_json(json_buf);
```

### OLED

OLED 实时显示：

```text
Mode
Gesture
Command
Status
```

---

## 10. 项目中的设计点

### 为什么使用 MQTT？

相比直接把 ESP32 与执行端绑定，MQTT 可以将：

```text
感知端
网关
执行端
```

解耦。

ESP32 只负责产生控制命令，具体协议转换和设备接入可以交给 Linux Gateway。

### 为什么保留 UART？

UART 更适合：

- 硬件早期联调
- 无网络调试
- 观察原始 JSON
- 定位 MQTT 与姿态识别之间的问题

因此 UART 和 MQTT 在当前项目中承担不同用途。

### 为什么控制命令不使用 Retain？

控制动作具有时效性。

例如旧的 `GRAB` 如果被 Broker 保留，设备重新上线后可能再次收到并执行，因此当前控制消息设置：

```text
Retain = 0
```

---

## 11. 当前实现边界

当前本仓库已经实现：

```text
MPU6050姿态采集
→ gesture识别
→ command映射
→ OLED反馈
→ UART JSON输出

以及

ESP32 WiFi
→ MQTT QoS1命令发布
```

需要明确：

1. 本仓库证明的是 **ESP32 MQTT 发布端**，完整的 MQTT → Linux → STM32 端到端可靠闭环需要结合其他仓库说明。
2. `seq` 已加入 MQTT Payload，但 ACK 超时重传、执行端去重等可靠通信机制主要在独立实验中验证，不能描述为已经全部接入本硬件链路。
3. STOP 在 ESP32 端表示高优先级停止命令的生成与强制发布；最终机械执行是否立即抢占由 STM32 执行端实现决定。
4. UART JSON 与 MQTT 当前同时存在，前者主要作为调试/备用通道，不应描述成两条同时必须经过的主链路。

---

## 12. 编译

本项目基于 ESP-IDF。

```bash
idf.py set-target esp32s3
idf.py build
idf.py flash monitor
```

---

## 13. 面试可讲点

- MPU6050 如何进行姿态采集
- I2C 为什么可以同时挂 MPU6050 和 OLED
- gesture 与 command 为什么分层
- NORMAL / CONTROL 模式设计
- ESP32 WiFi 与 MQTT Client 工作流程
- MQTT QoS1 为什么仍然需要业务去重
- 为什么控制命令不用 Retain
- MQTT 和 UART 在项目中的职责区别
- `seq`、ACK、超时重传和幂等如何进一步组成可靠控制闭环
- 如何从 ESP32 日志一路定位到 Linux Gateway 和 STM32 执行端

---

## 14. 相关仓库

```text
aiot-smart-glasses
→ ESP32-S3 智能眼镜控制端

linux-iot-gateway
→ Linux IoT Gateway

stm32_f407_hand_controller
→ STM32F407 灵动手执行端

aiot-embodied-control-system
→ 整体系统架构与协议说明
```

---

## 15. 开发记录

早期按阶段开发的详细记录已经移至：

```text
docs/development_log.md
```

README 只保留当前有效工程状态和最终设计。