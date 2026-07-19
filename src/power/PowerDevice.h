#pragma once

#include <stdint.h>

struct PowerDeviceConfig {
    const char* id;
    const char* group;
    uint8_t outputPin;
    bool activeHigh;
    uint16_t switchDelayMs;
    bool hasSensePin;
    uint8_t sensePin;
    bool senseActiveHigh;
    bool defaultOn;
};

struct PowerDeviceState {
    bool desiredOn;
    bool currentOn;
    uint32_t lastSwitchMs;
};

bool powerOutputHighForState(bool activeHigh, bool on);
bool powerSenseOnForLevel(bool senseActiveHigh, bool levelHigh);
bool powerIsValidOutputPin(uint8_t pin);
