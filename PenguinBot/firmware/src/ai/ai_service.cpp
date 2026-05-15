#include "ai_service.h"
#include "../config.h"
#include <Preferences.h>
#include <base64.h>
#include <esp_timer.h>

// ===== Conversation =====
void Conversation::add(const char* role, const char* content) {
    if (count >= AI_MAX_TURNS) {
        // 滑动窗口：丢弃最早的非 system 轮次
        uint8_t shift = (history[0].role == "system") ? 1 : 0;
        for (uint8_t i = shift; i < count - 1; i++)
            history[i] = history[i + 1];
        count--;
    }
    history[count].role = role;
    history[count].content = content;
    count++;
}

void Conversation::clear() {
    count = 0;
}

String Conversation::toJson(const char* system_prompt) {
    JsonDocument doc(8192);
    JsonArray msgs = doc["messages"].to<JsonArray>();

    // System prompt
    JsonObject sys = msgs.add<JsonObject>();
    sys["role"] = "system";
    sys["content"] = system_prompt;

    for (uint8_t i = 0; i < count; i++) {
        JsonObject m = msgs.add<JsonObject>();
        m["role"] = history[i].role;
        m["content"] = history[i].content;
    }

    String out;
    serializeJson(doc, out);
    return out;
}

// ===== AIService =====

void AIService::begin() {
    _loadConfig();
    _buildUrls();
    _conv.add("system", AI_SYSTEM_PROMPT);
}

void AIService::clearHistory() {
    _conv.clear();
    _conv.add("system", AI_SYSTEM_PROMPT);
}

bool AIService::_loadConfig() {
    Preferences p;
    p.begin(NVS_NS, true);
    _stt_key = p.getString(NVS_KEY_STT_KEY, AI_STT_API_KEY);
    _llm_key = p.getString(NVS_KEY_LLM_KEY, AI_LLM_API_KEY);
    _tts_key = p.getString(NVS_KEY_TTS_KEY, AI_TTS_API_KEY);
    p.end();
    return true;
}

void AIService::_saveConfig(const char* key, const char* value) {
    Preferences p;
    p.begin(NVS_NS, false);
    p.putString(key, value);
    p.end();
}

void AIService::_buildUrls() {
    // STT
    if (strlen(AI_STT_URL) > 5) _stt_url = AI_STT_URL;
    else if (strcmp(AI_STT_PROVIDER, "openai") == 0)
        _stt_url = "https://api.openai.com/v1/audio/transcriptions";
    else // baidu
        _stt_url = "https://vop.baidu.com/server_api";

    // LLM
    if (strlen(AI_LLM_URL) > 5) _llm_url = AI_LLM_URL;
    else if (strcmp(AI_LLM_PROVIDER, "deepseek") == 0)
        _llm_url = "https://api.deepseek.com/v1/chat/completions";
    else if (strcmp(AI_LLM_PROVIDER, "openai") == 0)
        _llm_url = "https://api.openai.com/v1/chat/completions";
    else // custom/default
        _llm_url = "https://api.deepseek.com/v1/chat/completions";

    // TTS
    if (strlen(AI_TTS_URL) > 5) _tts_url = AI_TTS_URL;
    else if (strcmp(AI_TTS_PROVIDER, "openai") == 0)
        _tts_url = "https://api.openai.com/v1/audio/speech";
    else if (strcmp(AI_TTS_PROVIDER, "baidu") == 0)
        _tts_url = "https://tsn.baidu.com/text2audio";
    else // edge (free, browser-like)
        _tts_url = "https://speech.platform.bing.com/consumer/speech/synthesize/readaloud/edge/v1";
}

// ===== STT =====

AIResult AIService::speechToText(AudioBuffer& audio) {
    auto start = millis();
    AIResult r;
    if (audio.len < 800) {  // 最小 50ms
        r.error = "audio too short";
        return r;
    }
    if (strcmp(AI_STT_PROVIDER, "openai") == 0) r = _sttOpenAI(audio);
    else r = _sttBaidu(audio);
    r.latency_ms = millis() - start;
    return r;
}

