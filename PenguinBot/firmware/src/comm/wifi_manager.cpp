#include "wifi_manager.h"
#include "../config.h"

void WiFiManager::begin() {
    // 生成唯一 SSID
    uint64_t chipid = ESP.getEfuseMac();
    char ssid[32];
    snprintf(ssid, sizeof(ssid), "%s%04X", WIFI_SSID_PREFIX, (uint16_t)(chipid & 0xFFFF));
    _ap_ssid = ssid;

    WiFi.mode(WIFI_AP);
    WiFi.softAP(ssid, WIFI_PASSWORD);

    log_i("WiFi AP: %s (password: %s)", ssid, WIFI_PASSWORD);
    log_i("IP: %s", WiFi.softAPIP().toString().c_str());
}

void WiFiManager::connectSTA(const char* ssid, const char* pass) {
    WiFi.mode(WIFI_AP_STA);
    WiFi.begin(ssid, pass);
    // 非阻塞，让 comm_task 轮询状态
}

String WiFiManager::getIP() const {
    if (WiFi.status() == WL_CONNECTED) {
        return WiFi.localIP().toString();
    }
    return WiFi.softAPIP().toString();
}
