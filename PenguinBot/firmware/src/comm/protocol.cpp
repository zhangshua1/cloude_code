#include "protocol.h"
#include "../config.h"
#include "../ai/conversation.h"
#include "../display/display_task.h"
#include <Preferences.h>

void ProtocolHandler::begin() {
    WebSocketServer::instance().onMessage([](const char* msg) {
        instance().handleMessage(msg);
    });
}

void ProtocolHandler::handleMessage(const char* msg) {
    JsonDocument doc(512);
    DeserializationError err = deserializeJson(doc, msg);
    if (err) {
        log_w("JSON parse error: %s", err.c_str());
        return;
    }

    const char* cmd = doc["cmd"];
    if (!cmd) return;

    JsonVariant params = doc["params"];

    if (strcmp(cmd, "move") == 0)          _cmdMove(params);
    else if (strcmp(cmd, "stop") == 0)     BalanceTask::instance().stop();
    else if (strcmp(cmd, "expression") == 0) _cmdExpression(params);
    else if (strcmp(cmd, "servo") == 0)    _cmdServo(params);
    else if (strcmp(cmd, "servo_sequence") == 0) _cmdServoSequence(params);
    else if (strcmp(cmd, "pid_set") == 0)  _cmdPidSet(params);
    else if (strcmp(cmd, "pid_get") == 0) {
        JsonDocument rsp(256);
        buildTelemetry(rsp);
        WebSocketServer::instance().broadcastTelemetry(rsp);
    }
    else if (strcmp(cmd, "pid_save") == 0) _cmdPidSave(params);
    else if (strcmp(cmd, "display_mode") == 0) _cmdDisplayMode(params);
    else if (strcmp(cmd, "animation") == 0) _cmdAnimation(params);
    else if (strcmp(cmd, "chat") == 0)     _cmdChat(params);
    else if (strcmp(cmd, "chat_clear") == 0) _cmdChatClear(params);
    else if (strcmp(cmd, "ai_config") == 0)  _cmdAIConfig(params);
    else {
        log_w("Unknown command: %s", cmd);
    }
}

void ProtocolHandler::_cmdMove(const JsonDocument& params) {
    const char* dir = params["direction"] | "forward";
    float speed = params["speed"] | 80.0f;
    float turn = params["turn"] | 0.0f;
    unsigned long dur = params["duration_ms"] | 0;

    if (strcmp(dir, "backward") == 0) speed = -speed;
    if (strcmp(dir, "left") == 0)     turn = -80;
    if (strcmp(dir, "right") == 0)    turn = 80;

    BalanceTask::instance().move(speed, turn, dur);
}

void ProtocolHandler::_cmdExpression(const JsonDocument& params) {
    const char* type = params["type"] | "neutral";
    static const struct { const char* n; Expression e; } map[] = {
        {"neutral", Expression::NEUTRAL}, {"happy", Expression::HAPPY},
        {"sad", Expression::SAD}, {"angry", Expression::ANGRY},
        {"surprised", Expression::SURPRISED}, {"wink", Expression::WINK},
        {"love", Expression::LOVE}, {"sleep", Expression::SLEEP},
        {"confused", Expression::CONFUSED}, {"excited", Expression::EXCITED},
    };
    for (auto& m : map) {
        if (strcmp(type, m.n) == 0) {
            DisplayTask::instance().setExpression(m.e);
            return;
        }
    }
}

void ProtocolHandler::_cmdServo(const JsonDocument& params) {
    int ch = params["channel"] | -1;
    float angle = params["angle"] | 90.0f;
    int speed_ms = params["speed"] | 200;
    if (ch >= 0 && ch < 5) {
        PCA9685Driver::instance().setAngleSlow(ch, angle, speed_ms);
    }
}

void ProtocolHandler::_cmdServoSequence(const JsonDocument& params) {
    // 简化处理：取第一个关键帧
    JsonArray kfs = params["keyframes"];
    if (kfs.size() > 0) {
        auto& kf = kfs[0];
        for (int i = 0; i < 5; i++) {
            if (kf.containsKey(String(i).c_str())) {
                PCA9685Driver::instance().setAngleSlow(i, kf[i], 200);
            }
        }
    }
}

void ProtocolHandler::_cmdPidSet(const JsonDocument& params) {
    const char* loop = params["loop"] | "angle";
    PIDParams p;
    p.kp = params["kp"] | p.kp;
    p.ki = params["ki"] | p.ki;
    p.kd = params["kd"] | p.kd;

    if (strcmp(loop, "angle") == 0)
        BalanceTask::instance().setAnglePid(p);
    else
        BalanceTask::instance().setRatePid(p);

    log_i("PID %s set: kp=%.2f ki=%.2f kd=%.2f", loop, p.kp, p.ki, p.kd);
}

