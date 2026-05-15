#pragma once
#include "../config.h"
#include "imu_manager.h"
#include "pid_controller.h"
#include "../motor/tb6612_driver.h"

// 遥测快照 (共享内存, 由 balance_task 写入, comm_task 读取)
struct TelemetrySnapshot {
    float angle_deg;
    float angular_velocity;
    float angle_target;
    int16_t motor_l_pwm;
    int16_t motor_r_pwm;
    float   motor_l_rpm;
    float   motor_r_rpm;
    float   battery_v;
    uint8_t battery_pct;
    uint8_t servo_pos[5];
    float   pid_angle_kp, pid_angle_ki, pid_angle_kd;
    float   pid_rate_kp,  pid_rate_ki,  pid_rate_kd;
    uint32_t uptime_s;
    int8_t   wifi_rssi;
    uint32_t free_heap;
    float    cpu_temp_c;
};

// 平衡控制模式
enum class BalanceMode {
    IDLE,       // 不控制，电机释放
    CALIBRATE,  // 正在校准
    STAND,      // 原地站立
    MOVE,       // 受控移动
    FALL,       // 检测到跌倒
    ERROR       // 传感器错误
};

// 移动指令
struct MoveCommand {
    bool  active = false;
    float forward_speed = 0;   // -255..0..255
    float turn_rate = 0;       // -255..0..255
    unsigned long issued_at = 0;
    unsigned long duration_ms = 0;
};

class BalanceTask {
public:
    static BalanceTask& instance() {
        static BalanceTask inst;
        return inst;
    }

    bool begin(IMUManager* imu, TB6612* motors);
    void run();  // 在 200Hz 定时器中调用

    // 控制接口
    void setMode(BalanceMode m);
    BalanceMode mode() const { return _mode; }

    void move(float speed, float turn, unsigned long duration_ms = 0);
    void stop();

    void setAnglePid(const PIDParams& p);
    void setRatePid(const PIDParams& p);

    // 获取遥测快照
    TelemetrySnapshot getTelemetry();

    float getPitch() const { return _current_pitch; }
    bool  isBalancing() const { return _mode == BalanceMode::STAND || _mode == BalanceMode::MOVE; }

private:
    BalanceTask() = default;
    IMUManager* _imu = nullptr;
    TB6612*     _motors = nullptr;

    CascadePID  _pid;
    BalanceMode _mode = BalanceMode::IDLE;
    MoveCommand _move_cmd;

    float _current_pitch = 0;
    float _current_gyro_y = 0;
    float _angle_setpoint = 0;  // 目标角度 (移动时非零)
    unsigned long _last_balance_us = 0;

    // 跌倒检测
    static constexpr float FALL_ANGLE = 45.0f;
    unsigned long _fall_start_ms = 0;
};
