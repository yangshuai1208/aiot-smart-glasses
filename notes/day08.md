# Day08 学习记录

## 今日目标

* 新建 aiot-smart-glasses 智能眼镜控制端仓库
* 编写 README.md 初版
* 确定智能眼镜控制端硬件组成
* 设计项目目录结构
* 新增 docs/hardware.md
* 新增 docs/mvp_design.md
* 完成 GitHub 提交

## 今日完成

* 创建 aiot-smart-glasses 项目目录
* 明确智能眼镜控制端在系统中的位置
* 确定 ESP32-S3、MPU6050、OLED、按键、蜂鸣器作为 MVP 硬件组成
* 编写 README.md，说明项目简介、MVP 目标、软件模块和后续计划
* 编写 docs/hardware.md，说明硬件清单、连接关系和供电注意事项
* 编写 docs/mvp_design.md，明确智能眼镜控制端 MVP 的功能标准
* 明确第 2 周开发目标：完成智能眼镜控制端 MVP

## 今日代码提交

* commit: feat: create aiot smart glasses project

## 实验现象

* aiot-smart-glasses 仓库已建立
* README.md 可以说明智能眼镜控制端项目定位
* docs/hardware.md 可以说明硬件组成
* docs/mvp_design.md 可以说明 MVP 合格标准
* 后续可以开始 MPU6050 通信和姿态识别开发

## 遇到的问题

1. 现象：智能眼镜功能听起来很多，容易一开始做得过大
2. 原因：如果一开始加入 AR、摄像头、OTA、低功耗和复杂外壳，会拖慢项目进度
3. 解决：先确定 MVP，只做 MPU6050 姿态识别、OLED 显示、按键模式切换和命令输出

## 面试可讲点

* 智能眼镜控制端是整体系统中的可穿戴输入端
* 第一版采用 MVP 思路，不做复杂 AR 或视觉识别
* 使用 MPU6050 识别头部姿态
* 使用 OLED 显示当前模式、手势和命令
* 使用按键切换 NORMAL / CONTROL 模式
* 后续通过 MQTT JSON 将 gesture 和 cmd 上传给 Linux IoT Gateway
* MVP 先验证智能眼镜作为控制端的可行性，再逐步扩展 OTA 和系统联调

## 明日计划

* 开始 Day9：跑通 MPU6050 I2C 通信
* 确认 MPU6050 接线
* 读取 WHO_AM_I 寄存器
* 读取加速度和角速度原始数据
* 编写 notes/day09.md
