#include "PowerController.h"

#include <string.h>

PowerController::PowerController(const PowerDeviceConfig* configs, uint8_t count)
    : _configs(configs),
      _count(count),
      _actionCount(0),
      _nextAction(0),
      _busy(false),
      _waitingBetweenActions(false),
      _waitStartedAt(0),
      _waitMs(0),
      _lastError("NONE") {
    _lastErrorId[0] = '\0';
}

void PowerController::begin() {
    for (uint8_t i = 0; i < _count && i < POWER_MAX_DEVICES; ++i) {
        const PowerDeviceConfig& cfg = _configs[i];
        const bool offHigh = powerOutputHighForState(cfg.activeHigh, false);
        digitalWrite(cfg.outputPin, offHigh ? HIGH : LOW);
        pinMode(cfg.outputPin, OUTPUT);

        _states[i].desiredOn = cfg.defaultOn;
        _states[i].currentOn = cfg.defaultOn;
        _states[i].lastSwitchMs = millis();
        if (cfg.defaultOn) {
            digitalWrite(cfg.outputPin, powerOutputHighForState(cfg.activeHigh, true) ? HIGH : LOW);
        }
    }
}

void PowerController::update() {
    if (!_busy) {
        return;
    }

    if (_waitingBetweenActions) {
        if ((uint32_t)(millis() - _waitStartedAt) < _waitMs) {
            return;
        }
        _waitingBetweenActions = false;
    }

    if (_nextAction >= _actionCount) {
        _busy = false;
        return;
    }

    const PendingAction& action = _actions[_nextAction];
    applyState(action.index, action.on);
    ++_nextAction;

    if (_nextAction >= _actionCount) {
        _busy = false;
        return;
    }

    _waitMs = _configs[action.index].switchDelayMs;
    _waitStartedAt = millis();
    _waitingBetweenActions = _waitMs > 0;
}

bool PowerController::setDeviceState(const char* id, bool on) {
    DeviceIdList onList;
    DeviceIdList offList;
    memset(&onList, 0, sizeof(onList));
    memset(&offList, 0, sizeof(offList));
    DeviceIdList& target = on ? onList : offList;
    strncpy(target.items[0], id, POWER_MAX_DEVICE_ID_LEN - 1);
    target.items[0][POWER_MAX_DEVICE_ID_LEN - 1] = '\0';
    target.count = 1;
    return setMultipleStates(onList, offList);
}

bool PowerController::setMultipleStates(const DeviceIdList& onList, const DeviceIdList& offList) {
    if (_busy) {
        setError("BUSY");
        return false;
    }
    if (onList.overflow || offList.overflow) {
        setError("TOO_MANY_DEVICES");
        return false;
    }

    clearActions();
    for (uint8_t i = 0; i < onList.count; ++i) {
        const int8_t index = findDevice(onList.items[i]);
        if (index < 0) {
            setError("UNKNOWN_DEVICE", onList.items[i]);
            return false;
        }
        if (!enqueueAction((uint8_t)index, true)) {
            return false;
        }
    }
    for (uint8_t i = 0; i < offList.count; ++i) {
        const int8_t index = findDevice(offList.items[i]);
        if (index < 0) {
            setError("UNKNOWN_DEVICE", offList.items[i]);
            return false;
        }
        if (!enqueueAction((uint8_t)index, false)) {
            return false;
        }
    }

    return startActions();
}

bool PowerController::setGroupState(const char* group, bool on) {
    if (_busy) {
        setError("BUSY");
        return false;
    }

    clearActions();
    for (uint8_t i = 0; i < _count && i < POWER_MAX_DEVICES; ++i) {
        if (strcmp(_configs[i].group, group) == 0) {
            if (!enqueueAction(i, on)) {
                return false;
            }
        }
    }
    if (_actionCount == 0) {
        setError("UNKNOWN_GROUP", group);
        return false;
    }
    return startActions();
}

