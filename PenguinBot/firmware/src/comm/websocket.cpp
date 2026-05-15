#include "websocket.h"

void WebSocketServer::begin() {
    _ws.onEvent([this](AsyncWebSocket* s, AsyncWebSocketClient* c,
                       AwsEventType t, void* arg, uint8_t* d, size_t l) {
        _onWsEvent(s, c, t, arg, d, l);
    });
    _server.addHandler(&_ws);

    // 简易 HTTP API
    _server.on("/api/status", HTTP_GET, [](AsyncWebServerRequest* req) {
        req->send(200, "application/json", "{\"status\":\"ok\"}");
    });

    _server.begin();
    log_i("WebSocket server started on port %d", WS_PORT);
}

void WebSocketServer::broadcastTelemetry(const JsonDocument& doc) {
    if (_ws.count() == 0) return;
    String out;
    serializeJson(doc, out);
    _ws.textAll(out);
}

void WebSocketServer::broadcastEvent(const char* event, const JsonDocument& data) {
    JsonDocument evt(256);
    evt["type"] = "event";
    evt["event"] = event;
    evt["data"] = data;
    String out;
    serializeJson(evt, out);
    _ws.textAll(out);
}

bool WebSocketServer::hasClients() const { return _ws.count() > 0; }

void WebSocketServer::_onWsEvent(AsyncWebSocket* server, AsyncWebSocketClient* client,
                                  AwsEventType type, void* arg, uint8_t* data, size_t len) {
    if (type == WS_EVT_CONNECT) {
        log_i("WS client connected (#%u)", client->id());
    } else if (type == WS_EVT_DISCONNECT) {
        log_i("WS client disconnected");
    } else if (type == WS_EVT_DATA) {
        String msg((char*)data, len);
        if (_handler) _handler(msg.c_str());
    }
}
