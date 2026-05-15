#include "conversation.h"
#include "../config.h"
#include "../display/st7789_driver.h"

bool ConversationTask::begin() {
    // 分配录音缓冲区 (PSRAM 优先)
    size_t buf_bytes = AUDIO_BUFFER_SAMPLES * sizeof(int16_t);
    _audio_buf.data = (int16_t*)heap_caps_malloc(buf_bytes,
                        MALLOC_CAP_SPIRAM | MALLOC_CAP_8BIT);
    if (!_audio_buf.data) {
        _audio_buf.data = (int16_t*)malloc(buf_bytes);
    }
    if (!_audio_buf.data) {
        log_e("Failed to allocate audio buffer (%u bytes)", buf_bytes);
        return false;
    }
    _allocated = true;
    _setState(ConvState::IDLE);
    return true;
}

void ConversationTask::trigger() {
    if (_state == ConvState::IDLE) {
        _setState(ConvState::LISTENING);
    }
}

void ConversationTask::triggerText(const char* text) {
    if (_state != ConvState::IDLE) return;
    // 跳过 STT，直接进 LLM
    _setState(ConvState::THINKING_LLM);
    strncpy(_last_reply, text, sizeof(_last_reply) - 1);
}

void ConversationTask::run() {
    switch (_state) {
    case ConvState::IDLE:         _handleIdle(); break;
    case ConvState::LISTENING:    _handleListening(); break;
    case ConvState::THINKING_STT: _handleSttThinking(); break;
    case ConvState::THINKING_LLM: _handleLlmThinking(); break;
    case ConvState::SPEAKING:     break;  // TTS 由 _handleSpeaking 驱动
    case ConvState::REPLY_DISPLAY: {
        // 3 秒后自动回到 IDLE
        if (millis() - _state_entered_ms > 3000) _setState(ConvState::IDLE);
        break;
    }
    case ConvState::ERROR: {
        if (millis() - _state_entered_ms > 2000) _setState(ConvState::IDLE);
        break;
    }
    }
}

void ConversationTask::_setState(ConvState s) {
    log_i("ConvState: %d → %d", (int)_state, (int)s);
    _state = s;
    _state_entered_ms = millis();
    _expressForState();
}

void ConversationTask::_cleanupAudio() {
    if (_audio_buf.data) {
        free(_audio_buf.data);
        _audio_buf.data = nullptr;
        _allocated = false;
    }
}

// ===== State Handlers =====

void ConversationTask::_handleIdle() {
    // 检查按键
    if (AI_TRIGGER_BUTTON) {
        static unsigned long last_press = 0;
        if (!digitalRead(PIN_WAKE_BTN) && millis() - last_press > 1000) {
            last_press = millis();
            trigger();
        }
    }
}

void ConversationTask::_handleListening() {
    if (!_allocated) {
        _setState(ConvState::ERROR);
        return;
    }
    // LED 指示录音
    digitalWrite(PIN_NEOPIXEL, HIGH);

    // 录音
    AudioCapture::instance().record(_audio_buf, AUDIO_RECORD_SECS * 1000);
    digitalWrite(PIN_NEOPIXEL, LOW);

    if (_audio_buf.len < 800) {
        _setState(ConvState::ERROR);
        _last_reply[0] = '\0';
        return;
    }
    _setState(ConvState::THINKING_STT);
}

void ConversationTask::_handleSttThinking() {
    DisplayTask::instance().setExpression(Expression::CONFUSED);

    AIResult r = AIService::instance().speechToText(_audio_buf);
    if (!r.ok || r.text.length() == 0) {
        _setState(ConvState::ERROR);
        snprintf(_last_reply, sizeof(_last_reply), "%s", r.error.c_str());
        log_e("STT failed: %s (%dms)", r.error.c_str(), r.latency_ms);
        return;
    }

    log_i("STT: \"%s\" (%dms)", r.text.c_str(), r.latency_ms);

    // 进 LLM
    strncpy(_last_reply, r.text.c_str(), sizeof(_last_reply) - 1);
    _setState(ConvState::THINKING_LLM);
}

void ConversationTask::_handleLlmThinking() {
    DisplayTask::instance().setExpression(Expression::CONFUSED);

    AIResult r = AIService::instance().chat(_last_reply);
    if (!r.ok || r.text.length() == 0) {
        _setState(ConvState::ERROR);
        snprintf(_last_reply, sizeof(_last_reply), "AI: %s", r.error.c_str());
        return;
    }

    log_i("LLM: \"%s\" (%dms)", r.text.c_str(), r.latency_ms);
    strncpy(_last_reply, r.text.c_str(), sizeof(_last_reply) - 1);

    // TTS 播放
    _handleSpeaking(r);
}

void ConversationTask::_handleSpeaking(AIResult& stt_r, AIResult* llm_r) {
    const char* text = llm_r ? llm_r->text.c_str() : stt_r.text.c_str();

    // 表情联动
    if (strstr(text, "哈哈") || strstr(text, "笑"))
        DisplayTask::instance().setExpression(Expression::HAPPY);
    else if (strstr(text, "抱歉") || strstr(text, "对不起"))
        DisplayTask::instance().setExpression(Expression::SAD);
    else if (strstr(text, "哇") || strstr(text, "太棒") || strstr(text, "!"))
        DisplayTask::instance().setExpression(Expression::EXCITED);
    else if (strstr(text, "爱"))
        DisplayTask::instance().setExpression(Expression::LOVE);
    else
        DisplayTask::instance().setExpression(Expression::HAPPY);

    TTSResult tts = AIService::instance().textToSpeech(text);
    if (tts.ok && tts.audio) {
        AIService::instance().playTTS(tts.audio, tts.len);
        free(tts.audio);
    }

    // 显示回复
    auto& display = DisplayDriver::instance();
    display.clear(TFT_BLACK);
    display.tft().setTextColor(TFT_WHITE, TFT_BLACK);
    display.tft().setTextFont(2);
    display.tft().setTextDatum(MC_DATUM);
    // 文字换行
    String wrapped = text;
    display.tft().drawString(wrapped, 120, 160);

    _setState(ConvState::REPLY_DISPLAY);
}

void ConversationTask::_handleError(const char* err) {
    log_e("Conv error: %s", err);
    DisplayTask::instance().setExpression(Expression::SAD);
    _setState(ConvState::REPLY_DISPLAY);
}

void ConversationTask::_expressForState() {
    switch (_state) {
    case ConvState::IDLE:
        DisplayTask::instance().setExpression(Expression::NEUTRAL);
        AnimationEngine::instance().playPreset("idle");
        break;
    case ConvState::LISTENING:
        DisplayTask::instance().setExpression(Expression::SURPRISED);
        break;
    case ConvState::THINKING_STT:
    case ConvState::THINKING_LLM:
        DisplayTask::instance().setExpression(Expression::CONFUSED);
        break;
    case ConvState::SPEAKING:
        AnimationEngine::instance().playPreset("wave");
        break;
    case ConvState::ERROR:
        DisplayTask::instance().setExpression(Expression::SAD);
        break;
    default: break;
    }
}
