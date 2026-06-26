cd ~/aiot-smart-glasses

cat > notes/day13.md << 'EOF'
# Day13 学习记录：按键模式切换

## 今日目标

今天完成智能眼镜控制端的按键交互功能，让系统支持 NORMAL / CONTROL 模式切换，并通过长按按键触发 STOP 安全命令。

## 今日完成内容

- 新增 button 按键组件
- 使用 GPIO4 作为按键输入引脚
- 实现 button_init()
- 实现 button_get_event()
- 支持短按识别
- 支持长按识别
- 短按切换 NORMAL / CONTROL 模式
- 长按触发 STOP 命令
- OLED 显示当前 Mode / Gesture / Cmd / Status
- main.c 中加入 app_mode_t 系统模式管理
- 编译通过

## 新增文件

```text
components/button/CMakeLists.txt
components/button/button.c
components/button/include/button.h
notes/day13.md