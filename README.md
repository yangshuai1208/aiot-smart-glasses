# AIoT 智能眼镜控制端

## 项目简介

本项目是 AIoT 智能眼镜 + 具身智能灵动手控制系统中的可穿戴控制端。

智能眼镜控制端通过 MPU6050 采集头部姿态数据，识别 LEFT、RIGHT、NOD、SHAKE 等基础动作，并将动作映射为 OPEN、GRAB、RELEASE、STOP 等控制命令。后续通过串口或 MQTT 将命令发送给 Linux IoT Gateway，再由网关控制 STM32 灵动手执行端完成动作。

本项目第一阶段目标不是实现完整 AR 智能眼镜，而是先完成智能眼镜控制端 MVP，也就是最小可用版本。

## 项目定位

本项目在整体系统中的位置：

```text
AIoT 智能眼镜控制端
    ↓ MQTT JSON / UART
Linux IoT Gateway
    ↓ UART 自定义协议
STM32 灵动手执行端
    ↓ PCA9685
MG90S 舵机 / 灵动手结构件
```

## MVP 目标

智能眼镜控制端 MVP 需要完成：

* MPU6050 姿态数据读取
* LEFT / RIGHT / NOD / SHAKE 动作识别
* OLED 显示当前模式和动作
* 按键切换 NORMAL / CONTROL / SETTING 模式
* 蜂鸣器提示状态变化
* 输出 gesture 和 cmd 控制命令
* 后续通过 MQTT 或串口发送给 Linux 网关

## 硬件组成

| 硬件              | 作用                  |
| --------------- | ------------------- |
| ESP32-S3 开发板    | 主控，负责姿态处理、WiFi、MQTT |
| MPU6050         | 采集头部姿态数据            |
| 0.96 寸 I2C OLED | 显示当前模式、手势和命令        |
| 轻触按键模块          | 模式切换、确认、停止          |
| 有源蜂鸣器           | 状态提示和错误提示           |
| 杜邦线 / 面包板       | 原型连接                |
| 眼镜支架 / 头部固定结构   | 固定 MPU6050 和开发板     |

## 软件模块设计

| 模块         | 文件                 | 作用                      |
| ---------- | ------------------ | ----------------------- |
| MPU6050 驱动 | `mpu6050.c/.h`     | 读取加速度和角速度数据             |
| 手势识别       | `gesture.c/.h`     | 判断 LEFT、RIGHT、NOD、SHAKE |
| OLED 显示    | `oled_ui.c/.h`     | 显示模式、手势、命令              |
| 按键模块       | `button.c/.h`      | 模式切换和停止命令               |
| 蜂鸣器模块      | `buzzer.c/.h`      | 状态提示                    |
| 命令生成       | `command.c/.h`     | gesture 映射为 cmd         |
| MQTT 通信    | `mqtt_client.c/.h` | 后续发布 JSON 到 Linux 网关    |

## 手势与命令映射

| gesture     | cmd     | 说明          |
| ----------- | ------- | ----------- |
| LEFT        | OPEN    | 灵动手张开       |
| NOD         | GRAB    | 灵动手抓取       |
| RIGHT       | RELEASE | 灵动手释放       |
| BUTTON_LONG | STOP    | 停止动作，进入安全状态 |
| SHAKE       | NONE    | 暂不执行        |

## MQTT JSON 示例

```json
{
  "device": "smart_glasses_01",
  "gesture": "NOD",
  "mode": "CONTROL",
  "cmd": "GRAB"
}
```

## 第一阶段计划

| Day   | 任务                          |
| ----- | --------------------------- |
| Day8  | 新建智能眼镜控制端仓库，完成 README 和硬件设计 |
| Day9  | 跑通 MPU6050 I2C 通信           |
| Day10 | 封装 MPU6050 驱动               |
| Day11 | 实现基础姿态识别                    |
| Day12 | 加入 OLED 显示                  |
| Day13 | 加入按键模式切换                    |
| Day14 | 完成智能眼镜控制端 MVP 总结和演示         |

