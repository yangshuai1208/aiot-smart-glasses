# Day15 学习记录：JSON 命令格式模块

## 今日目标

今天完成 JSON 命令格式模块，将智能眼镜控制端的 mode、gesture、command、status 等信息组织成标准 JSON 字符串，为后续 MQTT / UART / Linux IoT Gateway 通信做准备。

## 今日完成内容

- 新建 components/json_builder 组件
- 新建 json_builder.h
- 新建 json_builder.c
- 使用 snprintf() 生成 JSON 字符串
- JSON 中包含 device、mode、gesture、cmd、status
- main.c 中新增 print_json_status()
- 每次识别后串口打印 JSON
- 长按 STOP 时打印 STOP JSON
- 编译通过

## 新增文件

```text
components/json_builder/CMakeLists.txt
components/json_builder/json_builder.c
components/json_builder/include/json_builder.h
notes/day15.md