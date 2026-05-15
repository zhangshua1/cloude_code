#include "imu_manager.h"
#include <esp_timer.h>

bool IMUManager::begin() {
    if (!_mpu.begin(MPU6050_ADDR)) {
        log_e("MPU6050 not found at 0x%02X", MPU6050_ADDR);
        return false;
    }
    _mpu.setAccelerometerRange(MPU6050_RANGE_4_G);
    _mpu.setGyroRange(MPU6050_RANGE_250_DEG);
    _mpu.setFilterBandwidth(MPU6050_BAND_44_HZ);  // 44Hz 低通，减少高频噪声
    _last_us = esp_timer_get_time();
    return true;
}

void IMUManager::calibrate(unsigned samples, unsigned delay_ms) {
    float sum_gx = 0, sum_gy = 0, sum_gz = 0;

    for (unsigned i = 0; i < samples; i++) {
        sensors_event_t a, g, t;
        _mpu.getEvent(&a, &g, &t);
        sum_gx += g.gyro.x;
        sum_gy += g.gyro.y;
        sum_gz += g.gyro.z;
        delay(delay_ms);
    }

    _gyro_bias_x = sum_gx / samples;
    _gyro_bias_y = sum_gy / samples;
    _gyro_bias_z = sum_gz / samples;
    _calibrated = true;

    log_i("IMU calibrated: gyro_bias=(%.3f, %.3f, %.3f) deg/s",
          _gyro_bias_x, _gyro_bias_y, _gyro_bias_z);
}

IMUData IMUManager::read() {
    sensors_event_t a, g, t;
    _mpu.getEvent(&a, &g, &t);

    unsigned long now = esp_timer_get_time();
    float dt = (now - _last_us) / 1e6f;
    _last_us = now;

    // 去偏置
    float gy = g.gyro.y - _gyro_bias_y;
    float gx = g.gyro.x - _gyro_bias_x;
    float gz = g.gyro.z - _gyro_bias_z;

    // 加速度计推算角度
    float pitch_acc = atan2(-a.acceleration.x,
                            sqrt(a.acceleration.y * a.acceleration.y +
                                 a.acceleration.z * a.acceleration.z))
                      * 180.0f / PI;
    float roll_acc  = atan2(a.acceleration.y, a.acceleration.z) * 180.0f / PI;

    if (dt > 0 && dt < 0.1f) {
        // 互补滤波
        _pitch_accel = ALPHA * (_pitch_accel + gy * dt) + (1.0f - ALPHA) * pitch_acc;
        _roll_accel  = ALPHA * (_roll_accel  + gx * dt) + (1.0f - ALPHA) * roll_acc;
    } else {
        _pitch_accel = pitch_acc;
        _roll_accel  = roll_acc;
    }

    IMUData data;
    data.pitch    = _pitch_accel;
    data.roll     = _roll_accel;
    data.gyro_y   = gy;
    data.gyro_x   = gx;
    data.gyro_z   = gz;
    data.accel_x  = a.acceleration.x;
    data.accel_y  = a.acceleration.y;
    data.accel_z  = a.acceleration.z;
    data.temp_c   = t.temperature;
    data.ts_us    = now;
    return data;
}

float IMUManager::readGyroY() {
    sensors_event_t a, g, t;
    _mpu.getEvent(&a, &g, &t);
    return g.gyro.y - _gyro_bias_y;
}
