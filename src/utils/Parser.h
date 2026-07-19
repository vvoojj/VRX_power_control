#pragma once

#include <stdint.h>

static const uint8_t POWER_LINE_BUFFER_SIZE = 128;
static const uint8_t POWER_MAX_DEVICE_ID_LEN = 16;
static const uint8_t POWER_MAX_LIST_ITEMS = 8;

struct DeviceIdList {
    char items[POWER_MAX_LIST_ITEMS][POWER_MAX_DEVICE_ID_LEN];
    uint8_t count;
    bool overflow;
};

struct ParsedCommand {
    char device[17];
    char command[16];
    uint16_t seq;
    bool hasSeq;
    DeviceIdList onList;
    DeviceIdList offList;
    char group[POWER_MAX_DEVICE_ID_LEN];
    bool hasGroup;
    bool stateOn;
    bool hasState;
    bool testOutputs;
    bool hasTestOutputs;
    bool conflictingState;
};

class Parser {
public:
    static bool parseLine(char* line, ParsedCommand& out);
    static bool listContains(const DeviceIdList& list, const char* id);

private:
    static void parseDeviceList(const char* value, DeviceIdList& out);
};
