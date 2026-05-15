#pragma once
#include <Arduino.h>
#include <ArduinoJson.h>
#include <HTTPClient.h>
#include "../audio/audio_capture.h"

// 对话角色
struct ChatMessage {
    String role;     // "system" | "user" | "assistant"
    String content;
};

// 对话上下文
struct Conversation {
    ChatMessage history[AI_MAX_TURNS];
    uint8_t count = 0;

    void add(const char* role, const char* content);
    void clear();
    String toJson(const char* system_prompt);
};

// AI 交互结果
struct AIResult {
    bool    ok = false;
    String  text;       // STT/LLM 返回的文字
    int     http_code = 0;
    int     latency_ms = 0;
    String  error;
};

// 音频合成结果
struct TTSResult {
    bool    ok = false;
    int16_t* audio = nullptr;  // PCM 16kHz 16bit mono
    size_t   len = 0;          // 采样数
    uint32_t duration_ms = 0;
    int      http_code = 0;
    String   error;
};

class AIService {
public:
    static AIService& instance() {
        static AIService inst;
        return inst;
    }
    void begin();

    // ---- STT ----
    AIResult speechToText(AudioBuffer& audio);

    // ---- LLM ----
    AIResult chat(const String& user_msg);
    void clearHistory();   // 清除对话历史

    // ---- TTS ----
    TTSResult textToSpeech(const String& text);
    // 直接播放 TTS 结果 (内部调用 I2S)
    void playTTS(const int16_t* data, size_t samples);

private:
    AIService() = default;
    Conversation _conv;
    String _stt_url, _llm_url, _tts_url;
    String _stt_key, _llm_key, _tts_key;

    void _buildUrls();
    AIResult _sttBaidu(AudioBuffer& audio);
    AIResult _sttOpenAI(AudioBuffer& audio);
    AIResult _llmDeepSeek(const String& msg);
    AIResult _llmOpenAI(const String& msg);
    TTSResult _ttsEdge(const String& text);
    TTSResult _ttsOpenAI(const String& text);
    TTSResult _ttsBaidu(const String& text);

    bool _loadConfig();
    void _saveConfig(const char* key, const char* value);

    // HTTP 辅助
    String _httpPost(const String& url, const String& body, const char* contentType, int& code, int& latency);
    String _httpPostStream(const String& url, const String& body, int& code, int& latency);  // SSE/stream
};
