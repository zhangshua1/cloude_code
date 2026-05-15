#pragma once
#include <Arduino.h>
#include <ArduinoJson.h>
#include "websocket.h"
#include "../balance/balance_task.h"
#include "../servo/animation.h"
#include "../display/display_task.h"

class ProtocolHandler {
public:
    static ProtocolHandler& instance() {
        static ProtocolHandler inst;
        return inst;
    }
    void begin();
    void handleMessage(const char* msg);  // 从 WebSocket 回调

    // 构建遥测 JSON
    static void buildTelemetry(JsonDocument& doc);

private:
    ProtocolHandler() = default;

    void _cmdMove(const JsonDocument& params);
    void _cmdExpression(const JsonDocument& params);
    void _cmdServo(const JsonDocument& params);
    void _cmdServoSequence(const JsonDocument& params);
    void _cmdPidSet(const JsonDocument& params);
    void _cmdPidSave(const JsonDocument& params);
    void _cmdDisplayMode(const JsonDocument& params);
    void _cmdAnimation(const JsonDocument& params);
    void _cmdChat(const JsonDocument& params);
    void _cmdChatClear(const JsonDocument& params);
    void _cmdAIConfig(const JsonDocument& params);
};
