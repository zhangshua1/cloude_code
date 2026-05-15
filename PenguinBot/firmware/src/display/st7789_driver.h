#pragma once
#include <Arduino.h>
#include <TFT_eSPI.h>

// 表情类型
enum class Expression {
    NEUTRAL, HAPPY, SAD, ANGRY, SURPRISED,
    WINK, LOVE, SLEEP, CONFUSED, EXCITED,
    COUNT
};

class DisplayDriver {
public:
    static DisplayDriver& instance() {
        static DisplayDriver inst;
        return inst;
    }
    void begin();
    void clear(uint16_t color = TFT_BLACK);

    // 表情绘制
    void drawExpression(Expression expr);

    // 信息叠加层
    void drawInfoOverlay(const char* ip, float battery_pct, int rssi);

    // 调试模式全屏信息
    void drawDebugScreen(float pitch, float gyro_y, int16_t l_pwm, int16_t r_pwm,
                         float bat_v, float bat_pct, int rssi, uint32_t uptime);

    TFT_eSPI& tft() { return _tft; }

private:
    DisplayDriver() = default;
    TFT_eSPI _tft;
    Expression _current = Expression::NEUTRAL;

    void _drawEyes(Expression expr);
    void _drawMouth(Expression expr);
    void _drawCheeks(Expression expr);
};
