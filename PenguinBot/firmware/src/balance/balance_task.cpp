#include "balance_task.h"
#include "../system/battery_monitor.h"
#include <esp_timer.h>
#include <esp_wifi.h>

bool BalanceTask::begin(IMUManager* imu, TB6612* motors) {
    _imu = imu;
    _motors = motors;

    // 加载保存的 PID 或使用默认值
    PIDParams a_p, r_p;
    a_p.kp = PID_ANGLE_KP_DEFAULT; a_p.ki = PID_ANGLE_KI_DEFAULT;
    a_p.kd = PID_ANGLE_KD_DEFAULT; a_p.integral_limit = PID_ANGLE_ILIMIT;
    a_p.output_limit = PID_ANGLE_OLIMIT;

    r_p.kp = PID_RATE_KP_DEFAULT; r_p.ki = PID_RATE_KI_DEFAULT;
    r_p.kd = PID_RATE_KD_DEFAULT; r_p.integral_limit = PID_RATE_ILIMIT;
    r_p.output_limit = PID_RATE_OLIMIT;

    _pid.angle.setParams(a_p);
    _pid.rate.setParams(r_p);
    _last_balance_us = esp_timer_get_time();
    return true;
}

void BalanceTask::run() {
    unsigned long now = esp_timer_get_time();
    float dt = (now - _last_balance_us) / 1e6f;
    _last_balance_us = now;

    if (dt <= 0 || dt > 0.05f) dt = 0.005f;  // 钳制异常时间步

    // 读取 IMU
    IMUData imu = _imu->read();
    _current_pitch  = imu.pitch;
    _current_gyro_y = imu.gyro_y;

    // 跌倒检测
    if (fabs(_current_pitch) > FALL_ANGLE) {
        if (_fall_start_ms == 0) {
            _fall_start_ms = millis();
        } else if (millis() - _fall_start_ms > 300) {
            _mode = BalanceMode::FALL;
            _motors->stop();
            return;
        }
    } else {
        _fall_start_ms = 0;
    }

    if (_mode == BalanceMode::FALL || _mode == BalanceMode::ERROR) {
        _motors->stop();
        return;
    }

    if (_mode == BalanceMode::IDLE || _mode == BalanceMode::CALIBRATE) {
        _motors->stop();
        _pid.angle.reset();
        _pid.rate.reset();
        return;
    }

    // 移动指令处理
    if (_mode == BalanceMode::MOVE && _move_cmd.active) {
        if (_move_cmd.duration_ms > 0 &&
            millis() - _move_cmd.issued_at > _move_cmd.duration_ms) {
            _move_cmd.active = false;
            _move_cmd.forward_speed = 0;
            _move_cmd.turn_rate = 0;
        }
        // 速度 → 目标角度映射 (前倾加速)
        _angle_setpoint = _move_cmd.forward_speed * 0.05f;  // 约 ±12.75°
    } else {
        _angle_setpoint = 0;
    }

    // ---- 级联 PID ----
    // 外环：角度控制
    float rate_setpoint = _pid.angle.compute(_angle_setpoint, _current_pitch, dt);

    // 内环：角速度控制
    float motor_out = _pid.rate.compute(rate_setpoint, _current_gyro_y, dt);

    // 差速转向
    float turn = (_mode == BalanceMode::MOVE) ? _move_cmd.turn_rate : 0;
    int16_t l_pwm = constrain((int16_t)(motor_out - turn), -MOTOR_PWM_LIMIT, MOTOR_PWM_LIMIT);
    int16_t r_pwm = constrain((int16_t)(motor_out + turn), -MOTOR_PWM_LIMIT, MOTOR_PWM_LIMIT);

    _motors->setLeftSpeed(l_pwm);
    _motors->setRightSpeed(r_pwm);
}

void BalanceTask::setMode(BalanceMode m) {
    if (m != _mode) {
        log_i("Balance mode: %d → %d", (int)_mode, (int)m);
        _mode = m;
    }
}

void BalanceTask::move(float speed, float turn, unsigned long duration_ms) {
    _move_cmd.active = true;
    _move_cmd.forward_speed = constrain(speed, -255.0f, 255.0f);
    _move_cmd.turn_rate = constrain(turn, -255.0f, 255.0f);
    _move_cmd.issued_at = millis();
    _move_cmd.duration_ms = duration_ms;
    _mode = BalanceMode::MOVE;
}

void BalanceTask::stop() {
    _move_cmd.active = false;
    _move_cmd.forward_speed = 0;
    _move_cmd.turn_rate = 0;
    _mode = BalanceMode::STAND;
}

void BalanceTask::setAnglePid(const PIDParams& p) { _pid.angle.setParams(p); }
void BalanceTask::setRatePid(const PIDParams& p)  { _pid.rate.setParams(p); }

TelemetrySnapshot BalanceTask::getTelemetry() {
    TelemetrySnapshot ts = {};
    ts.angle_deg        = _current_pitch;
    ts.angular_velocity = _current_gyro_y;
    ts.angle_target     = _angle_setpoint;
    ts.motor_l_pwm      = _motors ? _motors->getLeftSpeed() : 0;
    ts.motor_r_pwm      = _motors ? _motors->getRightSpeed() : 0;
    ts.motor_l_rpm      = _motors ? _motors->getLeftRPM() : 0;
    ts.motor_r_rpm      = _motors ? _motors->getRightRPM() : 0;
    ts.battery_v        = BatteryMonitor::instance().getVoltage();
    ts.battery_pct      = BatteryMonitor::instance().getPercentage();
    ts.uptime_s         = millis() / 1000;
    ts.free_heap        = ESP.getFreeHeap();
    ts.wifi_rssi        = WiFi.status() == WL_CONNECTED ? WiFi.RSSI() : 0;

    auto& ap = _pid.angle.params();
    auto& rp = _pid.rate.params();
    ts.pid_angle_kp = ap.kp; ts.pid_angle_ki = ap.ki; ts.pid_angle_kd = ap.kd;
    ts.pid_rate_kp  = rp.kp; ts.pid_rate_ki  = rp.ki; ts.pid_rate_kd  = rp.kd;

    // 从 IMU 获取温度
    sensors_event_t a, g, temp;
    extern Adafruit_MPU6050 _shared_mpu;  // FIXME: 更好的方式
    ts.cpu_temp_c = temperatureRead();    // ESP32 内部温度

    return ts;
}