bool PowerController::allOff() {
    if (_busy) {
        setError("BUSY");
        return false;
    }

    clearActions();
    for (uint8_t i = 0; i < _count && i < POWER_MAX_DEVICES; ++i) {
        if (!enqueueAction(i, false)) {
            return false;
        }
    }
    return startActions();
}

bool PowerController::isBusy() const {
    return _busy;
}

uint8_t PowerController::deviceCount() const {
    return _count;
}

const PowerDeviceConfig& PowerController::configAt(uint8_t index) const {
    return _configs[index];
}

const PowerDeviceState& PowerController::stateAt(uint8_t index) const {
    return _states[index];
}

int8_t PowerController::findDevice(const char* id) const {
    for (uint8_t i = 0; i < _count && i < POWER_MAX_DEVICES; ++i) {
        if (strcmp(_configs[i].id, id) == 0) {
            return (int8_t)i;
        }
    }
    return -1;
}

bool PowerController::groupExists(const char* group) const {
    for (uint8_t i = 0; i < _count && i < POWER_MAX_DEVICES; ++i) {
        if (strcmp(_configs[i].group, group) == 0) {
            return true;
        }
    }
    return false;
}

const char* PowerController::lastError() const {
    return _lastError;
}

const char* PowerController::lastErrorId() const {
    return _lastErrorId;
}

bool PowerController::selfTest(const char** code, bool* warn) const {
    *code = "NONE";
    *warn = false;
    if (_count == 0 || _count > POWER_MAX_DEVICES) {
        *code = "BAD_DEVICE_COUNT";
        return false;
    }
    for (uint8_t i = 0; i < _count; ++i) {
        if (_configs[i].id == 0 || _configs[i].id[0] == '\0') {
            *code = "BAD_DEVICE_ID";
            return false;
        }
        if (!powerIsValidOutputPin(_configs[i].outputPin)) {
            *code = "BAD_OUTPUT_PIN";
            return false;
        }
        for (uint8_t j = i + 1; j < _count; ++j) {
            if (strcmp(_configs[i].id, _configs[j].id) == 0) {
                *code = "DUPLICATE_ID";
                return false;
            }
            if (_configs[i].outputPin == _configs[j].outputPin) {
                *code = "DUPLICATE_OUTPUT_PIN";
                return false;
            }
        }
    }
    return true;
}

void PowerController::clearActions() {
    _actionCount = 0;
    _nextAction = 0;
    _waitingBetweenActions = false;
    setError("NONE");
}

bool PowerController::enqueueAction(uint8_t index, bool on) {
    for (uint8_t i = 0; i < _actionCount; ++i) {
        if (_actions[i].index == index) {
            if (_actions[i].on != on) {
                setError("CONFLICTING_STATE", _configs[index].id);
                return false;
            }
            return true;
        }
    }

    if (_actionCount >= POWER_MAX_LIST_ITEMS) {
        setError("TOO_MANY_DEVICES");
        return false;
    }
    _actions[_actionCount].index = index;
    _actions[_actionCount].on = on;
    ++_actionCount;
    return true;
}

bool PowerController::startActions() {
    _nextAction = 0;
    _waitingBetweenActions = false;
    _busy = true;
    setError("NONE");
    return true;
}

void PowerController::applyState(uint8_t index, bool on) {
    const PowerDeviceConfig& cfg = _configs[index];
    _states[index].desiredOn = on;
    _states[index].currentOn = on;
    _states[index].lastSwitchMs = millis();
    digitalWrite(cfg.outputPin, powerOutputHighForState(cfg.activeHigh, on) ? HIGH : LOW);
}

void PowerController::setError(const char* code, const char* id) {
    _lastError = code;
    strncpy(_lastErrorId, id, sizeof(_lastErrorId) - 1);
    _lastErrorId[sizeof(_lastErrorId) - 1] = '\0';
}
