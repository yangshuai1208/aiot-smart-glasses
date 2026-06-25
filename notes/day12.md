# Day12 学习记录

## 今日目标

- 新增 OLED UI 显示模块
- 使用 I2C 驱动 0.96 寸 OLED
- OLED 显示 Mode、Gesture、Cmd、Status
- 将 Day11 的手势识别结果显示到屏幕上
- 完成 ESP-IDF 编译
- 完成 Git 提交

## 今日完成

- 新建 components/oled_ui 组件
- 新建 oled_ui.h
- 新建 oled_ui.c
- 实现 oled_ui_init()
- 实现 oled_ui_show_status()
- 修改 main.c，引入 oled_ui 模块
- 新增 gesture 到 cmd 的映射函数
- main/CMakeLists.txt 加入 REQUIRES oled_ui
- 编译通过

## 今日代码提交

- commit: feat: add oled ui status display

## 实验现象

编译成功后看到：

```text
Project build complete.