## 项目亮点

* 使用智能眼镜作为可穿戴控制端，而不是普通按钮遥控
* 通过 MPU6050 实现头部姿态识别
* 将 gesture 映射为具身智能灵动手动作命令
* 后续支持 MQTT JSON 上传到 Linux IoT Gateway
* 与 STM32 灵动手执行端组成完整 AIoT 控制闭环

## 面试可讲点

* 如何使用 MPU6050 识别头部姿态
* 如何设计 gesture 到 cmd 的命令映射
* 为什么智能眼镜控制端先做 MVP
* 为什么使用 MQTT JSON 作为上报格式
* 智能眼镜在系统中如何和 Linux 网关、STM32 灵动手协作
## Day09：MPU6050 I2C 通信测试

本阶段完成 ESP32-S3 与 MPU6050 的 I2C 通信测试。

完成内容：

- 初始化 ESP32-S3 I2C 主机
- 读取 MPU6050 WHO_AM_I 寄存器
- 唤醒 MPU6050
- 读取三轴加速度原始数据
- 读取三轴角速度原始数据
- 串口打印传感器数据

实验现象：

```text
WHO_AM_I = 0x68
ACC_X / ACC_Y / ACC_Z 持续输出
GYRO_X / GYRO_Y / GYRO_Z 持续输出
晃动模块时数据发生变化


---



```markdown
## Day11：基础姿态识别

本阶段新增 gesture 姿态识别组件，将 MPU6050 原始数据转换为智能眼镜动作类型。

新增组件：

```text
components/gesture/
├── CMakeLists.txt
├── gesture.c
└── include/
    └── gesture.h
    ## Day12：OLED 状态显示

本阶段新增 OLED UI 显示模块，用于显示智能眼镜控制端当前状态。

显示内容：

- Mode：当前模式
- Gesture：当前识别动作
- Cmd：映射后的控制命令
- Status：系统状态

OLED 和 MPU6050 共用 I2C 总线：

| 设备 | I2C 地址 |
|---|---:|
| MPU6050 | 0x68 |
| OLED | 0x3C |

当前显示示例：

```text
MODE:CONTROL
GEST:NONE
CMD:NONE
STATUS:OK

---

【面试考点】

### 1. OLED 为什么要单独封装组件？

因为 OLED 显示属于 UI 层，不应该和 `main.c` 或 MPU6050 驱动混在一起。封装成 `oled_ui` 后，主程序只需要调用 `oled_ui_show_status()`。

### 2. OLED 和 MPU6050 为什么可以共用 I2C？

因为 I2C 是总线结构，可以挂多个从设备。只要设备地址不同就可以共用 SDA/SCL。MPU6050 地址是 `0x68`，OLED 常见地址是 `0x3C`。

### 3. 为什么 OLED 很重要？

因为它让智能眼镜控制端具备可视化状态反馈，不再只依赖串口。后续演示时可以直接看到当前模式、动作和控制命令。

---

【明日任务】

明天做 **Day13：加入按键模式切换**。

核心任务：

```text
NORMAL / CONTROL 模式切换
按键短按切模式
长按触发 STOP
OLED 显示当前模式

## Day13：按键模式切换

本阶段新增 button 按键模块，用于增强智能眼镜控制端的人机交互能力。

### 功能说明

- 短按按键：切换 NORMAL / CONTROL 模式
- 长按按键：触发 STOP 安全命令
- OLED 显示当前 Mode / Gesture / Cmd / Status
- CONTROL 模式下根据手势生成控制命令
- NORMAL 模式下只显示姿态，不生成控制命令

### 按键接线

| ESP32-S3 | 按键模块 |
|---|---|
| 3V3 | VCC |
| GND | GND |
| GPIO4 | OUT / SIG / S |

### 模式说明

| 模式 | 说明 |
|---|---|
| NORMAL | 普通显示模式，只显示姿态，不生成控制命令 |
| CONTROL | 控制模式，根据手势生成 OPEN / GRAB / RELEASE |
| STOP | 长按按键触发的安全停止命令 |

### 当前系统链路

```text
MPU6050 姿态采集
        ↓
