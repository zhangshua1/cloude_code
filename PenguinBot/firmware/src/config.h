#pragma once
#include <Arduino.h>

// ============================================================
// PenguinBot 全局配置
// ============================================================

#define BOT_NAME       "PenguinBot"
#define FIRMWARE_VER   "1.0.0"

// --- WiFi AP ---
#define WIFI_SSID_PREFIX "PenguinBot-"
#define WIFI_PASSWORD    "penguin123"

// --- 引脚定义 ---

// TFT Display (SPI2 - FSPI)
#define TFT_SCLK  12
#define TFT_MOSI  11
#define TFT_DC    10
#define TFT_RST   9
#define TFT_CS    8
#define TFT_BL    13

// I2C0 (共享总线)
#define I2C_SDA   4
#define I2C_SCL   5
#define I2C_FREQ  400000  // 400kHz Fast mode

// TB6612FNG 电机驱动
#define MOT_AIN1  38
#define MOT_AIN2  39
#define MOT_PWMA  40
#define MOT_BIN1  41
#define MOT_BIN2  42
#define MOT_PWMB  43
#define MOT_STBY  44

// AS5600 编码器 (正交编码模式)
#define ENC_L_A   1
#define ENC_L_B   2
#define ENC_R_A   6
#define ENC_R_B   7

// I2S 音频 — 全双工 (共享 BCLK/WS)
//   麦克风: INMP441 (I2S slave, L/R→GND = 左声道)
//   功放:   MAX98357 (I2S slave)
#define I2S_BCK      15
#define I2S_WS       16
#define I2S_DIN      18   // INMP441 SD → ESP32 GPIO18
#define I2S_DOUT     17   // ESP32 → MAX98357 DIN

// 唤醒按键 (BOOT 复用，拉低触发)
#define PIN_WAKE_BTN 0

// 音频参数
#define AUDIO_SAMPLE_RATE   16000
#define AUDIO_BITS_PER_SAMPLE 16
#define AUDIO_RECORD_SECS   5      // 最长录音秒数
#define AUDIO_BUFFER_SAMPLES (AUDIO_SAMPLE_RATE * AUDIO_RECORD_SECS)

// 语音活动检测 (VAD)
#define VAD_ENERGY_THRESHOLD 500   // 环境自适应阈值下限
#define VAD_SILENCE_MS       1200  // 连续静音多久判定说话结束

// 模拟输入
#define PIN_BAT_ADC  14    // 电池电压分压 (VBAT/2)
#define BAT_R1       10000 // 分压电阻上臂 10kΩ
#define BAT_R2       10000 // 分压电阻下臂 10kΩ

// NeoPixel 状态 LED
#define PIN_NEOPIXEL  48
#define NUM_PIXELS    1

// I2C 地址
#define MPU6050_ADDR   0x68
#define PCA9685_ADDR   0x40

// PCA9685 舵机通道分配
#define SERVO_HEAD_YAW   0   // 头部摇头
#define SERVO_L_HIP      1   // 左髋
#define SERVO_L_KNEE     2   // 左膝
#define SERVO_R_HIP      3   // 右髋
#define SERVO_R_KNEE     4   // 右膝

// 舵机角度限位
#define SERVO_ANGLE_MIN   30
#define SERVO_ANGLE_MAX   150
#define SERVO_ANGLE_MID   90

// --- 控制参数 ---

// 平衡控制频率
#define BALANCE_HZ          200
#define BALANCE_PERIOD_MS   (1000 / BALANCE_HZ)

// 默认 PID 参数
#define PID_ANGLE_KP_DEFAULT   35.0f
#define PID_ANGLE_KI_DEFAULT   4.0f
#define PID_ANGLE_KD_DEFAULT   0.3f
#define PID_ANGLE_ILIMIT       500.0f
#define PID_ANGLE_OLIMIT       300.0f

#define PID_RATE_KP_DEFAULT    0.8f
#define PID_RATE_KI_DEFAULT    0.15f
#define PID_RATE_KD_DEFAULT    0.01f
#define PID_RATE_ILIMIT        100.0f
#define PID_RATE_OLIMIT        255.0f

// 电池保护
#define BAT_LOW_WARN_V     6.6f   // 3.3V/cell
#define BAT_CRITICAL_V     6.2f   // 3.1V/cell
#define BAT_FULL_V         8.4f
#define BAT_EMPTY_V        6.0f
#define BAT_SAMPLE_COUNT   10

// 电机
#define MOTOR_PWM_MAX      255
#define MOTOR_PWM_LIMIT    180   // 70% 限幅 (N20 @ 7.4V)
#define MOTOR_STANDBY_MS   50
#define ENCODER_CPR        11    // N20 编码器 11 pulses/rev (电机端)
#define GEAR_RATIO         690.0f

// 日志
#define LOG_BUF_SIZE       1024
#define LOG_MAX_ENTRIES    256

// WebSocket
#define WS_PORT            81
#define TELEMETRY_MS       50    // 20Hz 遥测推送

// --- AI 对话配置 ---

// STT (语音识别) 服务商: "baidu" | "openai" | "custom"
#define AI_STT_PROVIDER   "baidu"
#define AI_STT_URL        ""   // 留空用内置默认
#define AI_STT_API_KEY    ""

// LLM (大模型) 服务商: "openai" | "deepseek" | "claude" | "custom"
#define AI_LLM_PROVIDER   "deepseek"
#define AI_LLM_URL        ""   // https://api.deepseek.com/v1/chat/completions
#define AI_LLM_API_KEY    ""
#define AI_LLM_MODEL      "deepseek-chat"

// 系统提示词
#define AI_SYSTEM_PROMPT  "你是一只叫 PenguinBot 的企鹅机器人，身高15厘米，桌面级轮足机器人。请用可爱、元气、简短的风格回复，每句话不超过60个字。偶尔发出企鹅叫声。"

// TTS (语音合成) 服务商: "edge" | "openai" | "baidu" | "custom"
#define AI_TTS_PROVIDER   "edge"
#define AI_TTS_URL        ""
#define AI_TTS_API_KEY    ""

// 最大对话轮次 (上下文窗口)
#define AI_MAX_TURNS      20
#define AI_HTTP_TIMEOUT   15000   // HTTP 请求超时 ms

// 触发方式
#define AI_TRIGGER_BUTTON  1     // 按键触发
#define AI_TRIGGER_WAKE    0     // 唤醒词 (需要 ESP-SR, 默认关闭)

// --- FreeRTOS 任务优先级 ---
#define PRIO_BALANCE       25
#define PRIO_CONVERSATION  12
#define PRIO_SERVO         10
#define PRIO_DISPLAY       8
#define PRIO_COMM          5
#define PRIO_AUDIO         3
#define PRIO_MONITOR       1

// --- NVS Keys ---
#define NVS_NS             "penguinbot"
#define NVS_KEY_ANGLE_KP   "a_kp"
#define NVS_KEY_ANGLE_KI   "a_ki"
#define NVS_KEY_ANGLE_KD   "a_kd"
#define NVS_KEY_RATE_KP    "r_kp"
#define NVS_KEY_RATE_KI    "r_ki"
#define NVS_KEY_RATE_KD    "r_kd"
#define NVS_KEY_STT_KEY    "stt_key"
#define NVS_KEY_LLM_KEY    "llm_key"
#define NVS_KEY_TTS_KEY    "tts_key"
#define NVS_KEY_LLM_URL    "llm_url"
#define NVS_KEY_SYS_PROMPT "sys_prompt"
