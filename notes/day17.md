# Day17 学习记录：Linux UART JSON 接收程序

## 今日目标

今天完成 Linux 端 UART JSON 接收程序，用于读取 ESP32-S3 智能眼镜控制端通过 UART1 发出的 JSON 数据，为后续 Linux IoT Gateway 接入做准备。

## 今日完成内容

- 新建 tools/linux_uart_receiver 目录
- 编写 uart_receiver.c
- 使用 open() 打开串口设备
- 使用 termios 配置 115200 8N1
- 使用 read() 按字节读取 UART 数据
- 按换行符解析一条 JSON
- 提取 mode、gesture、cmd、status 字段
- 根据 cmd 打印对应动作
- 编写 Makefile
- 编译生成 uart_receiver 程序

## 新增文件

```text
tools/linux_uart_receiver/uart_receiver.c
tools/linux_uart_receiver/Makefile
tools/linux_uart_receiver/README.md
notes/day17.md