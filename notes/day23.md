Day23 学习记录：五指角度校准与动作安全优化
1. 在 Day22 五舵机动作组基础上，增加每根手指的安全角度范围
2. 使用 finger_min_angles / finger_max_angles 限制每个舵机角度
3. 使用 current_angles 记录当前角度
4. 使用平滑移动函数逐步移动到目标角度
5. OPEN / GRAB / RELEASE / STOP 改为可校准角度表
6. 后续根据机械手结构微调每根手指的角度