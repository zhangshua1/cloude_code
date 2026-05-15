#pragma once
#include <Arduino.h>
#include "st7789_driver.h"

enum class DisplayMode { EXPRESSION, DEBUG };

class DisplayTask {
public:
    static DisplayTask& instance() {
        static DisplayTask inst;
        return inst;
    }
    void begin();
    void run();  // 30Hz

    void setMode(DisplayMode m) { _mode = m; }
    void setExpression(Expression e) { _expr = e; _dirty = true; }
    Expression getExpression() const { return _expr; }

private:
    DisplayTask() = default;
    DisplayMode _mode = DisplayMode::EXPRESSION;
    Expression _expr = Expression::NEUTRAL;
    bool _dirty = true;
    unsigned long _last_run = 0;
};
