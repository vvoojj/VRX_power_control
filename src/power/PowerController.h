#pragma once

#include <Arduino.h>

#include "../config/PowerDeviceConfig.h"
#include "../utils/Parser.h"

class PowerController {
public:
    PowerController(const PowerDeviceConfig* configs, uint8_t count);

    void begin();
    void update();

    bool setDeviceState(const char* id, bool on);
    bool setMultipleStates(const DeviceIdList& onList, const DeviceIdList& offList);
    bool setGroupState(const char* group, bool on);
    bool allOff();

    bool isBusy() const;
    uint8_t deviceCount() const;
    const PowerDeviceConfig& configAt(uint8_t index) const;
    const PowerDeviceState& stateAt(uint8_t index) const;
    int8_t findDevice(const char* id) const;
    bool groupExists(const char* group) const;
    const char* lastError() const;
    const char* lastErrorId() const;
    bool selfTest(const char** code, bool* warn) const;

private:
    struct PendingAction {
        uint8_t index;
        bool on;
    };

    const PowerDeviceConfig* _configs;
    uint8_t _count;
    PowerDeviceState _states[POWER_MAX_DEVICES];
    PendingAction _actions[POWER_MAX_LIST_ITEMS];
    uint8_t _actionCount;
    uint8_t _nextAction;
    bool _busy;
    bool _waitingBetweenActions;
    uint32_t _waitStartedAt;
    uint16_t _waitMs;
    const char* _lastError;
    char _lastErrorId[POWER_MAX_DEVICE_ID_LEN];

    void clearActions();
    bool enqueueAction(uint8_t index, bool on);
    bool startActions();
    void applyState(uint8_t index, bool on);
    void setError(const char* code, const char* id = "");
};
