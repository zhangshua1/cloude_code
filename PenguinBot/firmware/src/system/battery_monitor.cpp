#include "battery_monitor.h"

// ===== BatteryMonitor =====

void BatteryMonitor::begin() {
    analogReadResolution(12);
    pinMode(PIN_BAT_ADC, INPUT);
    // 采样初始化
    for (int i = 0; i < BAT_SAMPLE_COUNT; i++) {
        _samples[i] = 0;
    }
}

float BatteryMonitor::getVoltage() {
    // 滑动平均
    float raw = analogRead(PIN_BAT_ADC) * (3.3f / 4095.0f);
    _samples[_idx] = raw * (BAT_R1 + BAT_R2) / BAT_R2;
    _idx = (_idx + 1) % BAT_SAMPLE_COUNT;

    float sum = 0;
    for (auto s : _samples) sum += s;
    _voltage = sum / BAT_SAMPLE_COUNT;
    return _voltage;
}

uint8_t BatteryMonitor::getPercentage() {
    if (_voltage >= BAT_FULL_V) return 100;
    if (_voltage <= BAT_EMPTY_V) return 0;
    return (uint8_t)((_voltage - BAT_EMPTY_V) / (BAT_FULL_V - BAT_EMPTY_V) * 100);
}

bool BatteryMonitor::isLow() { return _voltage < BAT_LOW_WARN_V; }
bool BatteryMonitor::isCritical() { return _voltage < BAT_CRITICAL_V; }

// ===== Logger (环形缓冲) =====

void Logger::begin() {
    memset(_buf, 0, sizeof(_buf));
}

void Logger::info(const char* fmt, ...) {
    va_list args;
    va_start(args, fmt);
    _log("INFO", fmt, args);
    va_end(args);
}

void Logger::warn(const char* fmt, ...) {
    va_list args;
    va_start(args, fmt);
    _log("WARN", fmt, args);
    va_end(args);
}

void Logger::error(const char* fmt, ...) {
    va_list args;
    va_start(args, fmt);
    _log("ERROR", fmt, args);
    va_end(args);
}

void Logger::_log(const char* level, const char* fmt, va_list args) {
    char entry[128];
    int len = snprintf(entry, sizeof(entry), "[%s] ", level);
    vsnprintf(entry + len, sizeof(entry) - len - 2, fmt, args);
    size_t entry_len = strlen(entry);

    // 环形写入
    if (_pos + entry_len + 2 >= LOG_BUF_SIZE) {
        // 缓冲区满，从头部重新开始
        _pos = 0;
    }
    memcpy(_buf + _pos, entry, entry_len);
    _pos += entry_len;
    _buf[_pos++] = '\n';
    _buf[_pos] = '\0';

    // 同时输出到串口
    Serial.println(entry);
}

void Logger::clear() {
    memset(_buf, 0, sizeof(_buf));
    _pos = 0;
}
