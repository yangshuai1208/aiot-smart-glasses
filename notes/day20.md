# Day20 学习记录：STM32 灵动手执行端协议解析模块

## 今日目标

今天完成 STM32 灵动手执行端协议解析模块，使 STM32 能够接收 Linux Gateway 发来的协议命令：

```text
HAND_OPEN
HAND_GRAB
HAND_RELEASE
HAND_STOP
```

并将这些字符串命令解析为 STM32 内部动作枚举，为后续舵机控制做准备。

## 今日完成内容

* 新增 `hand_protocol.h`
* 新增 `hand_protocol.c`
* 新增 `main_uart_rx_example.c`
* 新增 `test_hand_protocol.c`
* 实现 `hand_protocol_parse()`
* 实现 `hand_action_to_string()`
* 实现 `hand_action_execute()`
* 使用 Linux gcc 测试 STM32 协议解析逻辑
* 明确 CubeMX 生成的 `main.c` 中应该插入哪些 UART 接收代码

## 新增文件

```text
stm32_hand_controller/Core/Inc/hand_protocol.h
stm32_hand_controller/Core/Src/hand_protocol.c
stm32_hand_controller/Core/Src/main_uart_rx_example.c
stm32_hand_controller/test_hand_protocol.c
notes/day20.md
```

## 当前系统链路

```text
ESP32-S3 智能眼镜控制端
        ↓
UART JSON
        ↓
Linux Gateway
        ↓
解析 cmd 字段
        ↓
Gateway Dispatcher
        ↓
生成 STM32 协议命令
        ↓
STM32 Sender
        ↓
UART
        ↓
STM32 hand_protocol 解析
        ↓
HAND_ACTION_OPEN / GRAB / RELEASE / STOP
```

## 协议命令映射

| Linux Gateway 发送命令 | STM32 内部动作          |
| ------------------ | ------------------- |
| HAND_OPEN          | HAND_ACTION_OPEN    |
| HAND_GRAB          | HAND_ACTION_GRAB    |
| HAND_RELEASE       | HAND_ACTION_RELEASE |
| HAND_STOP          | HAND_ACTION_STOP    |
| 其他未知命令             | HAND_ACTION_NONE    |

## 核心代码说明

### 1. `hand_protocol.h`

`hand_protocol.h` 定义了 STM32 执行端内部动作枚举和三个对外接口函数：

```c
typedef enum
{
    HAND_ACTION_NONE = 0,
    HAND_ACTION_OPEN,
    HAND_ACTION_GRAB,
    HAND_ACTION_RELEASE,
    HAND_ACTION_STOP
} hand_action_t;
```

这些枚举用于表示 STM32 内部动作状态，避免程序内部一直直接处理字符串。

### 2. `hand_protocol_parse()`

```c
hand_action_t hand_protocol_parse(const char *cmd);
```

该函数负责将 Linux Gateway 发来的字符串命令转换为 STM32 内部动作枚举。

例如：

```text
HAND_OPEN   → HAND_ACTION_OPEN
HAND_GRAB   → HAND_ACTION_GRAB
HAND_STOP   → HAND_ACTION_STOP
UNKNOWN_CMD → HAND_ACTION_NONE
```

### 3. `hand_action_to_string()`

```c
const char *hand_action_to_string(hand_action_t action);
```

该函数用于调试打印，可以把枚举动作转换为字符串，方便观察实验现象。

### 4. `hand_action_execute()`

```c
void hand_action_execute(hand_action_t action);
```

当前阶段先通过 `printf()` 模拟动作：

```text
ACTION: open hand
ACTION: grab object
ACTION: release object
ACTION: emergency stop
```

后续 Day21 会将这里的打印逻辑替换或扩展为真实舵机控制逻辑。

## UART 接收设计

STM32 端后续使用 UART 中断方式接收 Linux Gateway 发来的命令。

Linux Gateway 每发送一条命令都会带上换行符：

```text
HAND_OPEN\n
```

STM32 按字节接收，当收到 `\n` 时，认为一条命令接收完成，然后调用：

```c
hand_protocol_parse(uart_line_buf);
```

完成协议解析。

## 测试方式

由于当前还没有真正移植到 CubeMX 工程中，所以先在 Linux 环境下测试协议解析模块。

编译命令：

```bash
cd ~/aiot-smart-glasses/stm32_hand_controller

gcc -Wall -Wextra -g \
    -ICore/Inc \
    Core/Src/hand_protocol.c \
    test_hand_protocol.c \
    -o test_hand_protocol
```

运行命令：

```bash
./test_hand_protocol
```

## 正确实验现象

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

看到以上现象，说明 STM32 执行端协议解析逻辑正确。

## 今日遇到的问题

### 1. 不能直接重写 CubeMX 生成的 main.c

`HAL_UART_Receive_IT()` 和 `HAL_UART_RxCpltCallback()` 不是随便写在普通 `main.c` 顶部的代码。

正确做法是：把 UART 接收相关代码插入到 CubeMX 生成的 `main.c` 对应的 `USER CODE` 区域中。

### 2. 字符和字符串不能混用

错误写法：

```c
uart_rx_byte == "\n"
```

正确写法：

```c
uart_rx_byte == '\n'
```

`'\n'` 是字符，`"\n"` 是字符串。

### 3. 函数名必须和头文件一致

例如：

```c
hand_protocol_parse()
hand_action_to_string()
```

`.h` 和 `.c` 中的函数名必须完全一致，否则会出现编译或链接错误。

### 4. enum 不能重复定义

枚举中不能重复写：

```c
HAND_ACTION_GRAB,
HAND_ACTION_GRAB,
```

否则会编译报错。

## 今日技术知识点

* STM32 协议解析模块设计
* 字符串命令转枚举
* `strcmp()` 字符串比较
* `enum` 动作建模
* UART 按行接收思想
* `\n` 作为协议结束符
* CubeMX `USER CODE` 区域的作用
* Linux 下用 gcc 先测试纯 C 模块
* 后续移植到 STM32 工程的方法

## 面试可讲点

Day20 中，我在 STM32 执行端新增了 `hand_protocol` 协议解析模块，用于解析 Linux Gateway 发来的 `HAND_OPEN`、`HAND_GRAB`、`HAND_RELEASE`、`HAND_STOP` 命令。

该模块将字符串协议命令转换为 STM32 内部动作枚举，例如 `HAND_OPEN` 转换为 `HAND_ACTION_OPEN`。这样后续舵机控制模块只需要根据枚举动作执行张开、抓取、释放、停止等动作，不需要直接处理字符串。

同时，我设计了基于 `\n` 的按行接收方式，Linux Gateway 每次发送一条完整命令，STM32 收到换行符后解析整条命令。这种方式简单、稳定，适合当前项目阶段的串口协议通信。

## 今日总结

Day20 完成后，STM32 端已经具备协议解析能力。项目链路进一步完善为：

```text
ESP32-S3 智能眼镜 → Linux Gateway → STM32 协议解析 → 后续舵机执行
```

STM32 端从“还未参与链路”升级为“可以理解 Linux Gateway 命令的执行端节点”。

## 明日计划

Day21 计划实现灵动手舵机控制基础，将 `HAND_ACTION_OPEN`、`HAND_ACTION_GRAB`、`HAND_ACTION_RELEASE`、`HAND_ACTION_STOP` 对应到实际舵机动作或 PWM 输出。
