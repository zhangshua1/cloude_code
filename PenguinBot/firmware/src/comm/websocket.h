#pragma once
#include <Arduino.h>
#include <ArduinoJson.h>
#include <ESPAsyncWebServer.h>

class WebSocketServer {
public:
    static WebSocketServer& instance() {
        static WebSocketServer inst;
        return inst;
    }
    void begin();
    void broadcastTelemetry(const JsonDocument& doc);
    void broadcastEvent(const char* event, const JsonDocument& data);
    void onMessage(std::function<void(const char*)> handler) { _handler = handler; }

    bool hasClients() const;
    AsyncWebServer& server() { return _server; }

private:
    WebSocketServer() = default;
    AsyncWebServer _server{WS_PORT};
    AsyncWebSocket _ws{"/ws"};
    std::function<void(const char*)> _handler;

    void _onWsEvent(AsyncWebSocket* server, AsyncWebSocketClient* client,
                    AwsEventType type, void* arg, uint8_t* data, size_t len);
};
