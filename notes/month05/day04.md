# 第五个月 Day4：完善 AIoT 智能眼镜 README

## 今日目标

精简智能眼镜项目 README，突出真实系统主线、姿态识别、命令映射、UART、WiFi、MQTT 和 OTA 规划。

## 今日完成内容

1. 备份原有每日开发记录
2. 重写项目主页 README
3. 整理 MPU6050 姿态采集流程
4. 整理阈值手势识别算法
5. 整理 gesture 到 command 的映射
6. 整理 OLED 和按键交互
7. 整理 UART 与 MQTT 数据流
8. 补充误识别优化思路
9. 补充 OTA 升级规划
10. 清理公开仓库中的 WiFi 敏感配置

## 当前项目主线

```text
MPU6050 姿态采集
        ↓
阈值手势识别
        ↓
NORMAL / CONTROL 模式判断
        ↓
gesture 到 command 映射
        ├── OLED 状态显示
        ├── UART JSON 输出
        └── MQTT 命令发布