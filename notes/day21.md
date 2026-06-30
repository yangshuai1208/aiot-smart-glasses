# Day21 学习记录：STM32F407 + PCA9685 单舵机控制

## 今日目标

今天完成 STM32F407VET6 灵动手执行端的单舵机硬件控制验证。

本阶段使用 STM32F407VET6 作为执行端主控，通过 I2C1 控制 PCA9685 16 路舵机驱动板，并先使用 PCA9685 的 CH0 控制一个 MG90S 舵机。STM32 通过 USART1 接收 Linux Gateway 或串口助手发送的 `HAND_OPEN`、`HAND_GRAB`、`HAND_RELEASE`、`HAND_STOP` 命令，解析后控制舵机转到对应角度。

## 今日使用的软件工具

| 软件 / 工具         | 作用                             |
| --------------- | ------------------------------ |
| STM32CubeMX     | 配置 STM32F407VET6 外设并生成 Keil 工程 |
| Keil MDK        | 编译、烧录、调试 STM32 工程              |
| VSCode          | 整理项目代码、README、notes、GitHub 提交  |
| 串口助手 / Linux 终端 | 向 STM32 发送 `HAND_xxx` 测试命令     |

## 今日完成内容

* 使用 STM32CubeMX 创建 STM32F407VET6 工程
* 配置 I2C1：PB6 / PB7，用于连接 PCA9685
* 配置 USART1：PA9 / PA10，115200 8N1，用于接收控制命令
* 在 Project Manager 中选择 MDK-ARM，生成 Keil 工程
* 在 Keil 中添加自定义模块文件
* 新增 `pca9685.h`
* 新增 `pca9685.c`
* 新增 `hand_servo.h`
* 新增 `hand_servo.c`
* 复用 Day20 的 `hand_protocol.h`
* 复用 Day20 的 `hand_protocol.c`
* 修改 CubeMX 生成的 `main.c`
* 实现串口接收命令、协议解析、舵机动作执行的完整流程
* 完成 PCA9685 CH0 单舵机控制逻辑

## 当前硬件链路

```text
Linux Gateway / 串口助手
        ↓ USART1
STM32F407VET6
        ↓ I2C1
PCA9685 16 路舵机驱动板
        ↓ CH0 PWM
MG90S 舵机
```

## 硬件接线

### STM32F407VET6 与 PCA9685

```text
STM32F407 PB6  → PCA9685 SCL
STM32F407 PB7  → PCA9685 SDA
STM32F407 3.3V → PCA9685 VCC
STM32F407 GND  → PCA9685 GND
```

### PCA9685 与外部 5V 电源

```text
5V 3A 电源 +  → PCA9685 V+
5V 3A 电源 -  → PCA9685 GND
STM32 GND     → PCA9685 GND
```

### MG90S 舵机接 CH0

```text
MG90S 信号线    → PCA9685 CH0 Signal
MG90S 红色线    → PCA9685 V+
MG90S 棕/黑色线 → PCA9685 GND
```

### USB-TTL / Linux Gateway 与 STM32 USART1

```text
USB-TTL TX  → STM32 PA10 USART1_RX
USB-TTL RX  → STM32 PA9  USART1_TX
USB-TTL GND → STM32 GND
```

## CubeMX 配置

### SYS

```text
Debug：Serial Wire
```

### I2C1

```text
Mode：I2C
PB6：I2C1_SCL
PB7：I2C1_SDA
Clock Speed：100000 Hz
```

### USART1

```text
Mode：Asynchronous
PA9 ：USART1_TX
PA10：USART1_RX
Baud Rate：115200
Word Length：8 Bits
Parity：None
Stop Bits：1
Hardware Flow Control：None
```

### NVIC

```text
USART1 global interrupt：Enable
```

### Project Manager

```text
Toolchain / IDE：MDK-ARM
```

## 新增文件

```text
stm32_hand_controller/Core/Inc/pca9685.h
stm32_hand_controller/Core/Src/pca9685.c
stm32_hand_controller/Core/Inc/hand_servo.h
stm32_hand_controller/Core/Src/hand_servo.c
```

## 修改文件

