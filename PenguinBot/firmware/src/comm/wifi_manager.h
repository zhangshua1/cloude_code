#pragma once
#include <Arduino.h>
#include <WiFi.h>

class WiFiManager {
public:
    static WiFiManager& instance() {
        static WiFiManager inst;
        return inst;
    }
    void begin();  // 启动 AP 模式
    void connectSTA(const char* ssid, const char* pass);  // 切换到 STA

    String getIP() const;
    int    getRSSI() const { return WiFi.RSSI(); }

private:
    WiFiManager() = default;
    String _ap_ssid;
};
