# Day11 学习记录

## 今日目标

- 新建 components/gesture 姿态识别组件
- 定义 gesture_type_t 枚举
- 实现 gesture_detect()
- 实现 gesture_to_string()
- 修改 main.c，读取 MPU6050 数据后识别 gesture
- 串口打印 ACC / GYRO / TEMP / Gesture
- 完成 Git 提交

## 今日完成

- 新建 components/gesture 目录
- 新建 components/gesture/include/gesture.h
- 新建 components/gesture/gesture.c
- 新建 components/gesture/CMakeLists.txt
- 在 gesture.h 中定义 GESTURE_NONE、GESTURE_LEFT、GESTURE_RIGHT、GESTURE_NOD、GESTURE_SHAKE
- 在 gesture.c 中使用阈值判断基础姿态
- 在 main.c 中调用 gesture_detect()
- 在 main.c 中使用 gesture_to_string() 打印识别结果
- main/CMakeLists.txt 中加入 REQUIRES gesture

## 今日代码提交

- commit: feat: add basic gesture detection

## 实验现象

编译成功后可以看到：

```text
Project build complete.