```text
stm32_hand_controller/Core/Src/main.c
README.md
notes/day21.md
.gitignore
```

## 核心模块说明

### 1. `hand_protocol`

`hand_protocol` 模块负责解析 Linux Gateway 或串口助手发来的字符串命令。

命令映射关系：

| 串口命令         | STM32 内部动作          |
| ------------ | ------------------- |
| HAND_OPEN    | HAND_ACTION_OPEN    |
| HAND_GRAB    | HAND_ACTION_GRAB    |
| HAND_RELEASE | HAND_ACTION_RELEASE |
| HAND_STOP    | HAND_ACTION_STOP    |
| 其他命令         | HAND_ACTION_NONE    |

核心函数：

```c
hand_action_t hand_protocol_parse(const char *cmd);
const char *hand_action_to_string(hand_action_t action);
```

### 2. `pca9685`

`pca9685` 模块负责通过 I2C 控制 PCA9685 舵机驱动板。

核心功能：

```text
1. 初始化 PCA9685
2. 设置 PWM 频率为 50Hz
3. 设置指定通道 PWM
4. 将舵机角度转换为 PWM 脉宽
5. 控制指定通道舵机角度
```

核心函数：

```c
HAL_StatusTypeDef pca9685_init(I2C_HandleTypeDef *hi2c);
HAL_StatusTypeDef pca9685_set_servo_angle(uint8_t channel, uint16_t angle);
```

### 3. `hand_servo`

`hand_servo` 模块负责把 STM32 内部动作转换为舵机角度，并调用 PCA9685 输出 PWM。

当前 Day21 只控制 CH0 一个 MG90S 舵机。

动作映射：

| 动作                  | 舵机角度 |
| ------------------- | ---- |
| HAND_ACTION_OPEN    | 30°  |
| HAND_ACTION_GRAB    | 130° |
| HAND_ACTION_RELEASE | 60°  |
| HAND_ACTION_STOP    | 90°  |

核心函数：

```c
HAL_StatusTypeDef hand_servo_init(I2C_HandleTypeDef *hi2c);
HAL_StatusTypeDef hand_servo_apply_action(hand_action_t action);
```

## main.c 逻辑

### 初始化阶段

在 CubeMX 生成的 `main.c` 中，初始化顺序为：

```c
MX_GPIO_Init();
MX_I2C1_Init();
MX_USART1_UART_Init();
```

之后执行：

```c
hand_servo_init(&hi2c1);
HAL_UART_Receive_IT(&huart1, &uart_rx_byte, 1);
```

含义：

```text
1. 初始化 PCA9685
2. 让 CH0 舵机回到默认停止角度
3. 开启 USART1 中断接收
```

### UART 接收回调

USART1 每次接收 1 个字节。

当收到普通字符时，存入缓冲区：

```text
H
A
N
D
_
O
P
E
N
```

当收到 `\n` 时，认为一条命令结束，然后解析命令：

```c
hand_pending_action = hand_protocol_parse(uart_line_buf);
hand_action_pending = 1;
```

### 主循环执行动作

UART 中断里只负责接收命令和设置标志位，不直接执行 I2C 操作。

真正的舵机控制放在 `while(1)` 主循环中：

```c
if (hand_action_pending) {
    hand_servo_apply_action(action);
}
```

这样可以避免在中断中执行阻塞式 I2C 操作。

## 测试方式

### 串口助手测试

串口参数：

```text
Baud Rate：115200
Data Bits：8
Stop Bits：1
Parity：None
Flow Control：None
```

发送命令时需要带换行：

```text
HAND_OPEN\n
HAND_GRAB\n
HAND_RELEASE\n
HAND_STOP\n
```

### Linux 终端测试

假设 STM32 串口为 `/dev/ttyUSB1`：

```bash
sudo chmod 666 /dev/ttyUSB1
```

注意：`chmod` 和 `666` 中间有空格，`666` 和 `/dev/ttyUSB1` 中间也有空格。

配置串口：

```bash
stty -F /dev/ttyUSB1 115200 cs8 -cstopb -parenb -ixon -ixoff -crtscts raw
```

发送测试命令：