AIResult AIService::_sttBaidu(AudioBuffer& audio) {
    AIResult r;
    // 百度 ASR 需要 PCM → base64 编码
    size_t b64len = base64_enc_len(audio.len * sizeof(int16_t));
    char* b64 = (char*)malloc(b64len + 1);
    if (!b64) { r.error = "OOM base64"; return r; }
    base64_encode(b64, (char*)audio.data, audio.len * sizeof(int16_t));

    JsonDocument req(4096);
    req["format"] = "pcm";
    req["rate"] = AUDIO_SAMPLE_RATE;
    req["channel"] = 1;
    req["cuid"] = "penguinbot";
    req["token"] = _stt_key;
    req["speech"] = b64;
    req["len"] = audio.len * sizeof(int16_t);

    String body; serializeJson(req, body);
    free(b64);

    int code = 0, lat = 0;
    String resp = _httpPost(_stt_url, body, "application/json", code, lat);

    if (code != 200) {
        r.http_code = code;
        r.error = "STT HTTP " + String(code);
        return r;
    }

    JsonDocument doc(2048);
    deserializeJson(doc, resp);
    if (doc["err_no"] != 0) {
        r.error = doc["err_msg"] | "STT error";
        return r;
    }
    const char* text = doc["result"][0];
    r.ok = true;
    r.text = text ? text : "";
    return r;
}

AIResult AIService::_sttOpenAI(AudioBuffer& audio) {
    AIResult r;
    // OpenAI Whisper: multipart/form-data
    // ESP32 下用简化方式：WAV header + raw PCM
    String boundary = "----PenguinBotBoundary";
    String body;

    // WAV header
    uint8_t wav[44] = {};
    memcpy(wav, "RIFF", 4);
    uint32_t dataSize = audio.len * 2;
    *(uint32_t*)(wav + 4) = dataSize + 36;
    memcpy(wav + 8, "WAVE", 4);
    memcpy(wav + 12, "fmt ", 4);
    *(uint32_t*)(wav + 16) = 16; // fmt size
    *(uint16_t*)(wav + 20) = 1;  // PCM
    *(uint16_t*)(wav + 22) = 1;  // mono
    *(uint32_t*)(wav + 24) = AUDIO_SAMPLE_RATE;
    *(uint32_t*)(wav + 28) = AUDIO_SAMPLE_RATE * 2;
    *(uint16_t*)(wav + 32) = 2;
    *(uint16_t*)(wav + 34) = 16;
    memcpy(wav + 36, "data", 4);
    *(uint32_t*)(wav + 40) = dataSize;

    // multipart
    body += "--" + boundary + "\r\n";
    body += "Content-Disposition: form-data; name=\"model\"\r\n\r\n";
    body += "whisper-1\r\n";
    body += "--" + boundary + "\r\n";
    body += "Content-Disposition: form-data; name=\"file\"; filename=\"audio.wav\"\r\n";
    body += "Content-Type: audio/wav\r\n\r\n";

    size_t body_start = body.length();
    body.reserve(body_start + 44 + dataSize + boundary.length() + 50);
    body.concat((char*)wav, 44);
    body.concat((char*)audio.data, dataSize);
    body += "\r\n--" + boundary + "--\r\n";

    int code = 0, lat = 0;
    String resp = _httpPost(_stt_url, body,
                           ("multipart/form-data; boundary=" + boundary).c_str(),
                           code, lat);
    if (code != 200) { r.http_code = code; r.error = "STT: " + String(code); return r; }

    JsonDocument doc(2048);
    deserializeJson(doc, resp);
    r.ok = true;
    r.text = doc["text"].as<String>();
    return r;
}

// ===== LLM =====

AIResult AIService::chat(const String& user_msg) {
    auto start = millis();
    _conv.add("user", user_msg.c_str());

    AIResult r;
    if (strcmp(AI_LLM_PROVIDER, "deepseek") == 0) r = _llmDeepSeek(user_msg);
    else if (strcmp(AI_LLM_PROVIDER, "openai") == 0) r = _llmOpenAI(user_msg);
    else r = _llmDeepSeek(user_msg);  // 默认 DeepSeek

    if (r.ok) {
        _conv.add("assistant", r.text.c_str());
    }
    r.latency_ms = millis() - start;
    return r;
}

AIResult AIService::_llmDeepSeek(const String& msg) {
    AIResult r;
    JsonDocument req(8192);
    req["model"] = AI_LLM_MODEL;
    req["temperature"] = 0.7;
    req["max_tokens"] = 300;
    req["stream"] = false;

    JsonArray msgs = req["messages"].to<JsonArray>();
    JsonObject sys = msgs.add<JsonObject>();
    sys["role"] = "system";
    sys["content"] = AI_SYSTEM_PROMPT;
    for (uint8_t i = 0; i < _conv.count; i++) {
        JsonObject m = msgs.add<JsonObject>();
        m["role"] = _conv.history[i].role;
        m["content"] = _conv.history[i].content;
    }

    String body; serializeJson(req, body);

    int code = 0, lat = 0;
    String resp = _httpPost(_llm_url, body, "application/json", code, lat);

    if (code != 200) {
        r.http_code = code;
        r.error = "LLM HTTP " + String(code) + ": " + resp.substring(0, 100);
        return r;
    }

    JsonDocument doc(8192);
    deserializeJson(doc, resp);
    r.ok = true;
    r.text = doc["choices"][0]["message"]["content"].as<String>();
    return r;
}

