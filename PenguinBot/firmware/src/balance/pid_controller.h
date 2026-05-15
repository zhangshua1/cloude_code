#pragma once
#include <Arduino.h>

struct PIDParams {
    float kp = 0;
    float ki = 0;
    float kd = 0;
    float integral_limit = 500;
    float output_limit = 255;
    float lowpass_alpha = 0.9f; // 0=无滤波, 1=完全平滑
};

struct PIDState {
    float integral = 0;
    float prev_error = 0;
    float prev_output = 0;  // 低通滤波后的输出
    bool  first_run = true;
};

class PIDController {
public:
    void setParams(const PIDParams& p) { _params = p; }
    const PIDParams& params() const { return _params; }
    PIDState& state() { return _state; }

    // 标准 PID 计算
    float compute(float setpoint, float measurement, float dt);

    // 重置积分和上一次误差
    void reset();

private:
    PIDParams _params;
    PIDState  _state;
};

// 便捷的级联 PID 容器
struct CascadePID {
    PIDController angle;
    PIDController rate;
};
