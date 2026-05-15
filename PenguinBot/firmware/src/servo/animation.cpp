#include "animation.h"

// === 预设动作定义 ===
// 每个 Keyframe: {头, 左髋, 左膝, 右髋, 右膝}, duration_ms

const Keyframe AnimationEngine::PRESET_WAVE[] = {
    {{90, 90, 90, 90, 90}, 200},  // 站立
    {{90, 60, 90, 90, 90}, 400},  // 左腿抬起
    {{60, 60, 90, 90, 90}, 300},  // 转头+抬腿
    {{120,80, 90, 90, 90}, 300},  // 头右转
    {{60, 60, 90, 90, 90}, 300},  // 头左转
    {{90, 90, 90, 90, 90}, 500},  // 恢复
};

const Keyframe AnimationEngine::PRESET_DANCE[] = {
    {{90, 90, 120, 90, 120}, 400},  // 蹲下
    {{90, 60, 90, 120, 120}, 400},  // 左抬起
    {{90, 120,120, 60, 90}, 400},   // 右抬起
    {{90, 90, 120, 90, 120}, 400},  // 蹲
    {{90, 90, 90,  90, 90},  600},  // 站起
};

const Keyframe AnimationEngine::PRESET_CROUCH[] = {
    {{90, 90, 90,  90, 90},  500},
    {{90, 80, 120, 80, 120}, 800},
};

const Keyframe AnimationEngine::PRESET_STRETCH[] = {
    {{90, 90, 90,  90, 90},  300},
    {{90, 90, 60,  90, 60},  600},   // 伸腿
    {{90, 90, 90,  90, 90},  600},
};

const Keyframe AnimationEngine::PRESET_LOOK_AROUND[] = {
    {{90, 90, 90, 90, 90}, 300},
    {{40, 90, 90, 90, 90}, 600},   // 左看
    {{140,90, 90, 90, 90}, 600},   // 右看
    {{90, 90, 90, 90, 90}, 400},
};

const Keyframe AnimationEngine::PRESET_IDLE[] = {
    {{90, 90, 90, 90, 90}, 1000},
    {{95, 85, 95, 85, 95}, 2000},  // 微动
    {{90, 90, 90, 90, 90}, 2000},
};

// === 引擎实现 ===

void AnimationEngine::begin() {}

void AnimationEngine::update() {
    if (!_playing || !_frames) return;

    unsigned long elapsed = millis() - _start_ms;
    if (elapsed >= _frames[_index].duration_ms) {
        _index++;
        if (_index >= _count) {
            if (_loop) {
                _index = 0;
            } else {
                _playing = false;
                return;
            }
        }
        _start_ms = millis();

        // 应用当前关键帧
        auto& kf = _frames[_index];
        for (int i = 0; i < 5; i++) {
            if (kf.channels[i] != 255) {
                PCA9685Driver::instance().setAngleSlow(i, kf.channels[i],
                                                       kf.duration_ms);
            }
        }
    }
}

void AnimationEngine::playSequence(const Keyframe* frames, uint8_t count,
                                   bool loop) {
    stop();
    _frames = frames;
    _count = count;
    _loop = loop;
    _index = 0;
    _playing = true;
    _start_ms = millis();

    // 立即应用首帧
    for (int i = 0; i < 5; i++) {
        if (frames[0].channels[i] != 255) {
            PCA9685Driver::instance().setAngleSlow(i, frames[0].channels[i], 200);
        }
    }
}

void AnimationEngine::playPreset(const char* name) {
    if (strcmp(name, "wave") == 0)
        playSequence(PRESET_WAVE, 6, false);
    else if (strcmp(name, "dance") == 0)
        playSequence(PRESET_DANCE, 5, true);
    else if (strcmp(name, "crouch") == 0)
        playSequence(PRESET_CROUCH, 2, false);
    else if (strcmp(name, "stretch") == 0)
        playSequence(PRESET_STRETCH, 3, false);
    else if (strcmp(name, "look") == 0)
        playSequence(PRESET_LOOK_AROUND, 4, false);
    else if (strcmp(name, "idle") == 0)
        playSequence(PRESET_IDLE, 3, true);
}

void AnimationEngine::stop() {
    _playing = false;
    _frames = nullptr;
    _count = 0;
    _loop = false;
}
