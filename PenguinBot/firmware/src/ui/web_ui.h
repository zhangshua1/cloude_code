#pragma once
#include <Arduino.h>
#include <SPIFFS.h>

class WebUI {
public:
    static WebUI& instance() {
        static WebUI inst;
        return inst;
    }
    void begin();  // 挂载 SPIFFS, 注册路由
};
