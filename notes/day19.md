# Day19 学习记录：Linux Gateway 到 STM32 串口发送模块

## 今日目标

今天在 Linux Gateway 中新增 STM32 串口发送模块，使 Linux 端不仅能接收 ESP32-S3 智能眼镜控制端发来的 JSON 命令，还能将解析后的控制命令转换为 STM32 灵动手执行端协议，并通过串口发送给 STM32。

## 今日完成内容

* 新增 `stm32_sender.h`
* 新增 `stm32_sender.c`
* 实现 STM32 串口打开函数 `stm32_sender_open()`
* 实现 STM32 命令发送函数 `stm32_sender_send_cmd()`
* 实现 STM32 串口关闭函数 `stm32_sender_close()`
* 修改 `uart_receiver.c`
* 支持第二个串口参数 `[stm32_uart_device]`
* 保留 `--test` 模拟测试模式
* 支持无 STM32 时的 print-only 模式
* 修改 Makefile，加入 `stm32_sender.c`
* 完成 Linux Gateway 到 STM32 的发送逻辑雏形

## 新增文件

```text
tools/linux_uart_receiver/stm32_sender.h
tools/linux_uart_receiver/stm32_sender.c
notes/day19.md
```

## 修改文件

```text
tools/linux_uart_receiver/uart_receiver.c
tools/linux_uart_receiver/Makefile
README.md
```

## 当前系统链路

```text
ESP32-S3 智能眼镜控制端
        ↓
UART JSON
        ↓
Linux UART Receiver
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
STM32 灵动手执行端
```

## STM32 协议命令映射

| JSON cmd | Gateway cmd    | STM32 协议命令   |
| -------- | -------------- | ------------ |
| OPEN     | GW_CMD_OPEN    | HAND_OPEN    |
| GRAB     | GW_CMD_GRAB    | HAND_GRAB    |
| RELEASE  | GW_CMD_RELEASE | HAND_RELEASE |
| STOP     | GW_CMD_STOP    | HAND_STOP    |
| NONE     | GW_CMD_NONE    | HAND_NONE    |

## 运行方式

### 1. 无硬件测试

```bash
cd ~/aiot-smart-glasses/tools/linux_uart_receiver
make clean
make
./uart_receiver --test
```

该模式不会真正打开串口，只测试 JSON 解析和命令分发逻辑。

### 2. 只有 ESP32 输入，没有 STM32

```bash
./uart_receiver /dev/ttyUSB0
```

正确现象：

```text
STM32 sender disabled, print-only mode
RX JSON: ...
GATEWAY CMD = OPEN
STM32 CMD   = HAND_OPEN
```

### 3. ESP32 + STM32 双串口

```bash
./uart_receiver /dev/ttyUSB0 /dev/ttyUSB1
```

含义：

```text
/dev/ttyUSB0：接收 ESP32-S3 发来的 JSON
/dev/ttyUSB1：发送 HAND_OPEN / HAND_GRAB 等命令给 STM32
```

正确现象：

```text
STM32 sender opened: /dev/ttyUSB1
STM32 sender enabled
RX JSON: {"device":"smart_glasses_01","mode":"CONTROL","gesture":"LEFT","cmd":"OPEN","status":"OK"}
  GATEWAY CMD = OPEN
  STM32 CMD   = HAND_OPEN
  SEND STM32 = HAND_OPEN
```

## 今日核心代码说明

### 1. `stm32_sender_open()`

用于打开 STM32 串口设备，例如 `/dev/ttyUSB1`。

```c
int stm32_sender_open(const char *device);
```

功能：

```text
打开串口设备
配置 115200 波特率
配置 8N1
返回串口 fd
```

### 2. `stm32_sender_send_cmd()`

用于向 STM32 发送协议命令。

```c
int stm32_sender_send_cmd(int fd, const char *cmd);
```

例如发送：

```text
HAND_OPEN
HAND_GRAB
HAND_RELEASE
HAND_STOP
```

每条命令后面追加 `\n`，方便 STM32 按行解析。

### 3. `stm32_sender_close()`

用于关闭 STM32 串口。

```c
void stm32_sender_close(int fd);
```

避免程序退出后文件描述符泄漏。

## 今日技术知识点

### 1. Linux 串口发送

Day19 不再只是 `read()` 接收数据，而是开始使用 `write()` 向串口发送数据。

```c
write(fd, cmd, len);
write(fd, "\n", 1);
```

### 2. `tcdrain()`

发送完命令后调用：

```c
tcdrain(fd);
```

作用是等待串口输出缓冲区的数据真正发送完成。

### 3. 115200 8N1 串口配置

STM32 串口和 Linux 串口需要保持一致：

```text
波特率：115200
数据位：8
校验位：无
停止位：1
流控：关闭
```

### 4. 双串口网关模型

Linux Gateway 现在具备两个串口方向：

```text
ESP32-S3 → Linux Gateway：输入串口
Linux Gateway → STM32：输出串口
```

这体现了 Linux 作为中间网关的作用。

### 5. print-only 模式

当没有 STM32 串口时，程序仍然可以运行，只打印要发送的协议命令，方便调试和演示。

## 今日遇到的问题

### 1. termios 配置容易写错

例如：

```c
cfsetospeed(&tty, B115200);
tty.c_cflag |= CS8;
tty.c_cflag |= CREAD | CLOCAL;
```

这些位操作方向不能写反。

### 2. 函数名拼写错误

例如：

```text
stm32w_sender_open
```

应为：

```text
stm32_sender_open
```

### 3. 参数传递错误

`handle_json_line()` 已经改为：

```c
handle_json_line(line_buf, stm32_fd);
```

不能继续写成：

```c
handle_json_line(line_buf);
```

## 面试可讲点

Day19 中，我在 Linux Gateway 中新增了 `stm32_sender` 模块，用于将 `gateway_dispatcher` 生成的 STM32 协议命令通过 UART 发送给 STM32 灵动手执行端。

程序支持两个串口参数：第一个串口用于接收 ESP32-S3 智能眼镜控制端发来的 JSON，第二个串口用于向 STM32 发送 `HAND_OPEN`、`HAND_GRAB`、`HAND_RELEASE`、`HAND_STOP` 等执行端协议命令。

这样 Linux Gateway 就从 Day18 的“命令解析和分发”进一步升级为 Day19 的“命令转发”，形成了智能眼镜到执行端之间的通信雏形。

## 今日总结

Day19 完成后，Linux Gateway 已经具备向 STM32 执行端发送协议命令的能力。项目链路从“接收 JSON”升级为“接收 JSON → 解析命令 → 转换协议 → 发送给 STM32”。

## 明日计划

Day20 计划编写 STM32 灵动手执行端协议解析模块，使 STM32 能够接收 Linux Gateway 发来的 `HAND_OPEN`、`HAND_GRAB`、`HAND_RELEASE`、`HAND_STOP`，并先通过串口打印或 LED 模拟动作。
