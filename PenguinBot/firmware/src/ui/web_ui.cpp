#include "web_ui.h"
#include "../comm/websocket.h"

void WebUI::begin() {
    if (!SPIFFS.begin(true)) {
        log_e("SPIFFS mount failed");
        return;
    }

    auto& server = WebSocketServer::instance().server();

    // SPIFFS 静态文件
    server.serveStatic("/", SPIFFS, "/").setDefaultFile("index.html");

    // 日志 API
    server.on("/api/log", HTTP_GET, [](AsyncWebServerRequest* req) {
        extern const char* getLogBuffer();  // from logger
        req->send(200, "text/plain", Logger::instance().getEntries());
    });

    // 重启
    server.on("/api/restart", HTTP_POST, [](AsyncWebServerRequest* req) {
        req->send(200, "application/json", "{\"ok\":true}");
        delay(100);
        ESP.restart();
    });

    log_i("WebUI ready: http://%s/", WiFiManager::instance().getIP().c_str());
}
