#pragma once
#include <Arduino.h>
#include <driver/i2s.h>

struct AudioBuffer {
    int16_t* data = nullptr;
    size_t   len = 0;       // 采样数
    uint32_t timestamp_ms = 0;
    bool     voice_detected = false;
};

class AudioCapture {
public:
    static AudioCapture& instance() {
        static AudioCapture inst;
        return inst;
    }
    bool begin();
    void end();

    // 开始录音，返回实际录音采样数
    // 自动 VAD 检测：静音超过 VAD_SILENCE_MS 自动停止
    size_t record(AudioBuffer& buf, uint32_t timeout_ms = 5000);

    // 实时能量检测 (用于 VAD)
    float getRMS() const { return _rms; }

    // 调试：回放最新录音
    void playbackLast();

private:
    AudioCapture() = default;
    bool _running = false;

    // 能量
    float _rms = 0;

    // 上一次录音数据 (调试回放)
    static constexpr size_t LAST_BUF_SAMPLES = 16000 * 3;
    int16_t* _last_buf = nullptr;
    size_t   _last_len = 0;

    // I2S 驱动封装
    static bool _i2sInstalled;

    // DMA 缓冲区 (内部)
    static constexpr size_t DMA_BUF_COUNT = 4;
    static constexpr size_t DMA_BUF_LEN   = 512;  // 采样数
};
