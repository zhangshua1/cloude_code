#pragma once
#include <Adafruit_MPU6050.h>

struct IMUData {
    float pitch;          // 俯仰角 (度), 正=前倾
    float roll;           // 横滚角 (度)
    float gyro_y;         // Y轴角速度 (deg/s), 正=向前倾倒
    float gyro_x;         // X轴角速度
    float gyro_z;         // Z轴角速度
    float accel_x;
    float accel_y;
    float accel_z;
    float temp_c;         // 温度 (摄氏度)
    unsigned long ts_us;  // 时间戳 (微秒)
};

class IMUManager {
public:
    bool begin();
    void calibrate(unsigned samples = 500, unsigned delay_ms = 2);

    // 返回最新融合数据
    IMUData read();

    // 直接读取原始角速度 (更快, 用于内环)
    float readGyroY();

    bool isCalibrated() const { return _calibrated; }
    float getGyroBiasY() const { return _gyro_bias_y; }

private:
    Adafruit_MPU6050 _mpu;
    float _gyro_bias_y = 0;
    float _gyro_bias_x = 0;
    float _gyro_bias_z = 0;
    bool  _calibrated = false;

    // 互补滤波系数
    static constexpr float ALPHA = 0.96f;
    float _pitch_accel = 0;
    float _roll_accel  = 0;
    unsigned long _last_us = 0;
};
