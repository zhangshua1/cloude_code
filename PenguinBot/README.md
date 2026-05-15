# PenguinBot - 桌面轮足机器人

> 双轮自平衡 + 企鹅形态 | ESP32-S3 | WebUI 远程调试 | ¥486 BOM

## 快速开始

```bash
# 1. 安装 PlatformIO
pip install platformio

# 2. 编译固件
cd firmware
pio run

# 3. 烧录
pio run -t upload

# 4. 上传 WebUI 到 SPIFFS
pio run -t uploadfs

# 5. 连接 PenguinBot WiFi AP
# SSID: PenguinBot-XXXX  密码: penguin123
# 浏览器打开 http://192.168.4.1
```

## 项目结构

```
PenguinBot/
├── firmware/                 # ESP32-S3 固件 (PlatformIO)
│   ├── platformio.ini
│   ├── src/
│   │   ├── main.cpp          # 入口 + FreeRTOS 任务
│   │   ├── config.h          # 引脚/PID/WiFi 配置
│   │   ├── balance/          # 平衡控制 (IMU + 级联PID + 200Hz任务)
│   │   ├── motor/            # TB6612 驱动 + AS5600 编码器 + 里程计
│   │   ├── servo/            # PCA9685 驱动 + 关键帧动画引擎
│   │   ├── display/          # ST7789 TFT 驱动 + 表情精灵
│   │   ├── comm/             # WiFi AP + WebSocket + JSON 协议
│   │   ├── system/           # 电池监测 + 环形日志
│   │   └── ui/               # SPIFFS WebUI 托管
│   └── data/                 # WebUI 静态资源
│       ├── index.html
│       ├── style.css
│       └── app.js
├── mechanical/               # OpenSCAD 3D 模型 (12 件)
│   ├── config.scad           # 全局参数和工具模块
│   ├── body.scad             # 机身壳体 + 底盖
│   ├── head.scad             # 头部前后壳 + 舵机支架
│   ├── leg.scad              # 大腿 + 小腿 + 髋支架
│   ├── wheel.scad            # TPU 轮毂
│   └── assembly.scad         # 完整装配预览
└── docs/
    └── assembly_guide.md     # 打印与装配指南
```

## 硬件架构

```
ESP32-S3 (240MHz) ←→ I2C → MPU6050 (IMU) + PCA9685 (舵机)
    ├── SPI → ST7789 2.0" TFT (240×320)
    ├── GPIO → TB6612FNG → N20 ×2 (编码器电机)
    ├── I2S → MAX98357 → Speaker
    ├── ADC → 电池电压检测
    └── WiFi AP → WebSocket → WebUI 远程调试面板
```

## 核心特性

- **200Hz 级联 PID** 自平衡控制（角度环 + 角速度环）
- **Core 1 独占** 平衡任务，WiFi/显示等跑在 Core 0
- **10 种表情** 精灵渲染（开心/伤心/生气/眨眼/爱心/睡觉...）
- **6 种预设动作**（挥手/跳舞/蹲下/伸展/张望/待机）
- **WebUI** 3D 姿态可视化 + PID 实时调参 + 舵机编辑器 + 虚拟摇杆
- **OTA 固件升级** + 失败自动回滚
- **电池保护** 低压告警/自动关机

## BOM

| 类别 | 金额 |
|------|------|
| 电子元器件 | ¥281.5 |
| 紧固件 | ¥10 |
| 3D 打印耗材 | ¥195 |
| **合计** | **~¥486** |

详见设计计划文件: `.claude/plans/fluttering-noodling-breeze.md`
