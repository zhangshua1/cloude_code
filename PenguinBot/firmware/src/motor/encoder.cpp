#include "encoder.h"

void Encoder::begin() {
    pinMode(ENC_L_A, INPUT_PULLUP);
    pinMode(ENC_L_B, INPUT_PULLUP);
    pinMode(ENC_R_A, INPUT_PULLUP);
    pinMode(ENC_R_B, INPUT_PULLUP);
    attachInterrupt(digitalPinToInterrupt(ENC_L_A), isrL, RISING);
    attachInterrupt(digitalPinToInterrupt(ENC_R_A), isrR, RISING);
}

void Encoder::reset() {
    noInterrupts();
    _count_l = 0;
    _count_r = 0;
    interrupts();
}
