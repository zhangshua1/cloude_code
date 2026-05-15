#include "display_task.h"
#include "../balance/balance_task.h"
#include "../system/battery_monitor.h"
#include <WiFi.h>

void DisplayTask::begin() {
    DisplayDriver::instance().begin();
}

void DisplayTask::run() {
    unsigned long now = millis();
    if (now - _last_run < 33) return;  // ~30Hz
    _last_run = now;

    auto& disp = DisplayDriver::instance();

    if (_mode == DisplayMode::EXPRESSION && _dirty) {
        _dirty = false;
        disp.drawExpression(_expr);

        // 叠加状态栏
        char ip[16] = "PenguinBot";
        if (WiFi.status() == WL_CONNECTED) {
            snprintf(ip, sizeof(ip), "%s", WiFi.localIP().toString().c_str());
        }
        disp.drawInfoOverlay(ip,
                             BatteryMonitor::instance().getPercentage(),
                             WiFi.RSSI());

    } else if (_mode == DisplayMode::DEBUG) {
        auto ts = BalanceTask::instance().getTelemetry();
        disp.drawDebugScreen(ts.angle_deg, ts.angular_velocity,
                             ts.motor_l_pwm, ts.motor_r_pwm,
                             ts.battery_v, ts.battery_pct,
                             ts.wifi_rssi, ts.uptime_s);
    }
}
