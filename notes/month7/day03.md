# 第七阶段 Day03 学习记录

## 一、今日完成内容

### 1. 综合面试复习

完成 12 道嵌入式综合面试题，覆盖：

- C 二级指针
- 函数指针与回调
- struct 内存对齐
- I2C 通信时序
- SPI CPOL / CPHA
- UART DMA + IDLE
- FreeRTOS 任务栈
- 软件定时器与硬件定时器
- epoll LT / ET
- TCP partial read / write
- ESP32 OTA
- ROS2 与 AIoT 系统架构

### 今日薄弱点

重点补强：

- UART DMA + IDLE
- 软件 Timer 与硬件 Timer
- epoll ET
- OTA 完整执行顺序
- ROS2 在项目中的定位


---

## 二、综合笔试训练

完成 20 道综合笔试题，覆盖：

- C / C++
- STM32
- FreeRTOS
- Linux
- TCP/IP
- OTA
- ROS2

### 易错点

#### 1. 数组指针与指针数组

```c
int *p[10];
表示：

10 个 int * 组成的数组。

int (*p)[10];

表示：

指向包含 10 个 int 元素数组的指针。

2. struct 内存对齐
struct S
{
    char a;
    int b;
    short c;
};

典型 32 位平台大小通常为 12 字节。

原因：

每个成员需要满足自身对齐要求
编译器可能插入 padding
整个结构体大小通常为最大对齐要求的整数倍
3. std::move

std::move() 本身不会移动资源。

它只是将对象转换为右值，使移动构造或移动赋值可以被调用。

4. UART DMA + IDLE

DMA：

负责外设和内存之间的数据搬运。

IDLE：

用于判断 UART 一段可变长数据什么时候暂时接收完成。

两者作用不同。

5. PWM

常见公式：

PWM频率 =
TimerClock /
((PSC + 1) × (ARR + 1))

提高频率一般可以减小 PSC 或 ARR。

三、Day03 代码训练

代码统一放入：

c-data-structure/seventh_stage/day03/
1. command_dispatch.c

实现：

Command
   ↓
函数指针表
   ↓
对应 Handler

使用：

typedef void (*command_handler_t)(void);

建立：

CMD_OPEN    -> handle_open
CMD_GRAB    -> handle_grab
CMD_RELEASE -> handle_release
CMD_STOP    -> handle_stop

主要知识点：

enum
typedef
函数指针
函数指针数组
边界检查
表驱动设计

实际运行结果：

HAND_OPEN
HAND_GRAB
HAND_STOP
Invalid command:100

面试可讲：

我使用函数指针表实现命令分发，将命令枚举和处理函数进行映射，相比大量 switch-case，可以降低分发代码的耦合并提高扩展性。

2. binary_search_bounds.cpp

实现：

find_first()
find_last()

对于：

1 2 2 2 4 5 7

查找 2：

first = 1
last = 3

核心：

查左边界：

result = mid;
right = mid - 1;

查右边界：

result = mid;
left = mid + 1;

时间复杂度：

O(log n)

空间复杂度：

O(1)
四、ESP32-S3 OTA Day03
1. OTA Writer 状态保护

在 OTA Writer 中增加：

static bool ota_started = false;

防止以下非法情况：

未 begin
↓
直接 write / finish

只有：

esp_ota_begin() 成功

以后才允许继续写入。

2. ota_writer_begin()

主要流程：

esp_ota_get_next_update_partition()
        ↓
esp_ota_begin()
        ↓
ota_started = true

用于获取备用 OTA 分区并启动升级过程。

3. ota_writer_write()

增加：

OTA 状态检查
data 判空
len 检查
esp_ota_write 错误日志

作用：

将固件分块写入 OTA 分区。

4. ota_writer_finish()

流程：

esp_ota_end()
        ↓
镜像检查成功
        ↓
esp_ota_set_boot_partition()

关键原则：

必须先完成固件写入和镜像检查，再切换启动分区。

5. ota_writer_abort()

新增接口：

esp_err_t ota_writer_abort(void);

当下载、网络或者 Flash 写入出现异常时：

OTA异常
  ↓
esp_ota_abort()
  ↓
清除OTA状态
  ↓
保留旧版本

用于安全终止本次升级。

五、OTA 当前软件架构
IDLE
 │
 │ ota_writer_begin()
 ▼
STARTED
 │
 ├── ota_writer_write()
 │       ↓
 │    分块写Flash
 │
 ├── 正常
 │      ↓
 │   ota_writer_finish()
 │      ↓
 │   set_boot_partition
 │
 └── 异常
        ↓
     ota_writer_abort()
        ↓
       IDLE

OTA 正确顺序：

get_next_partition
        ↓
begin
        ↓
write
        ↓
end
        ↓
set_boot_partition
        ↓
restart
六、今日环境问题

VS Code 重启后出现：

idf.py: command not found

原因：

ESP-IDF 环境变量没有自动加载。

同时曾出现临时 activation 脚本异常。

处理：

rm -rf /tmp/esp_idf_activate_yang
source ~/esp/esp-idf/export.sh

ESP-IDF 环境恢复正常。

七、实验现象
已验证
OTA Writer 软件接口完成
OTA 状态保护逻辑完成
OTA abort 异常退出机制完成
函数指针命令分发表运行正常
二分左右边界算法完成
尚未验证

由于今日未携带 ESP32-S3 开发板：

未进行 Flash
未进行 Monitor
未读取实际 Running Partition
未验证 Next OTA Partition
未完成真实 V1 -> V2 OTA

以上板载验证顺延至 Day04。