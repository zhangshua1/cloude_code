#pragma once
#include <Arduino.h>
#include "../config.h"

class Encoder {
public:
    static Encoder& instance() {
        static Encoder inst;
        return inst;
    }
    void begin();
    int32_t getLeft()  const { return _count_l; }
    int32_t getRight() const { return _count_r; }
    void reset();

    static void isrL() { instance()._count_l++; }
    static void isrR() { instance()._count_r++; }

private:
    Encoder() = default;
    volatile int32_t _count_l = 0;
    volatile int32_t _count_r = 0;
};
