#include "st7789_driver.h"
#include "../config.h"

void DisplayDriver::begin() {
    _tft.init();
    _tft.setRotation(0);   // 竖屏 240×320
    _tft.fillScreen(TFT_BLACK);
    _tft.setTextColor(TFT_WHITE, TFT_BLACK);
    _tft.setTextDatum(MC_DATUM);

    // 启动画面
    _tft.setTextFont(4);
    _tft.drawString("PenguinBot", 120, 140);
    _tft.setTextFont(2);
    _tft.drawString("v" FIRMWARE_VER, 120, 180);
    delay(1000);
}

void DisplayDriver::clear(uint16_t color) {
    _tft.fillScreen(color);
}

void DisplayDriver::drawExpression(Expression expr) {
    _current = expr;
    _tft.fillScreen(TFT_BLACK);
    _drawEyes(expr);
    _drawMouth(expr);
    _drawCheeks(expr);
}

void DisplayDriver::_drawEyes(Expression expr) {
    int ey_y = 100;
    int ey_lx = 80, ey_rx = 160;
    int ey_w = 36, ey_h = 40;

    switch (expr) {
    case Expression::NEUTRAL:
        // 圆形眼
        _tft.fillCircle(ey_lx, ey_y, 18, TFT_WHITE);
        _tft.fillCircle(ey_rx, ey_y, 18, TFT_WHITE);
        _tft.fillCircle(ey_lx, ey_y, 8, TFT_BLACK);
        _tft.fillCircle(ey_rx, ey_y, 8, TFT_BLACK);
        break;
    case Expression::HAPPY:
        // 弯弯笑眼 (^ ^)
        for (int i = 0; i < 3; i++) {
            _tft.drawArc(ey_lx, ey_y + 5, 18 - i, 18 - i, 225, 315, TFT_WHITE, TFT_BLACK);
            _tft.drawArc(ey_rx, ey_y + 5, 18 - i, 18 - i, 225, 315, TFT_WHITE, TFT_BLACK);
        }
        break;
    case Expression::SAD:
        _tft.fillCircle(ey_lx, ey_y, 18, TFT_WHITE);
        _tft.fillCircle(ey_rx, ey_y, 18, TFT_WHITE);
        _tft.fillCircle(ey_lx, ey_y + 5, 8, TFT_BLACK);
        _tft.fillCircle(ey_rx, ey_y + 5, 8, TFT_BLACK);
        break;
    case Expression::ANGRY:
        // V 形眉 + 眼
        _tft.fillRect(ey_lx - 18, ey_y - 28, 36, 4, TFT_RED);  // 眉毛
        _tft.fillRect(ey_rx - 18, ey_y - 28, 36, 4, TFT_RED);
        _tft.fillCircle(ey_lx, ey_y, 18, TFT_WHITE);
        _tft.fillCircle(ey_rx, ey_y, 18, TFT_WHITE);
        _tft.fillCircle(ey_lx, ey_y, 7, TFT_BLACK);
        _tft.fillCircle(ey_rx, ey_y, 7, TFT_BLACK);
        break;
    case Expression::SURPRISED:
        _tft.fillCircle(ey_lx, ey_y, 22, TFT_WHITE);
        _tft.fillCircle(ey_rx, ey_y, 22, TFT_WHITE);
        _tft.fillCircle(ey_lx, ey_y, 10, TFT_BLACK);
        _tft.fillCircle(ey_rx, ey_y, 10, TFT_BLACK);
        break;
    case Expression::WINK:
        _tft.fillCircle(ey_lx, ey_y, 18, TFT_WHITE);
        _tft.fillCircle(ey_lx, ey_y, 8, TFT_BLACK);
        // 右眼眯起
        for (int i = 0; i < 3; i++)
            _tft.drawArc(ey_rx, ey_y + 5, 18 - i, 18 - i, 225, 315, TFT_WHITE, TFT_BLACK);
        break;
    case Expression::LOVE:
        // 爱心眼
        _tft.fillCircle(ey_lx - 6, ey_y - 4, 12, TFT_RED);
        _tft.fillCircle(ey_lx + 6, ey_y - 4, 12, TFT_RED);
        _tft.fillTriangle(ey_lx - 17, ey_y, ey_lx + 17, ey_y, ey_lx, ey_y + 18, TFT_RED);
        _tft.fillCircle(ey_rx - 6, ey_y - 4, 12, TFT_RED);
        _tft.fillCircle(ey_rx + 6, ey_y - 4, 12, TFT_RED);
        _tft.fillTriangle(ey_rx - 17, ey_y, ey_rx + 17, ey_y, ey_rx, ey_y + 18, TFT_RED);
        break;
    case Expression::SLEEP:
        // = = 短线眼
        _tft.fillRect(ey_lx - 16, ey_y - 2, 32, 5, TFT_WHITE);
        _tft.fillRect(ey_rx - 16, ey_y - 2, 32, 5, TFT_WHITE);
        break;
    case Expression::CONFUSED:
        _tft.fillCircle(ey_lx, ey_y, 18, TFT_WHITE);
        _tft.fillCircle(ey_rx, ey_y, 18, TFT_WHITE);
        _tft.fillCircle(ey_lx + 6, ey_y, 8, TFT_BLACK);  // 斗鸡眼
        _tft.fillCircle(ey_rx - 6, ey_y, 8, TFT_BLACK);
        break;
    case Expression::EXCITED:
        // 星形简化
        _tft.fillCircle(ey_lx, ey_y, 20, TFT_YELLOW);
        _tft.fillCircle(ey_rx, ey_y, 20, TFT_YELLOW);
        _tft.fillCircle(ey_lx, ey_y, 7, TFT_BLACK);
        _tft.fillCircle(ey_rx, ey_y, 7, TFT_BLACK);
        break;
    default: break;
    }
}