gesture 手势识别
        ↓
button 按键模式控制
        ↓
cmd 命令生成
        ↓
OLED 状态显示
```markdown
## Day14：command 命令生成模块

本阶段新增 command 命令生成模块，用于将 gesture 手势识别结果转换为标准控制命令。

### 功能说明

- 将 LEFT 映射为 OPEN
- 将 RIGHT 映射为 RELEASE
- 将 NOD 映射为 GRAB
- 将 SHAKE 映射为 NONE
- 长按按键触发 STOP
- main.c 不再直接处理命令映射逻辑

### 新增组件

```text
components/command/
├── CMakeLists.txt
├── command.c
└── include/
    └── command.h

## Day15：JSON 命令格式模块

本阶段新增 json_builder 模块，用于将智能眼镜控制端的运行状态格式化为标准 JSON 字符串。

### JSON 示例

```json
{"device":"smart_glasses_01","mode":"CONTROL","gesture":"NOD","cmd":"GRAB","status":"OK"}

## Day16：UART JSON 输出模块

本阶段新增 uart_sender 模块，用于将 json_builder 生成的 JSON 字符串通过 UART1 输出。

### UART 配置

| 参数 | 值 |
|---|---|
| UART | UART1 |
| TX | GPIO17 |
| RX | GPIO18 |
| Baudrate | 115200 |
| Data bits | 8 |
| Parity | None |
| Stop bits | 1 |

### 接线

| ESP32-S3 | USB-TTL / Linux Gateway |
|---|---|
| GPIO17 TX | RX |
| GPIO18 RX | TX |
| GND | GND |

### JSON 输出示例

```json
{"device":"smart_glasses_01","mode":"CONTROL","gesture":"NOD","cmd":"GRAB","status":"OK"}
## Day17：Linux UART JSON 接收程序

本阶段新增 Linux 端 UART JSON 接收程序，用于接收 ESP32-S3 智能眼镜控制端通过 UART1 发出的 JSON 数据。

### 新增目录

```text
tools/linux_uart_receiver/
├── Makefile
├── README.md
└── uart_receiver.c## Day18：Linux Gateway 命令分发模块

本阶段新增 Linux Gateway 命令分发模块，用于将 ESP32-S3 智能眼镜控制端发来的 JSON cmd 字段转换为 STM32 灵动手执行端协议命令。

### 新增模块

```text
tools/linux_uart_receiver/
├── gateway_dispatcher.c
├── gateway_dispatcher.h
├── uart_receiver.c
└── Makefile
## Day19：Linux Gateway 到 STM32 串口发送模块

Day19 新增 `stm32_sender` 模块，用于将 Linux Gateway 生成的 STM32 协议命令通过 UART 发送给 STM32 灵动手执行端。

### 项目链路

```text
ESP32-S3 智能眼镜控制端
        ↓
UART JSON
        ↓
Linux UART Receiver
        ↓
Gateway Dispatcher
        ↓
STM32 Sender
        ↓
UART
        ↓
STM32 灵动手执行端
```

### 新增模块

```text
tools/linux_uart_receiver/
├── uart_receiver.c
├── gateway_dispatcher.c
├── gateway_dispatcher.h
├── stm32_sender.c
├── stm32_sender.h
└── Makefile
```

### 功能说明

`stm32_sender` 模块主要负责三件事：

```text
1. 打开 STM32 串口设备
2. 配置 115200 8N1 串口参数
3. 发送 HAND_OPEN / HAND_GRAB / HAND_RELEASE / HAND_STOP 协议命令
```

### 协议命令映射

| JSON cmd | Gateway cmd    | STM32 协议命令   |
| -------- | -------------- | ------------ |
| OPEN     | GW_CMD_OPEN    | HAND_OPEN    |
| GRAB     | GW_CMD_GRAB    | HAND_GRAB    |
| RELEASE  | GW_CMD_RELEASE | HAND_RELEASE |
| STOP     | GW_CMD_STOP    | HAND_STOP    |
| NONE     | GW_CMD_NONE    | HAND_NONE    |

