#include "pid_controller.h"

float PIDController::compute(float setpoint, float measurement, float dt) {
    if (dt <= 0) return _state.prev_output;

    float error = setpoint - measurement;

    // 比例项
    float p_term = _params.kp * error;

    // 积分项 (带限幅抗饱和)
    _state.integral += error * dt;
    if (_params.integral_limit > 0) {
        _state.integral = constrain(_state.integral,
                                    -_params.integral_limit,
                                     _params.integral_limit);
    }
    float i_term = _params.ki * _state.integral;

    // 微分项 (对误差微分，首次跳变做平滑)
    float d_term = 0;
    if (!_state.first_run) {
        d_term = _params.kd * (error - _state.prev_error) / dt;
    }
    _state.prev_error = error;
    _state.first_run = false;

    float raw_output = p_term + i_term + d_term;

    // 低通滤波平滑输出
    float filtered = _params.lowpass_alpha * _state.prev_output +
                     (1.0f - _params.lowpass_alpha) * raw_output;
    _state.prev_output = filtered;

    // 输出限幅
    if (_params.output_limit > 0) {
        filtered = constrain(filtered, -_params.output_limit, _params.output_limit);
    }
    return filtered;
}

void PIDController::reset() {
    _state.integral = 0;
    _state.prev_error = 0;
    _state.prev_output = 0;
    _state.first_run = true;
}
