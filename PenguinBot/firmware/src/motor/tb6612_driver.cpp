#include "tb6612_driver.h"

void TB6612::begin() {
    pinMode(MOT_AIN1, OUTPUT);
    pinMode(MOT_AIN2, OUTPUT);
    pinMode(MOT_PWMA, OUTPUT);
    pinMode(MOT_BIN1, OUTPUT);
    pinMode(MOT_BIN2, OUTPUT);
    pinMode(MOT_PWMB, OUTPUT);
    pinMode(MOT_STBY, OUTPUT);
    digitalWrite(MOT_STBY, HIGH);  // 使能

    ledcSetup(0, 20000, 8);  // 20kHz, 8-bit
    ledcSetup(1, 20000, 8);
    ledcAttachPin(MOT_PWMA, 0);
    ledcAttachPin(MOT_PWMB, 1);
}

void TB6612::setLeftSpeed(int16_t pwm) {
    _l_speed = constrain(pwm, -MOTOR_PWM_LIMIT, MOTOR_PWM_LIMIT);
    _setMotor(MOT_AIN1, MOT_AIN2, MOT_PWMA, _l_speed);
}

void TB6612::setRightSpeed(int16_t pwm) {
    _r_speed = constrain(pwm, -MOTOR_PWM_LIMIT, MOTOR_PWM_LIMIT);
    _setMotor(MOT_BIN1, MOT_BIN2, MOT_PWMB, _r_speed);
}

void TB6612::stop() {
    _l_speed = 0; _r_speed = 0;
    digitalWrite(MOT_AIN1, LOW); digitalWrite(MOT_AIN2, LOW);
    digitalWrite(MOT_BIN1, LOW); digitalWrite(MOT_BIN2, LOW);
    ledcWrite(0, 0); ledcWrite(1, 0);
}

void TB6612::standby() {
    stop();
    digitalWrite(MOT_STBY, LOW);
    delay(MOTOR_STANDBY_MS);
    digitalWrite(MOT_STBY, HIGH);
}

void TB6612::_setMotor(int ain1, int ain2, int pwma, int16_t speed) {
    if (speed > 0) {
        digitalWrite(ain1, HIGH); digitalWrite(ain2, LOW);
    } else if (speed < 0) {
        digitalWrite(ain1, LOW);  digitalWrite(ain2, HIGH);
        speed = -speed;
    } else {
        digitalWrite(ain1, LOW); digitalWrite(ain2, LOW);
    }
    // PWM 通道: 0=左, 1=右
    int ch = (pwma == MOT_PWMA) ? 0 : 1;
    ledcWrite(ch, speed);
}
