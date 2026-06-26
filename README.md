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
## Day09：MPU6050 I2C 通信测试

本阶段完成 ESP32-S3 与 MPU6050 的 I2C 通信测试。

完成内容：

- 初始化 ESP32-S3 I2C 主机
- 读取 MPU6050 WHO_AM_I 寄存器
- 唤醒 MPU6050
- 读取三轴加速度原始数据
- 读取三轴角速度原始数据
- 串口打印传感器数据

实验现象：

```text
WHO_AM_I = 0x68
ACC_X / ACC_Y / ACC_Z 持续输出
GYRO_X / GYRO_Y / GYRO_Z 持续输出
晃动模块时数据发生变化


---



```markdown
## Day11：基础姿态识别

本阶段新增 gesture 姿态识别组件，将 MPU6050 原始数据转换为智能眼镜动作类型。

新增组件：

```text
components/gesture/
├── CMakeLists.txt
├── gesture.c
└── include/
    └── gesture.h
    ## Day12：OLED 状态显示

本阶段新增 OLED UI 显示模块，用于显示智能眼镜控制端当前状态。

显示内容：

- Mode：当前模式
- Gesture：当前识别动作
- Cmd：映射后的控制命令
- Status：系统状态

OLED 和 MPU6050 共用 I2C 总线：

| 设备 | I2C 地址 |
|---|---:|
| MPU6050 | 0x68 |
| OLED | 0x3C |

当前显示示例：

```text
MODE:CONTROL
GEST:NONE
CMD:NONE
STATUS:OK

---

【面试考点】

### 1. OLED 为什么要单独封装组件？

因为 OLED 显示属于 UI 层，不应该和 `main.c` 或 MPU6050 驱动混在一起。封装成 `oled_ui` 后，主程序只需要调用 `oled_ui_show_status()`。

### 2. OLED 和 MPU6050 为什么可以共用 I2C？

因为 I2C 是总线结构，可以挂多个从设备。只要设备地址不同就可以共用 SDA/SCL。MPU6050 地址是 `0x68`，OLED 常见地址是 `0x3C`。

### 3. 为什么 OLED 很重要？

因为它让智能眼镜控制端具备可视化状态反馈，不再只依赖串口。后续演示时可以直接看到当前模式、动作和控制命令。

---

【明日任务】

明天做 **Day13：加入按键模式切换**。

核心任务：

```text
NORMAL / CONTROL 模式切换
按键短按切模式
长按触发 STOP
OLED 显示当前模式

## Day13：按键模式切换

本阶段新增 button 按键模块，用于增强智能眼镜控制端的人机交互能力。

### 功能说明

- 短按按键：切换 NORMAL / CONTROL 模式
- 长按按键：触发 STOP 安全命令
- OLED 显示当前 Mode / Gesture / Cmd / Status
- CONTROL 模式下根据手势生成控制命令
- NORMAL 模式下只显示姿态，不生成控制命令

### 按键接线

| ESP32-S3 | 按键模块 |
|---|---|
| 3V3 | VCC |
| GND | GND |
| GPIO4 | OUT / SIG / S |

### 模式说明

| 模式 | 说明 |
|---|---|
| NORMAL | 普通显示模式，只显示姿态，不生成控制命令 |
| CONTROL | 控制模式，根据手势生成 OPEN / GRAB / RELEASE |
| STOP | 长按按键触发的安全停止命令 |

### 当前系统链路

```text
MPU6050 姿态采集
        ↓
gesture 手势识别
        ↓
button 按键模式控制
        ↓
cmd 命令生成
        ↓
OLED 状态显示