void ProtocolHandler::_cmdPidSave(const JsonDocument& params) {
    Preferences prefs;
    prefs.begin(NVS_NS, false);
    auto ts = BalanceTask::instance().getTelemetry();
    prefs.putFloat(NVS_KEY_ANGLE_KP, ts.pid_angle_kp);
    prefs.putFloat(NVS_KEY_ANGLE_KI, ts.pid_angle_ki);
    prefs.putFloat(NVS_KEY_ANGLE_KD, ts.pid_angle_kd);
    prefs.putFloat(NVS_KEY_RATE_KP,  ts.pid_rate_kp);
    prefs.putFloat(NVS_KEY_RATE_KI,  ts.pid_rate_ki);
    prefs.putFloat(NVS_KEY_RATE_KD,  ts.pid_rate_kd);
    prefs.end();
    log_i("PID saved to NVS");
}

void ProtocolHandler::_cmdDisplayMode(const JsonDocument& params) {
    const char* mode = params["mode"] | "expression";
    if (strcmp(mode, "debug") == 0)
        DisplayTask::instance().setMode(DisplayMode::DEBUG);
    else
        DisplayTask::instance().setMode(DisplayMode::EXPRESSION);
}

void ProtocolHandler::_cmdAnimation(const JsonDocument& params) {
    const char* name = params["name"] | "wave";
    AnimationEngine::instance().playPreset(name);
}

// ---- AI 对话 ----

void ProtocolHandler::_cmdChat(const JsonDocument& params) {
    const char* text = params["text"];
    if (!text || strlen(text) == 0) return;

    ConversationTask::instance().triggerText(text);
}

void ProtocolHandler::_cmdChatClear(const JsonDocument& params) {
    ConversationTask::instance().clearHistory();
}

void ProtocolHandler::_cmdAIConfig(const JsonDocument& params) {
    const char* key = params["key"];
    const char* value = params["value"];
    if (!key || !value) return;

    Preferences prefs;
    prefs.begin(NVS_NS, false);
    if (strcmp(key, "stt_key") == 0)
        prefs.putString(NVS_KEY_STT_KEY, value);
    else if (strcmp(key, "llm_key") == 0)
        prefs.putString(NVS_KEY_LLM_KEY, value);
    else if (strcmp(key, "tts_key") == 0)
        prefs.putString(NVS_KEY_TTS_KEY, value);
    prefs.end();
    log_i("AI config saved: %s", key);
}

// ---- buildTelemetry (保持原样) ----

void ProtocolHandler::buildTelemetry(JsonDocument& doc) {
    auto ts = BalanceTask::instance().getTelemetry();

    doc["type"] = "telemetry";
    doc["timestamp_ms"] = millis();
    JsonObject data = doc["data"].to<JsonObject>();
    data["angle"] = ts.angle_deg;
    data["angular_velocity"] = ts.angular_velocity;
    data["angle_target"] = ts.angle_target;
    data["motor_l_pwm"] = ts.motor_l_pwm;
    data["motor_r_pwm"] = ts.motor_r_pwm;
    data["motor_l_rpm"] = ts.motor_l_rpm;
    data["motor_r_rpm"] = ts.motor_r_rpm;
    data["battery_v"] = ts.battery_v;
    data["battery_pct"] = ts.battery_pct;
    data["expression"] = (int)DisplayTask::instance().getExpression();
    data["uptime_s"] = ts.uptime_s;
    data["wifi_rssi"] = ts.wifi_rssi;
    data["free_heap"] = ts.free_heap;
    data["cpu_temp_c"] = ts.cpu_temp_c;
    data["conv_state"] = (int)ConversationTask::instance().state();
    data["last_reply"] = ConversationTask::instance().lastReply();

    JsonObject pid_a = data["pid_angle"].to<JsonObject>();
    pid_a["kp"] = ts.pid_angle_kp; pid_a["ki"] = ts.pid_angle_ki; pid_a["kd"] = ts.pid_angle_kd;
    JsonObject pid_r = data["pid_rate"].to<JsonObject>();
    pid_r["kp"] = ts.pid_rate_kp; pid_r["ki"] = ts.pid_rate_ki; pid_r["kd"] = ts.pid_rate_kd;

    JsonArray sv = data["servo_positions"].to<JsonArray>();
    for (int i = 0; i < 5; i++)
        sv.add(PCA9685Driver::instance().getAngle(i));
}
