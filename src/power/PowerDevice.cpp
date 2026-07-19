#include "PowerDevice.h"

bool powerOutputHighForState(bool activeHigh, bool on) {
    return activeHigh ? on : !on;
}

bool powerSenseOnForLevel(bool senseActiveHigh, bool levelHigh) {
    return senseActiveHigh ? levelHigh : !levelHigh;
}

bool powerIsValidOutputPin(uint8_t pin) {
    // Keep D0/D1 free for UART. Nano A6/A7 are input-only and are excluded.
    return pin >= 2 && pin <= 19;
}
