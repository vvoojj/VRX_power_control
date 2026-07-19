#include "Parser.h"

#include <stdlib.h>
#include <string.h>

static void copyToken(char* dst, uint8_t dstSize, const char* src) {
    if (dstSize == 0) {
        return;
    }
    strncpy(dst, src, dstSize - 1);
    dst[dstSize - 1] = '\0';
}

static bool equalsIgnoreCase(const char* left, const char* right) {
    while (*left != '\0' && *right != '\0') {
        char a = *left++;
        char b = *right++;
        if (a >= 'a' && a <= 'z') {
            a = (char)(a - 'a' + 'A');
        }
        if (b >= 'a' && b <= 'z') {
            b = (char)(b - 'a' + 'A');
        }
        if (a != b) {
            return false;
        }
    }
    return *left == '\0' && *right == '\0';
}

bool Parser::parseLine(char* line, ParsedCommand& out) {
    memset(&out, 0, sizeof(out));

    char* token = strtok(line, " \t\r\n");
    if (token == 0 || token[0] != '@' || token[1] == '\0') {
        return false;
    }
    copyToken(out.device, sizeof(out.device), token + 1);

    token = strtok(0, " \t\r\n");
    if (token == 0) {
        return false;
    }
    copyToken(out.command, sizeof(out.command), token);

    while ((token = strtok(0, " \t\r\n")) != 0) {
        char* equals = strchr(token, '=');
        if (equals == 0) {
            continue;
        }
        *equals = '\0';
        const char* key = token;
        const char* value = equals + 1;

        if (strcmp(key, "SEQ") == 0) {
            out.seq = (uint16_t)strtoul(value, 0, 10);
            out.hasSeq = true;
        } else if (strcmp(key, "ON") == 0) {
            parseDeviceList(value, out.onList);
        } else if (strcmp(key, "OFF") == 0) {
            parseDeviceList(value, out.offList);
        } else if (strcmp(key, "GROUP") == 0) {
            copyToken(out.group, sizeof(out.group), value);
            out.hasGroup = true;
        } else if (strcmp(key, "STATE") == 0) {
            if (equalsIgnoreCase(value, "ON") || strcmp(value, "1") == 0) {
                out.stateOn = true;
                out.hasState = true;
            } else if (equalsIgnoreCase(value, "OFF") || strcmp(value, "0") == 0) {
                out.stateOn = false;
                out.hasState = true;
            }
        } else if (strcmp(key, "TEST_OUTPUTS") == 0) {
            out.testOutputs = strtoul(value, 0, 10) != 0;
            out.hasTestOutputs = true;
        }
    }

    for (uint8_t i = 0; i < out.onList.count; ++i) {
        if (listContains(out.offList, out.onList.items[i])) {
            out.conflictingState = true;
        }
    }

    return true;
}

bool Parser::listContains(const DeviceIdList& list, const char* id) {
    for (uint8_t i = 0; i < list.count; ++i) {
        if (strcmp(list.items[i], id) == 0) {
            return true;
        }
    }
    return false;
}

void Parser::parseDeviceList(const char* value, DeviceIdList& out) {
    out.count = 0;
    out.overflow = false;

    const char* start = value;
    while (*start != '\0') {
        const char* end = start;
        while (*end != '\0' && *end != ',') {
            ++end;
        }

        const uint8_t len = (uint8_t)(end - start);
        if (len > 0) {
            if (out.count >= POWER_MAX_LIST_ITEMS) {
                out.overflow = true;
            } else {
                uint8_t copyLen = len;
                if (copyLen >= POWER_MAX_DEVICE_ID_LEN) {
                    copyLen = POWER_MAX_DEVICE_ID_LEN - 1;
                }
                memcpy(out.items[out.count], start, copyLen);
                out.items[out.count][copyLen] = '\0';
                ++out.count;
            }
        }

        start = *end == ',' ? end + 1 : end;
    }
}
