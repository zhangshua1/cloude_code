#pragma once
#include <Arduino.h>
#include "../config.h"

class TB6612 {
public:
    void begin();
    void setLeftSpeed(int16_t pwm);   // 正=前进
    void setRightSpeed(int16_t pwm);
    void stop();
    void standby();

    int16_t getLeftSpeed() const  { return _l_speed; }
    int16_t getRightSpeed() const { return _r_speed; }
    float   getLeftRPM() const    { return _l_rpm; }
    float   getRightRPM() const   { return _r_rpm; }

    // 由编码器中断更新
    void updateLeftRPM(float rpm)  { _l_rpm = rpm; }
    void updateRightRPM(float rpm) { _r_rpm = rpm; }

private:
    int16_t _l_speed = 0;
    int16_t _r_speed = 0;
    float   _l_rpm = 0;
    float   _r_rpm = 0;

    void _setMotor(int ain1, int ain2, int pwma, int16_t speed);
};
