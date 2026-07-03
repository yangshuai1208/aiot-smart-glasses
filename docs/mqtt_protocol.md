# MQTT 通信协议设计

## 1. 通信角色

| 角色 | 设备 | 作用 |
|---|---|---|
| Publisher | ESP32-S3 | 发布手势控制命令 |
| Broker | Mosquitto | MQTT 消息服务器 |
| Subscriber | Linux Gateway | 订阅命令并转发给 STM32 |

---

## 2. Topic 设计

```text
aiot/glasses/cmd
```

字段含义：

```text
aiot     → 项目类型
glasses  → 智能眼镜控制端
cmd      → 控制命令
```

后续可扩展 Topic：

```text
aiot/glasses/status
aiot/hand/status
aiot/gateway/log
```

---

## 3. Payload 格式

ESP32-S3 发布 JSON 数据：

```json
{"device":"glasses01","cmd":"OPEN","seq":1}
```

字段说明：

| 字段 | 含义 |
|---|---|
| device | 设备编号 |
| cmd | 控制命令 |
| seq | 消息序号，用于日志追踪和命令去重 |

---

## 4. 支持的命令

| MQTT cmd | 含义 |
|---|---|
| OPEN | 五指张开 |
| GRAB | 五指抓取 |
| RELEASE | 五指释放 |
| STOP | 急停 / 回中位 |

---

## 5. Linux Gateway 命令映射

| MQTT cmd | Gateway 转换 | STM32 UART 命令 | 舵机动作 |
|---|---|---|---|
| OPEN | HAND_OPEN | HAND_OPEN\n | 五指张开 |
| GRAB | HAND_GRAB | HAND_GRAB\n | 五指抓取 |
| RELEASE | HAND_RELEASE | HAND_RELEASE\n | 五指释放 |
| STOP | HAND_STOP | HAND_STOP\n | 回到安全中位 |
| 其他 | HAND_NONE | 不发送 | 不动作 |

---

## 6. Gateway 稳定性设计

Linux Gateway 不直接盲目转发，而是进行以下处理：

```text
MQTT JSON 接收
        ↓
提取 cmd 字段
        ↓
判断命令是否合法
        ↓
普通重复命令 1 秒内去重
        ↓
STOP 急停命令不去重
        ↓
转换为 HAND_xxx
        ↓
UART 重试发送
        ↓
STM32 执行
```

---

## 7. 异常处理

| 异常情况 | Gateway 处理 |
|---|---|
| JSON 缺少 cmd 字段 | 丢弃 |
| cmd 为 HELLO / TEST 等无效值 | 丢弃 |
| 普通命令短时间重复 | 去重 |
| STOP 连续出现 | 不去重，继续下发 |
| UART 发送失败 | 重试 |
| MQTT 断开 | 自动重连 |

---

## 8. 示例日志

```text
[12:03:10][MQTT] TOPIC = aiot/glasses/cmd
[12:03:10][MQTT] PAYLOAD = {"device":"glasses01","cmd":"OPEN","seq":1}
[12:03:10][CMD] cmd=OPEN, stm32_cmd=HAND_OPEN
UART SEND    = HAND_OPEN
[12:03:10][OK] forward success, cmd=HAND_OPEN
```

---

## 9. 为什么使用 MQTT

1. MQTT 适合 IoT 场景。
2. 控制端和执行端解耦。
3. ESP32 不需要直接关心 STM32 串口协议。
4. STM32 不需要支持 WiFi / MQTT。
5. 后续方便扩展手机 App、云端日志、状态上报。