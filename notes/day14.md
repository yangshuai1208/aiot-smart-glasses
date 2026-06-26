# Day14 学习记录：command 命令生成模块

## 今日目标

今天完成 command 命令生成模块，将 main.c 中的手势到命令映射逻辑拆分出去，使项目结构更加清晰。

## 今日完成内容

- 新建 components/command 组件
- 新建 command.h
- 新建 command.c
- 定义 command_type_t 命令枚举
- 实现 command_from_gesture()
- 实现 command_to_string()
- 修改 main.c，删除原来的 cmd_from_gesture()
- main.c 通过 command 模块获取控制命令
- 修改 main/CMakeLists.txt，加入 REQUIRES command
- 编译通过

## 新增文件

```text
components/command/CMakeLists.txt
components/command/command.c
components/command/include/command.h
notes/day14.md