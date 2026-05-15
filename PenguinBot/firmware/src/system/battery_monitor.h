#pragma once
#include <Arduino.h>
#include "../config.h"

class BatteryMonitor {
public:
    static BatteryMonitor& instance() {
        static BatteryMonitor inst;
        return inst;
    }
    void begin();
    float getVoltage();       // 电池电压 (V)
    uint8_t getPercentage();  // 电量百分比 (0-100)
    bool   isLow();           // 低电量告警
    bool   isCritical();      // 欠压保护

private:
    BatteryMonitor() = default;
    float _voltage = 0;
    float _samples[BAT_SAMPLE_COUNT] = {};
    uint8_t _idx = 0;
};

class Logger {
public:
    static Logger& instance() {
        static Logger inst;
        return inst;
    }
    void begin();
    void info(const char* fmt, ...);
    void warn(const char* fmt, ...);
    void error(const char* fmt, ...);
    const char* getEntries() const { return _buf; }
    void clear();

private:
    Logger() = default;
    char _buf[LOG_BUF_SIZE] = {};
    uint16_t _pos = 0;
    void _log(const char* level, const char* fmt, va_list args);
};
