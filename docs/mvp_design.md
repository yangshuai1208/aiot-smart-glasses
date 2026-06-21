# 智能眼镜控制端 MVP 设计

## 一、MVP 定义

MVP 是 Minimum Viable Product，也就是最小可用产品。

智能眼镜控制端 MVP 的目标是先完成最小可运行闭环：

```text
头部动作
    ↓
MPU6050 姿态数据
    ↓
gesture 识别
    ↓
cmd 命令生成
    ↓
OLED 显示
    ↓
串口 / MQTT 输出