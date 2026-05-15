#include "odometry.h"

void Odometry::begin(TB6612* motors) {
    _motors = motors;
    Encoder::instance().begin();
}

void Odometry::update() {
    auto& enc = Encoder::instance();
    unsigned long now = millis();
    float dt = (now - _prev_ms) / 1000.0f;
    _prev_ms = now;

    if (dt <= 0 || dt > 0.5f) return;

    int32_t dl = enc.getLeft() - _prev_l;
    int32_t dr = enc.getRight() - _prev_r;
    _prev_l = enc.getLeft();
    _prev_r = enc.getRight();

    // RPM = (pulses/s) / (CPR *  gear_ratio) * 60
    float ns = (float)ENCODER_CPR * GEAR_RATIO;
    float rpm_l = (dl / dt) / ns * 60.0f;
    float rpm_r = (dr / dt) / ns * 60.0f;

    if (_motors) {
        _motors->updateLeftRPM(rpm_l);
        _motors->updateRightRPM(rpm_r);
    }
}
