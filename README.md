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