void DisplayDriver::_drawMouth(Expression expr) {
    int m_y = 160;
    int m_x = 120;

    switch (expr) {
    case Expression::HAPPY:
    case Expression::EXCITED:
        _tft.drawArc(m_x, m_y - 10, 25, 25, 30, 150, TFT_WHITE, TFT_BLACK);  // 笑脸弧
        break;
    case Expression::SAD:
        _tft.drawArc(m_x, m_y + 15, 25, 25, 210, 330, TFT_WHITE, TFT_BLACK); // 哭脸弧
        break;
    case Expression::ANGRY:
        _tft.fillRect(m_x - 20, m_y, 40, 5, TFT_WHITE);  // 直线嘴
        break;
    case Expression::SURPRISED:
        _tft.fillCircle(m_x, m_y, 12, TFT_WHITE);  // O嘴
        _tft.fillCircle(m_x, m_y, 6, TFT_BLACK);
        break;
    case Expression::LOVE:
        _tft.fillCircle(m_x - 5, m_y - 4, 8, TFT_RED);
        _tft.fillCircle(m_x + 5, m_y - 4, 8, TFT_RED);
        _tft.fillTriangle(m_x - 12, m_y, m_x + 12, m_y, m_x, m_y + 12, TFT_RED);
        break;
    case Expression::SLEEP:
        _tft.fillCircle(m_x, m_y, 6, TFT_WHITE);  // o嘴
        break;
    case Expression::WINK:
        _tft.drawArc(m_x, m_y - 5, 20, 20, 30, 150, TFT_WHITE, TFT_BLACK);
        break;
    case Expression::CONFUSED:
        _tft.fillRect(m_x - 8, m_y + 5, 16, 4, TFT_WHITE);  // ~ 波浪
        break;
    default:
        _tft.fillRect(m_x - 12, m_y, 24, 3, TFT_WHITE);  // 中性嘴
        break;
    }
}

void DisplayDriver::_drawCheeks(Expression expr) {
    if (expr == Expression::HAPPY || expr == Expression::EXCITED ||
        expr == Expression::LOVE) {
        _tft.fillCircle(50, 150, 10, TFT_PINK);
        _tft.fillCircle(190, 150, 10, TFT_PINK);
    }
}

// --- 信息叠加 (顶部状态栏) ---
void DisplayDriver::drawInfoOverlay(const char* ip, float battery_pct, int rssi) {
    _tft.setTextColor(TFT_WHITE, TFT_BLACK);
    _tft.setTextFont(1);
    _tft.setTextDatum(TL_DATUM);

    char buf[32];
    snprintf(buf, sizeof(buf), "%.0f%%", battery_pct);
    _tft.drawString(buf, 4, 4);

    if (ip) {
        snprintf(buf, sizeof(buf), "%s", ip);
        _tft.drawString(buf, 80, 4);
    }

    snprintf(buf, sizeof(buf), "%ddBm", rssi);
    _tft.drawString(200, 4, buf);
}

void DisplayDriver::drawDebugScreen(float pitch, float gyro_y, int16_t l_pwm,
                                     int16_t r_pwm, float bat_v, float bat_pct,
                                     int rssi, uint32_t uptime) {
    _tft.fillScreen(TFT_BLACK);
    _tft.setTextColor(TFT_GREEN, TFT_BLACK);
    _tft.setTextFont(2);
    _tft.setTextDatum(TL_DATUM);

    _tft.drawString("DEBUG MODE", 4, 4);
    _tft.drawLine(0, 20, 240, 20, TFT_DARKGREY);

    int y = 25, dy = 20;
    char buf[40];
    snprintf(buf, sizeof(buf), "Pitch: %.2f deg", pitch); _tft.drawString(buf, 4, y); y += dy;
    snprintf(buf, sizeof(buf), "GyroY: %.2f d/s", gyro_y); _tft.drawString(buf, 4, y); y += dy;
    snprintf(buf, sizeof(buf), "L_PWM: %d  R_PWM: %d", l_pwm, r_pwm); _tft.drawString(buf, 4, y); y += dy;
    snprintf(buf, sizeof(buf), "Batt : %.2fV (%.0f%%)", bat_v, bat_pct); _tft.drawString(buf, 4, y); y += dy;
    snprintf(buf, sizeof(buf), "RSSI : %d dBm", rssi); _tft.drawString(buf, 4, y); y += dy;

    uint32_t s = uptime % 60; uint32_t m = (uptime / 60) % 60;
    uint32_t h = uptime / 3600;
    snprintf(buf, sizeof(buf), "Up   : %02lu:%02lu:%02lu", h, m, s);
    _tft.drawString(buf, 4, y); y += dy;

    snprintf(buf, sizeof(buf), "Heap : %u", ESP.getFreeHeap());
    _tft.drawString(buf, 4, y);
}
