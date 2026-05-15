#pragma once
#include <Arduino.h>
#include "ai_service.h"
#include "../display/display_task.h"
#include "../servo/animation.h"

enum class ConvState {
    IDLE,           // 等待触发
    LISTENING,      // 录音中 (显示"正在听...")
    THINKING_STT,   // 语音识别中
    THINKING_LLM,   // 大模型思考中
    SPEAKING,       // TTS 播放中
    REPLY_DISPLAY,  // 显示文字回复
    ERROR           // 出错
};

class ConversationTask {
public:
    static ConversationTask& instance() {
        static ConversationTask inst;
        return inst;
    }
    bool begin();
    void run();  // 在 loop 中调用，事件驱动

    // 触发对话
    void trigger();             // 按键触发
    void triggerText(const char* text);  // 文本直接输入 (WebUI)

    ConvState state() const { return _state; }
    const char* lastReply() const { return _last_reply; }
    void clearHistory() { AIService::instance().clearHistory(); }

private:
    ConversationTask() = default;
    ConvState _state = ConvState::IDLE;
    AudioBuffer _audio_buf = {};
    char _last_reply[512] = {};
    bool _allocated = false;
    uint32_t _state_entered_ms = 0;

    void _setState(ConvState s);
    void _cleanupAudio();

    // 状态处理函数
    void _handleIdle();
    void _handleListening();
    void _handleSttThinking();
    void _handleLlmThinking();
    void _handleSpeaking(AIResult& stt_r, AIResult* llm_r = nullptr);
    void _handleError(const char* err);

    // 表情联动
    void _expressForState();
};
