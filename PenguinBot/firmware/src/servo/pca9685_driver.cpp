#include "pca9685_driver.h"

bool PCA9685Driver::begin() {
    if (!_pwm.begin()) {
        log_e("PCA9685 not found at 0x%02X", PCA9685_ADDR);
        return false;
    }
    _pwm.setPWMFreq(50);  // 50Hz 标准舵机频率
    // 所有舵机初始归中位
    for (int i = 0; i < 5; i++) {
        setAngle(i, SERVO_ANGLE_MID);
    }
    return true;
}

void PCA9685Driver::setAngle(uint8_t channel, float angle) {
    if (channel >= 5) return;
    angle = constrain(angle, SERVO_ANGLE_MIN, SERVO_ANGLE_MAX);
    _angles[channel] = angle;
    _targets[channel] = angle;
    _durations[channel] = 0;

    float us = map(angle, 0, 180, SERVO_MIN_US, SERVO_MAX_US);
    uint16_t pulse = (uint16_t)(us / 20000.0f * 4096.0f);
    _pwm.setPWM(channel, 0, pulse);
}

void PCA9685Driver::setAngleSlow(uint8_t channel, float target, uint16_t ms) {
    if (channel >= 5 || ms == 0) return;
    target = constrain(target, SERVO_ANGLE_MIN, SERVO_ANGLE_MAX);
    _targets[channel] = target;
    _start_ms[channel] = millis();
    _durations[channel] = ms;
}

float PCA9685Driver::getAngle(uint8_t channel) const {
    return (channel < 5) ? _angles[channel] : 0;
}

void PCA9685Driver::update() {
    unsigned long now = millis();
    for (int i = 0; i < 5; i++) {
        if (_durations[i] == 0) continue;
        float elapsed = now - _start_ms[i];
        if (elapsed >= _durations[i]) {
            setAngle(i, _targets[i]);
            _durations[i] = 0;
        } else {
            float t = elapsed / _durations[i];
            // ease-in-out
            float eased = t < 0.5f ? 2 * t * t : 1 - pow(-2 * t + 2, 2) / 2;
            float cur = _angles[i] + (_targets[i] - _angles[i]) * eased;
            // 直接设置（不走 setAngle 以避免覆盖 target）
            cur = constrain(cur, SERVO_ANGLE_MIN, SERVO_ANGLE_MAX);
            _angles[i] = cur;
            float us = map(cur, 0, 180, SERVO_MIN_US, SERVO_MAX_US);
            _pwm.setPWM(i, 0, (uint16_t)(us / 20000.0f * 4096.0f));
        }
    }
}
