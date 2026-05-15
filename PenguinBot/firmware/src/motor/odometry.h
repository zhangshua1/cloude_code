#pragma once
#include "encoder.h"
#include "tb6612_driver.h"

class Odometry {
public:
    static Odometry& instance() {
        static Odometry inst;
        return inst;
    }
    void begin(TB6612* motors);
    void update();  // 定期调用 (50Hz) —— 计算轮速并传递给 TB6612

private:
    Odometry() = default;
    TB6612* _motors = nullptr;
    int32_t _prev_l = 0, _prev_r = 0;
    unsigned long _prev_ms = 0;

    float _wheel_circumference_m = 0.045f * PI;  // Φ45mm 轮径
    static constexpr float WHEEL_DIAMETER_M = 0.045f;
};