```bash
printf "HAND_OPEN\n" > /dev/ttyUSB1
printf "HAND_GRAB\n" > /dev/ttyUSB1
printf "HAND_RELEASE\n" > /dev/ttyUSB1
printf "HAND_STOP\n" > /dev/ttyUSB1
```

## 正确实验现象

### 上电初始化

如果 PCA9685 初始化成功，串口输出：

```text
PCA9685 init OK
```

如果失败，串口输出：

```text
PCA9685 init FAIL
```

### 发送 `HAND_OPEN`

```text
MG90S 舵机转到张开角度
串口返回：ACTION OK: HAND_ACTION_OPEN
```

### 发送 `HAND_GRAB`

```text
MG90S 舵机转到抓取角度
串口返回：ACTION OK: HAND_ACTION_GRAB
```

### 发送 `HAND_RELEASE`

```text
MG90S 舵机转到释放角度
串口返回：ACTION OK: HAND_ACTION_RELEASE
```

### 发送 `HAND_STOP`

```text
MG90S 舵机回到中位
串口返回：ACTION OK: HAND_ACTION_STOP
```

## 今日遇到的问题

### 1. pca9685.h 函数声明和 pca9685.c 定义必须一致

例如：

```c
HAL_StatusTypeDef pca9685_set_pwm(uint8_t channel, uint16_t on, uint16_t off);
```

如果头文件和源文件参数数量不同，会编译报错。

### 2. I2C 宏名容易拼错

正确写法：

```c
I2C_MEMADD_SIZE_8BIT
```

不能写成：

```text
I2C_MEMAOD_SIZE_8BIT
```

### 3. 字符串结束符必须是 `'\0'`

正确写法：

```c
uart_line_buf[uart_line_index] = '\0';
```

不能写成：

```c
uart_line_buf[uart_line_index] = '0';
```

`'\0'` 是字符串结束符，`'0'` 是字符 0。

### 4. 动作处理不能放进 Error_Handler

`Error_Handler()` 只在程序错误时进入，正常舵机动作处理应该放在 `while(1)` 主循环中。

### 5. 不要在 UART 中断里直接控制 PCA9685

UART 中断里只设置 pending 标志，I2C 控制放在主循环里执行，避免中断里执行阻塞操作。

## 今日技术知识点

* STM32CubeMX 生成 Keil MDK 工程
* Keil 添加 `.c` 文件并编译烧录
* STM32F407 USART1 中断接收
* STM32F407 I2C1 通信
* PCA9685 16 路 PWM 舵机驱动原理
* MG90S 舵机 PWM 控制
* 50Hz 舵机周期
* 500us / 1500us / 2500us 舵机脉宽
* 字符串协议按行解析
* UART 中断与主循环解耦
* 最小硬件链路验证方法

## 面试可讲点

Day21 中，我将 STM32F407VET6 接入灵动手执行端，通过 I2C1 控制 PCA9685 16 路舵机驱动板，并先使用 CH0 控制一个 MG90S 舵机。

Linux Gateway 或串口助手通过 USART1 向 STM32 发送 `HAND_OPEN`、`HAND_GRAB`、`HAND_RELEASE`、`HAND_STOP` 命令。STM32 端通过 `hand_protocol` 模块解析命令，再通过 `hand_servo` 模块将动作映射为舵机角度，最后调用 `pca9685` 驱动输出 PWM 控制舵机动作。

为了保证稳定性，我没有在 UART 中断里直接执行 I2C 控制，而是在中断中只设置动作 pending 标志，再由主循环执行舵机控制。这种方式避免了中断中执行阻塞操作，更适合后续扩展多舵机动作组。

## 今日总结

Day21 完成了 STM32F407VET6 + PCA9685 + MG90S 单舵机控制链路。当前阶段先跑通 PCA9685 CH0 单舵机，为 Day22 扩展到 CH0~CH4 五个 MG90S 舵机动作组打基础。

## 明日计划

Day22 计划实现 PCA9685 五舵机动作组控制，将 `HAND_OPEN`、`HAND_GRAB`、`HAND_RELEASE`、`HAND_STOP` 映射为五个 MG90S 舵机的联合动作。