AIResult AIService::_llmOpenAI(const String& msg) {
    // 与 DeepSeek 格式兼容，只是 URL 和 API key 不同
    return _llmDeepSeek(msg);  // 复用
}

// ===== TTS =====

TTSResult AIService::textToSpeech(const String& text) {
    auto start = millis();
    TTSResult r;
    if (strcmp(AI_TTS_PROVIDER, "openai") == 0) r = _ttsOpenAI(text);
    else if (strcmp(AI_TTS_PROVIDER, "baidu") == 0) r = _ttsBaidu(text);
    else r = _ttsEdge(text);  // 默认 Edge (免费)
    r.duration_ms = millis() - start;
    return r;
}

TTSResult AIService::_ttsEdge(const String& text) {
    // Microsoft Edge TTS (免费, 无需 API key)
    TTSResult r;
    String ssml = "<speak version='1.0' xml:lang='zh-CN'><voice name='zh-CN-XiaoxiaoNeural'>";
    ssml += text;
    ssml += "</voice></speak>";

    int code = 0, lat = 0;
    String resp = _httpPost(_tts_url, ssml, "application/ssml+xml", code, lat);

    if (code != 200 || resp.length() == 0) {
        r.http_code = code;
        r.error = "TTS HTTP " + String(code);
        return r;
    }

    // Edge TTS 返回原始 PCM 或 MP3。简化：直接当 PCM 16kHz 16bit
    // 实际是 MP3，但 ESP32 可用 helix 软解；此处预留接口
    size_t samples = resp.length() / 2;
    r.audio = (int16_t*)heap_caps_malloc(samples * sizeof(int16_t),
                                         MALLOC_CAP_SPIRAM | MALLOC_CAP_8BIT);
    if (!r.audio) r.audio = (int16_t*)malloc(samples * sizeof(int16_t));
    if (r.audio) {
        memcpy(r.audio, resp.c_str(), resp.length());
        r.len = samples;
    }
    r.ok = true;
    return r;
}

TTSResult AIService::_ttsOpenAI(const String& text) {
    TTSResult r;
    JsonDocument req(512);
    req["model"] = "tts-1";
    req["input"] = text;
    req["voice"] = "nova";
    req["response_format"] = "pcm";
    req["speed"] = 1.0;

    String body; serializeJson(req, body);

    int code = 0, lat = 0;
    String resp = _httpPost(_tts_url, body, "application/json", code, lat);

    if (code != 200 || resp.length() == 0) {
        r.http_code = code; r.error = "TTS: " + String(code); return r;
    }
    size_t samples = resp.length() / 2;
    r.audio = (int16_t*)heap_caps_malloc(samples * sizeof(int16_t),
                                         MALLOC_CAP_SPIRAM | MALLOC_CAP_8BIT);
    if (!r.audio) r.audio = (int16_t*)malloc(samples * sizeof(int16_t));
    if (r.audio) {
        memcpy(r.audio, resp.c_str(), resp.length());
        r.len = samples;
    }
    r.ok = true;
    return r;
}

TTSResult AIService::_ttsBaidu(const String& text) {
    TTSResult r;
    // 百度 TTS 返回 MP3 (需要软解)，简化示例
    String url = _tts_url + "?tex=" + urlencode(text) +
                 "&lan=zh&cuid=penguinbot&ctp=1&tok=" + _tts_key;
    // 实际实现需要 base64 token 计算，此处省略
    r.error = "Baidu TTS requires token, use Edge instead";
    return r;
}

void AIService::playTTS(const int16_t* data, size_t samples) {
    if (!data || samples == 0) return;
    size_t written = 0;
    i2s_write(I2S_NUM_0, data, samples * sizeof(int16_t), &written, portMAX_DELAY);
}

// ===== HTTP 辅助 =====

String AIService::_httpPost(const String& url, const String& body,
                             const char* contentType, int& code, int& latency) {
    auto t0 = esp_timer_get_time();
    HTTPClient http;
    http.begin(url);
    http.addHeader("Content-Type", contentType);

    // API Key 注入
    if (_llm_key.length() > 0) {
        http.addHeader("Authorization", "Bearer " + _llm_key);
    }
    if (strcmp(AI_STT_PROVIDER, "openai") == 0 && _stt_key.length() > 0) {
        http.addHeader("Authorization", "Bearer " + _stt_key);
    }

    http.setTimeout(AI_HTTP_TIMEOUT);
    code = http.POST(body);

    String resp;
    if (code > 0) resp = http.getString();
    http.end();

    latency = (esp_timer_get_time() - t0) / 1000;
    return resp;
}
