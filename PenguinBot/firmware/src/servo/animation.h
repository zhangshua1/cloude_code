#pragma once
#include <Arduino.h>
#include <ArduinoJson.h>
#include "pca9685_driver.h"

struct Keyframe {
    uint8_t channels[5];  // 目标角度 (255=保持)
    uint16_t duration_ms;
};

class AnimationEngine {
public:
    static AnimationEngine& instance() {
        static AnimationEngine inst;
        return inst;
    }

    void begin();
    void update();  // 50Hz 调用

    void playSequence(const Keyframe* frames, uint8_t count, bool loop);
    void playPreset(const char* name);
    void stop();
    bool isPlaying() const { return _playing; }

    // 预设动作
    static const Keyframe PRESET_WAVE[];
    static const Keyframe PRESET_DANCE[];
    static const Keyframe PRESET_CROUCH[];
    static const Keyframe PRESET_STRETCH[];
    static const Keyframe PRESET_LOOK_AROUND[];
    static const Keyframe PRESET_IDLE[];

private:
    AnimationEngine() = default;
    const Keyframe* _frames = nullptr;
    uint8_t _count = 0;
    uint8_t _index = 0;
    bool _playing = false;
    bool _loop = false;
    unsigned long _start_ms = 0;
};
