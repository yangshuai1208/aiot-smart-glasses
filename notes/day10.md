# Day10 学习记录

## 今日目标

* 将 Day09 写在 main.c 中的 MPU6050 代码拆分成独立驱动模块
* 新建 components/mpu6050 组件
* 编写 mpu6050.h 和 mpu6050.c
* 定义 mpu6050_raw_data_t 结构体
* 封装 mpu6050_init()
* 封装 mpu6050_read_raw()
* 修改 main.c，使 main.c 只调用驱动接口
* 通过 idf.py build 编译工程

## 今日完成

* 新建 components/mpu6050 目录
* 新建 components/mpu6050/include/mpu6050.h
* 新建 components/mpu6050/mpu6050.c
* 新建 components/mpu6050/CMakeLists.txt
* 修改 main/CMakeLists.txt，加入 REQUIRES mpu6050
* 在 mpu6050.h 中定义 mpu6050_raw_data_t 结构体
* 在 mpu6050.c 中封装 I2C 初始化、寄存器读写、MPU6050 初始化和原始数据读取
* main.c 中只保留应用层逻辑：初始化 MPU6050、周期读取数据、打印数据
* 成功执行 idf.py build

## 今日代码提交

* commit: feat: modularize mpu6050 driver

## 实验现象

当前已完成编译验证：

```text
Project build complete.
```

说明：

* ESP-IDF 工程结构正确
* components/mpu6050 组件已被 ESP-IDF 正确识别
* main.c 能正确依赖 mpu6050 组件
* mpu6050.h / mpu6050.c 基础语法和 CMake 配置通过

当前还未完成硬件烧录验证。后续烧录到 ESP32-S3 后，期望串口看到：

```text
I APP_MAIN: AIoT Smart Glasses Day10 Start
I MPU6050: I2C init success
I MPU6050: WHO_AM_I = 0x68
I MPU6050: MPU6050 wake up success
I APP_MAIN: ACC_X=xxx ACC_Y=xxx ACC_Z=xxx | GYRO_X=xxx GYRO_Y=xxx GYRO_Z=xxx | TEMP=xx.xx
```

## 今日遇到的问题

### 问题 1：ESP-IDF 找不到 mpu6050 组件

现象：

```text
The component 'mpu6050' could not be found
```

原因：

* components/mpu6050 目录结构不完整
* components/mpu6050/CMakeLists.txt 未正确配置
* main/CMakeLists.txt 中 REQUIRES mpu6050 与组件目录不匹配
* 修改 CMake 后没有清理旧 build

解决：

* 确保目录结构为 components/mpu6050
* 在 components/mpu6050/CMakeLists.txt 中注册组件
* 在 main/CMakeLists.txt 中添加 REQUIRES mpu6050
* 执行 rm -rf build 后重新 idf.py build

### 问题 2：头文件和源文件混在一起

现象：

* mpu6050.h 中误写了函数实现
* 结构体类型名写错
* 编译无法识别 mpu6050_raw_data_t

原因：

* .h 和 .c 文件职责没有分清楚

解决：

* mpu6050.h 只放结构体定义和函数声明
* mpu6050.c 只放函数实现
* 正确声明结构体类型为 mpu6050_raw_data_t

### 问题 3：拼写错误导致编译失败

出现过的错误包括：

```text
paMS_TO_TICKS 写错，应为 pdMS_TO_TICKS
data.temp 写错，应为 data.temperature
ESP_ERR_INVALD_ARG 写错，应为 ESP_ERR_INVALID_ARG
szie_t 写错，应为 size_t
ESP_LOG(TAG, ...) 写错，应为 ESP_LOGI(TAG, ...)
```

解决：

* 编译报错时优先看 error 所在行
* 统一检查变量名、函数名、宏名是否拼写一致

## 今日知识点

### 1. ESP-IDF 组件化开发

ESP-IDF 可以将功能模块拆分到 components 目录中，例如：

```text
components/mpu6050/
├── CMakeLists.txt
├── mpu6050.c
└── include/
    └── mpu6050.h
```

这样可以让 MPU6050 驱动独立于 main.c，后续 gesture、OLED、button、buzzer 也可以按同样方式封装。

### 2. 头文件和源文件分工

mpu6050.h 负责对外提供接口：

* 结构体定义
* 函数声明

mpu6050.c 负责内部实现：

* I2C 初始化
* 寄存器写入
* 寄存器读取
* MPU6050 初始化
* 原始数据读取

### 3. main.c 的职责

main.c 不应该堆很多底层驱动细节。

当前 main.c 只负责：

* 调用 mpu6050_init()
* 定义 mpu6050_raw_data_t data
* 调用 mpu6050_read_raw(&data)
* 打印数据
* 使用 vTaskDelay() 控制读取周期

### 4. mpu6050_raw_data_t 结构体

结构体用于集中保存 MPU6050 原始数据：

```c
typedef struct
{
    int16_t acc_x;
    int16_t acc_y;
    int16_t acc_z;

    int16_t gyro_x;
    int16_t gyro_y;
    int16_t gyro_z;

    float temperature;
} mpu6050_raw_data_t;
```

好处：

* 数据更清晰
* 函数参数更简单
* 后续 gesture.c 可以直接使用该结构体做姿态识别

### 5. CMake 依赖关系

components/mpu6050/CMakeLists.txt：

```cmake
idf_component_register(SRCS "mpu6050.c"
                       INCLUDE_DIRS "include"
                       REQUIRES driver)
```

main/CMakeLists.txt：

```cmake
idf_component_register(SRCS "main.c"
                       INCLUDE_DIRS "."
                       REQUIRES mpu6050)
```

含义：

* mpu6050 组件依赖 ESP-IDF 的 driver 组件
* main 组件依赖自定义的 mpu6050 组件

## 面试可讲点

* 我将 MPU6050 驱动从 main.c 中拆分出来，封装成 ESP-IDF 自定义组件
* 通过 components/mpu6050 管理 MPU6050 驱动代码，提高了项目模块化和可维护性
* mpu6050.h 对外提供 mpu6050_init() 和 mpu6050_read_raw() 接口
* mpu6050.c 内部实现 I2C 初始化、寄存器读写和原始数据读取
* main.c 只负责调用驱动接口和处理应用层逻辑，避免底层驱动代码和业务代码混在一起
* 这种结构方便后续继续扩展 gesture、OLED、button、buzzer、MQTT 等模块

## 明日计划

* 开始 Day11：实现基础姿态识别
* 新建 components/gesture 组件
* 定义 gesture_type_t 枚举
* 根据 acc_x、acc_y、acc_z 判断 LEFT、RIGHT、NOD
* 将 MPU6050 原始数据转换为 gesture
* 在 main.c 中打印当前 gesture
