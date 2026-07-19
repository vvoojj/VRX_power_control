#pragma once

#include <Arduino.h>

#include "../power/PowerController.h"
#include "../utils/Parser.h"

class SerialProtocol {
public:
    explicit SerialProtocol(PowerController& power);

    void begin();
    void printBoot() const;
    void update();

private:
    enum PendingKind {
        PENDING_NONE,
        PENDING_SET,
        PENDING_SET_GROUP,
        PENDING_ALL_OFF
    };

    PowerController& _power;
    char _line[POWER_LINE_BUFFER_SIZE];
    uint8_t _lineLen;
    PendingKind _pendingKind;
    uint16_t _pendingSeq;
    DeviceIdList _pendingOn;
    DeviceIdList _pendingOff;
    char _pendingGroup[POWER_MAX_DEVICE_ID_LEN];
    bool _pendingStateOn;

    void handleLine(char* line);
    void handleCommand(const ParsedCommand& cmd);
    void pollCompletion();
    void beginTransmit() const;
    void endTransmit() const;
    void printPrefix(const char* type, uint16_t seq) const;
    void printErr(uint16_t seq, const char* code, const char* id = 0) const;
    void printList(uint16_t seq) const;
    void printStatus(uint16_t seq) const;
    void printSelfTest(uint16_t seq, const ParsedCommand& cmd) const;
    void copyList(DeviceIdList& dst, const DeviceIdList& src);
    void printDeviceListField(const char* key, const DeviceIdList& list) const;
};