### 编译方式

```bash
cd tools/linux_uart_receiver
make clean
make
```

Makefile 中需要包含：

```makefile
SRCS = uart_receiver.c gateway_dispatcher.c stm32_sender.c
```

注意：Makefile 命令行前面必须是 Tab，不是空格。

### 运行方式

#### 无硬件测试

```bash
./uart_receiver --test
```

#### 只有 ESP32 输入

```bash
./uart_receiver /dev/ttyUSB0
```

#### ESP32 + STM32 双串口

```bash
./uart_receiver /dev/ttyUSB0 /dev/ttyUSB1
```

其中：

```text
/dev/ttyUSB0：接收 ESP32-S3 发来的 JSON 数据
/dev/ttyUSB1：向 STM32 发送协议命令
```

### 正确实验现象

收到 `OPEN` 命令时：

```text
RX JSON: {"device":"smart_glasses_01","mode":"CONTROL","gesture":"LEFT","cmd":"OPEN","status":"OK"}
  cmd         = OPEN
  GATEWAY CMD = OPEN
  STM32 CMD   = HAND_OPEN
  SEND STM32 = HAND_OPEN
```

收到 `GRAB` 命令时：

```text
GATEWAY CMD = GRAB
STM32 CMD   = HAND_GRAB
SEND STM32 = HAND_GRAB
```

收到 `STOP` 命令时：

```text
GATEWAY CMD = STOP
STM32 CMD   = HAND_STOP
SEND STM32 = HAND_STOP
```

### Day19 面试亮点

Day19 将 Linux Gateway 从命令分发升级为命令转发。

Linux 端可以接收 ESP32-S3 发来的 JSON，解析出 `cmd` 字段，再通过 `gateway_dispatcher` 转换为 STM32 协议命令，最后通过 `stm32_sender` 模块发送给 STM32 执行端。

这使整个项目从单设备通信逐步扩展为：

```text
智能眼镜控制端 + Linux 网关 + STM32 执行端
```

具备了真实嵌入式系统中“多设备通信、协议转换、命令转发”的项目特征。
## Day20：STM32 灵动手执行端协议解析模块

Day20 新增 STM32 执行端协议解析模块 `hand_protocol`，用于解析 Linux Gateway 发来的 `HAND_OPEN`、`HAND_GRAB`、`HAND_RELEASE`、`HAND_STOP` 等协议命令。

### 当前项目链路

```text
ESP32-S3 智能眼镜控制端
        ↓
UART JSON
        ↓
Linux UART Receiver
        ↓
Gateway Dispatcher
        ↓
STM32 Sender
        ↓
UART
        ↓
STM32 hand_protocol
        ↓
灵动手动作执行
```

### 新增文件

```text
stm32_hand_controller/
├── Core
│   ├── Inc
│   │   └── hand_protocol.h
│   └── Src
│       ├── hand_protocol.c
│       ├── main.c
│       └── main_uart_rx_example.c
└── test_hand_protocol.c
```

### 模块功能

`hand_protocol` 模块主要负责将 Linux Gateway 发送来的字符串命令转换为 STM32 内部动作枚举。

| Linux Gateway 命令 | STM32 内部动作          |
| ---------------- | ------------------- |
| HAND_OPEN        | HAND_ACTION_OPEN    |
| HAND_GRAB        | HAND_ACTION_GRAB    |
| HAND_RELEASE     | HAND_ACTION_RELEASE |
| HAND_STOP        | HAND_ACTION_STOP    |
| 其他未知命令           | HAND_ACTION_NONE    |

### 核心接口

```c
hand_action_t hand_protocol_parse(const char *cmd);
const char *hand_action_to_string(hand_action_t action);
void hand_action_execute(hand_action_t action);
```

### 测试方式

当前阶段先在 Linux 环境下测试协议解析逻辑：

