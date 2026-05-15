#pragma once
#include <Arduino.h>
#include <Adafruit_PWMServoDriver.h>
#include "../config.h"

class PCA9685Driver {
public:
    static PCA9685Driver& instance() {
        static PCA9685Driver inst;
        return inst;
    }
    bool begin();
    void setAngle(uint8_t channel, float angle);  // 30°~150°
    void setAngleSlow(uint8_t channel, float target, uint16_t ms);
    float getAngle(uint8_t channel) const { return _angles[channel]; }

    // 批量更新（减少 I2C 事务）
    void update();

private:
    PCA9685Driver() = default;
    Adafruit_PWMServoDriver _pwm = Adafruit_PWMServoDriver(PCA9685_ADDR);
    float _angles[5] = {90, 90, 90, 90, 90};
    float _targets[5] = {90, 90, 90, 90, 90};
    unsigned long _start_ms[5] = {0};
    uint16_t _durations[5] = {0};
    static constexpr float SERVO_MIN_US = 500;
    static constexpr float SERVO_MAX_US = 2500;
};
