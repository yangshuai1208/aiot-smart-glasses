# Day16 学习记录：UART JSON 输出模块

## 今日目标

今天完成 UART JSON 输出模块，将 Day15 生成的 JSON 字符串通过 UART1 输出，为后续 Linux IoT Gateway 接收智能眼镜控制端数据做准备。

## 今日完成内容

- 新建 components/uart_sender 组件
- 新建 uart_sender.h
- 新建 uart_sender.c
- 使用 UART1 作为 JSON 输出串口
- 默认 TX 使用 GPIO17
- 默认 RX 使用 GPIO18
- 波特率设置为 115200
- 实现 uart_sender_init()
- 实现 uart_sender_send_json()
- main.c 中引入 uart_sender
- JSON 生成后通过 UART 输出
- 保留 ESP_LOGI 打印，方便调试
- 编译通过

## 新增文件

```text
components/uart_sender/CMakeLists.txt
components/uart_sender/uart_sender.c
components/uart_sender/include/uart_sender.h
notes/day16.md