```bash
cd stm32_hand_controller

gcc -Wall -Wextra -g \
    -ICore/Inc \
    Core/Src/hand_protocol.c \
    test_hand_protocol.c \
    -o test_hand_protocol

./test_hand_protocol
```

### 正确实验现象

```text
CMD    : HAND_OPEN
ACTION : HAND_ACTION_OPEN
ACTION: open hand
--------------------------------
CMD    : HAND_GRAB
ACTION : HAND_ACTION_GRAB
ACTION: grab object
--------------------------------
CMD    : HAND_RELEASE
ACTION : HAND_ACTION_RELEASE
ACTION: release object
--------------------------------
CMD    : HAND_STOP
ACTION : HAND_ACTION_STOP
ACTION: emergency stop
--------------------------------
CMD    : UNKNOWN_CMD
ACTION : HAND_ACTION_NONE
ACTION: no valid command
--------------------------------
```

### CubeMX main.c 插入说明

`main_uart_rx_example.c` 不是完整的 STM32 `main.c`，不能直接烧录。

该文件用于说明：后续应将 UART 接收中断相关代码插入到 CubeMX 生成的 `main.c` 对应 `USER CODE` 区域中，包括：

```text
1. Includes 区域加入 hand_protocol.h
2. PV 区域加入 UART 接收缓冲区
3. main() 中 USART 初始化后开启 HAL_UART_Receive_IT()
4. USER CODE BEGIN 4 区域加入 HAL_UART_RxCpltCallback()
```

### Day20 面试亮点

Day20 将 STM32 执行端接入整个项目链路。

STM32 端通过 `hand_protocol` 模块解析 Linux Gateway 发来的 `HAND_OPEN`、`HAND_GRAB`、`HAND_RELEASE`、`HAND_STOP` 协议命令，并将其转换为内部枚举动作。

这样后续舵机控制模块不需要关心字符串协议，只需要根据 `HAND_ACTION_OPEN`、`HAND_ACTION_GRAB` 等动作枚举执行对应动作，提升了代码清晰度和可维护性。

## Day21：STM32F407 + PCA9685 单舵机控制

本阶段使用 STM32F407VET6 作为灵动手执行端主控，通过 I2C1 控制 PCA9685 16 路舵机驱动板，并先使用 CH0 控制一个 MG90S 舵机。

串口助手或 Linux Gateway 通过 USART1 向 STM32 发送 `HAND_OPEN`、`HAND_GRAB`、`HAND_RELEASE`、`HAND_STOP` 命令。STM32 接收后通过 `hand_protocol` 模块解析协议，再通过 `hand_servo` 模块映射为舵机角度，最终调用 `pca9685` 驱动输出 PWM 控制 MG90S 舵机动作。

当前阶段先完成单舵机硬件闭环验证，后续扩展到 CH0~CH4 五个舵机组成完整机械手动作组。

### 使用软件

| 软件 / 工具         | 用途                             |
| --------------- | ------------------------------ |
| STM32CubeMX     | 配置 STM32F407VET6 外设，生成 Keil 工程 |
| Keil MDK        | 编译、烧录、调试 STM32 工程              |
| VSCode          | 整理项目代码、README、notes、GitHub 提交  |
| 串口助手 / Linux 终端 | 发送 `HAND_xxx` 测试命令             |

### Day21 硬件链路

```text
Linux Gateway / 串口助手
        ↓ USART1
STM32F407VET6
        ↓ I2C1
PCA9685 16 路舵机驱动板
        ↓ CH0 PWM
MG90S 舵机
```

### 接线说明

```text
STM32F407 PB6  → PCA9685 SCL
STM32F407 PB7  → PCA9685 SDA
STM32F407 3.3V → PCA9685 VCC
STM32F407 GND  → PCA9685 GND

5V 3A 电源 +  → PCA9685 V+
5V 3A 电源 -  → PCA9685 GND
STM32 GND     → PCA9685 GND

MG90S 信号线    → PCA9685 CH0 Signal
MG90S 红色线    → PCA9685 V+
MG90S 棕/黑色线 → PCA9685 GND

USB-TTL TX  → STM32 PA10 USART1_RX
USB-TTL RX  → STM32 PA9  USART1_TX
USB-TTL GND → STM32 GND
```

