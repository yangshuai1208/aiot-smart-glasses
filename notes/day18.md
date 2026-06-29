# Day18 学习记录：Linux Gateway 命令分发模块
今天将 Linux UART Receiver 升级为 Linux Gateway 命令分发模块，使其不仅能接收 ESP32-S3 发来的 JSON 数据，还能解析 cmd 字段，并转换为后续 STM32 灵动手执行端可识别的协议命令。
## 今日完成内容
- 新增 gateway_dispatcher.h
- 新增 gateway_dispatcher.c
- 定义 gateway_cmd_t 网关命令枚举
- 定义 gateway_cmd_t 网关命令枚举
- 实现 gateway_cmd_from_string()
- 实现 gateway_cmd_to_string()
- 修改 uart_receiver.c，使其接收到 JSON 后调用 gateway_dispatcher
- 修改 Makefile，加入 gateway_dispatcher.c 多文件编译
- 支持 --test 模拟测试模式
- 完成 make 编译测试

## 新增文件

```text
tools/linux_uart_receiver/gateway_dispatcher.h
tools/linux_uart_receiver/gateway_dispatcher.c
notes/day18.md