### CubeMX 配置

```text
SYS Debug：Serial Wire

I2C1：
PB6 → I2C1_SCL
PB7 → I2C1_SDA
Clock Speed：100kHz

USART1：
PA9  → USART1_TX
PA10 → USART1_RX
Baud Rate：115200
Word Length：8 Bits
Parity：None
Stop Bits：1
Hardware Flow Control：None

NVIC：
USART1 global interrupt Enable

Project Manager：
Toolchain / IDE：MDK-ARM
```

### Day21 新增模块

```text
Core/Inc/pca9685.h
Core/Src/pca9685.c
Core/Inc/hand_servo.h
Core/Src/hand_servo.c
```

### 复用模块

```text
Core/Inc/hand_protocol.h
Core/Src/hand_protocol.c
```

### 模块职责

| 模块            | 职责                       |
| ------------- | ------------------------ |
| hand_protocol | 解析 `HAND_OPEN` 等串口命令     |
| pca9685       | 通过 I2C 控制 PCA9685 PWM 输出 |
| hand_servo    | 将动作映射为舵机角度               |
| main.c        | 接收串口命令，调度动作执行            |

### 命令与动作映射

| 串口命令         | STM32 动作            | 舵机角度 |
| ------------ | ------------------- | ---- |
| HAND_OPEN    | HAND_ACTION_OPEN    | 30°  |
| HAND_GRAB    | HAND_ACTION_GRAB    | 130° |
| HAND_RELEASE | HAND_ACTION_RELEASE | 60°  |
| HAND_STOP    | HAND_ACTION_STOP    | 90°  |

### 测试方式

串口参数：

```text
Baud Rate：115200
Data Bits：8
Stop Bits：1
Parity：None
Flow Control：None
```

发送命令：

```text
HAND_OPEN\n
HAND_GRAB\n
HAND_RELEASE\n
HAND_STOP\n
```

Linux 终端测试示例：

```bash
sudo chmod 666 /dev/ttyUSB1

stty -F /dev/ttyUSB1 115200 cs8 -cstopb -parenb -ixon -ixoff -crtscts raw

printf "HAND_OPEN\n" > /dev/ttyUSB1
printf "HAND_GRAB\n" > /dev/ttyUSB1
printf "HAND_RELEASE\n" > /dev/ttyUSB1
printf "HAND_STOP\n" > /dev/ttyUSB1
```

### 正确实验现象

```text
PCA9685 init OK
```

发送 `HAND_OPEN`：

```text
MG90S 舵机转到张开角度
ACTION OK: HAND_ACTION_OPEN
```

发送 `HAND_GRAB`：

```text
MG90S 舵机转到抓取角度
ACTION OK: HAND_ACTION_GRAB
```

发送 `HAND_RELEASE`：

```text
MG90S 舵机转到释放角度
ACTION OK: HAND_ACTION_RELEASE
```

发送 `HAND_STOP`：

```text
MG90S 舵机回到中位
ACTION OK: HAND_ACTION_STOP
```

### Day21 面试亮点

Day21 将 STM32F407VET6 接入灵动手执行端，通过 I2C1 控制 PCA9685 16 路 PWM 舵机驱动板，并先使用 CH0 控制一个 MG90S 舵机。

系统通过 USART1 接收 Linux Gateway 或串口助手发送的 `HAND_OPEN`、`HAND_GRAB`、`HAND_RELEASE`、`HAND_STOP` 命令。STM32 端将字符串协议解析为内部动作枚举，再将动作映射为舵机角度，最后通过 PCA9685 输出 PWM 控制舵机。

该阶段完成了从“命令解析”到“真实硬件动作”的最小闭环，为后续五指动作组控制打下基础。
