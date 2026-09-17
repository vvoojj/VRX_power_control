#include "SerialProtocol.h"

#include <string.h>

static bool isAddressedToController(const char* line, char nextChar = '\0') {
    const char* address = line + strspn(line, " \t\r\n");
    const size_t addressLen = strcspn(address, " \t\r\n");
    const bool completeAddress = address[addressLen] != '\0' ||
                                 nextChar == '\0' || strchr(" \t\r\n", nextChar) != 0;
    return completeAddress && addressLen == strlen(DEVICE_ID) + 1 && address[0] == '@' &&
           strncmp(address + 1, DEVICE_ID, addressLen - 1) == 0;
}

SerialProtocol::SerialProtocol(PowerController& power)
    : _power(power),
      _lineLen(0),
      _discardingLine(false),
      _pendingKind(PENDING_NONE),
      _pendingSeq(0),
      _pendingStateOn(false) {
    _line[0] = '\0';
    memset(&_pendingOn, 0, sizeof(_pendingOn));
    memset(&_pendingOff, 0, sizeof(_pendingOff));
    _pendingGroup[0] = '\0';
}

void SerialProtocol::begin() {
    const bool receiveHigh = POWER_RS485_TX_ENABLE_LEVEL == LOW;
    digitalWrite(POWER_RS485_DIRECTION_PIN, receiveHigh ? HIGH : LOW);
    pinMode(POWER_RS485_DIRECTION_PIN, OUTPUT);
    _lineLen = 0;
    _discardingLine = false;
    _line[0] = '\0';
}

void SerialProtocol::printBoot() const {
    beginTransmit();
    Serial.print('@');
    Serial.print(DEVICE_ID);
    Serial.println(F(" BOOT NAME=Power Controller"));
    endTransmit();
}

void SerialProtocol::update() {
    while (Serial.available() > 0) {
        const char c = (char)Serial.read();
        if (c == '\n' || c == '\r') {
            if (!_discardingLine && _lineLen > 0) {
                _line[_lineLen] = '\0';
                handleLine(_line);
            }
            _lineLen = 0;
            _discardingLine = false;
        } else if (_discardingLine) {
            continue;
        } else if (_lineLen < POWER_LINE_BUFFER_SIZE - 1) {
            _line[_lineLen++] = c;
        } else {
            _line[_lineLen] = '\0';
            if (isAddressedToController(_line, c)) {
                beginTransmit();
                printErr(0, "LINE_TOO_LONG");
                endTransmit();
            }
            _lineLen = 0;
            _discardingLine = true;
        }
    }
    pollCompletion();
}

void SerialProtocol::handleLine(char* line) {
    if (!isAddressedToController(line)) {
        return;
    }

    ParsedCommand cmd;
    if (!Parser::parseLine(line, cmd)) {
        beginTransmit();
        printErr(0, "BAD_FORMAT");
        endTransmit();
        return;
    }
    if (!cmd.hasSeq) {
        beginTransmit();
        printErr(0, "BAD_SEQ");
        endTransmit();
        return;
    }
    beginTransmit();
    handleCommand(cmd);
    endTransmit();
}

void SerialProtocol::handleCommand(const ParsedCommand& cmd) {
    if (strcmp(cmd.command, "PING") == 0) {
        printPrefix("PONG", cmd.seq);
        Serial.println();
        return;
    }

    if (strcmp(cmd.command, "LIST") == 0) {
        printList(cmd.seq);
        return;
    }

    if (strcmp(cmd.command, "GET_STATUS") == 0) {
        printStatus(cmd.seq);
        return;
    }

    if (strcmp(cmd.command, "SELFTEST") == 0) {
        printSelfTest(cmd.seq, cmd);
        return;
    }

    if (_pendingKind != PENDING_NONE || _power.isBusy()) {
        printErr(cmd.seq, "BUSY");
        return;
    }

    if (strcmp(cmd.command, "SET") == 0) {
        if (cmd.conflictingState) {
            printErr(cmd.seq, "CONFLICTING_STATE");
            return;
        }
        if (cmd.onList.count == 0 && cmd.offList.count == 0) {
            printErr(cmd.seq, "BAD_COMMAND");
            return;
        }
        if (!_power.setMultipleStates(cmd.onList, cmd.offList)) {
            printErr(cmd.seq, _power.lastError(), _power.lastErrorId());
            return;
        }
        copyList(_pendingOn, cmd.onList);
        copyList(_pendingOff, cmd.offList);
        _pendingKind = PENDING_SET;
        _pendingSeq = cmd.seq;
        printPrefix("ACK", cmd.seq);
        Serial.println(F(" CMD=SET"));
        return;
    }

    if (strcmp(cmd.command, "SET_GROUP") == 0) {
        if (!cmd.hasGroup || !cmd.hasState) {
            printErr(cmd.seq, "BAD_COMMAND");
            return;
        }
        if (!_power.setGroupState(cmd.group, cmd.stateOn)) {
            printErr(cmd.seq, _power.lastError(), _power.lastErrorId());
            return;
        }
        strncpy(_pendingGroup, cmd.group, sizeof(_pendingGroup) - 1);
        _pendingGroup[sizeof(_pendingGroup) - 1] = '\0';
        _pendingStateOn = cmd.stateOn;
        _pendingKind = PENDING_SET_GROUP;
        _pendingSeq = cmd.seq;
        printPrefix("ACK", cmd.seq);
        Serial.println(F(" CMD=SET_GROUP"));
        return;
    }

    if (strcmp(cmd.command, "ALL_OFF") == 0) {
        if (!_power.allOff()) {
            printErr(cmd.seq, _power.lastError(), _power.lastErrorId());
            return;
        }
        _pendingKind = PENDING_ALL_OFF;
        _pendingSeq = cmd.seq;
        printPrefix("ACK", cmd.seq);
        Serial.println(F(" CMD=ALL_OFF"));
        return;
    }

    printErr(cmd.seq, "BAD_COMMAND");
}

void SerialProtocol::pollCompletion() {
    if (_pendingKind == PENDING_NONE || _power.isBusy()) {
        return;
    }

    beginTransmit();
    if (_pendingKind == PENDING_SET) {
        printPrefix("DONE", _pendingSeq);
        printDeviceListField("ON", _pendingOn);
        printDeviceListField("OFF", _pendingOff);
        Serial.println();
    } else if (_pendingKind == PENDING_SET_GROUP) {
        printPrefix("DONE", _pendingSeq);
        Serial.print(F(" GROUP="));
        Serial.print(_pendingGroup);
        Serial.print(F(" STATE="));
        Serial.println(_pendingStateOn ? F("ON") : F("OFF"));
    } else if (_pendingKind == PENDING_ALL_OFF) {
        printPrefix("DONE", _pendingSeq);
        Serial.println(F(" ALL=OFF"));
    }
    endTransmit();
    _pendingKind = PENDING_NONE;
}

void SerialProtocol::beginTransmit() const {
    digitalWrite(POWER_RS485_DIRECTION_PIN, POWER_RS485_TX_ENABLE_LEVEL);
}

void SerialProtocol::endTransmit() const {
    Serial.flush();
    digitalWrite(POWER_RS485_DIRECTION_PIN, POWER_RS485_TX_ENABLE_LEVEL == HIGH ? LOW : HIGH);
}

void SerialProtocol::printPrefix(const char* type, uint16_t seq) const {
    Serial.print('@');
    Serial.print(DEVICE_ID);
    Serial.print(' ');
    Serial.print(type);
    Serial.print(F(" SEQ="));
    Serial.print(seq);
}

void SerialProtocol::printErr(uint16_t seq, const char* code, const char* id) const {
    printPrefix("ERR", seq);
    Serial.print(F(" CODE="));
    Serial.print(code);
    if (id != 0 && id[0] != '\0') {
        Serial.print(F(" ID="));
        Serial.print(id);
    }
    Serial.println();
}

void SerialProtocol::printList(uint16_t seq) const {
    for (uint8_t i = 0; i < _power.deviceCount(); ++i) {
        const PowerDeviceConfig& cfg = _power.configAt(i);
        const PowerDeviceState& state = _power.stateAt(i);
        printPrefix("DEVICE", seq);
        Serial.print(F(" ID="));
        Serial.print(cfg.id);
        Serial.print(F(" GROUP="));
        Serial.print(cfg.group);
        Serial.print(F(" STATE="));
        Serial.print(state.currentOn ? F("ON") : F("OFF"));
        Serial.print(F(" PIN="));
        Serial.println(cfg.outputPin);
    }
    printPrefix("LIST_DONE", seq);
    Serial.print(F(" COUNT="));
    Serial.println(_power.deviceCount());
}

void SerialProtocol::printStatus(uint16_t seq) const {
    for (uint8_t i = 0; i < _power.deviceCount(); ++i) {
        const PowerDeviceConfig& cfg = _power.configAt(i);
        const PowerDeviceState& state = _power.stateAt(i);
        printPrefix("DEVICE_STATUS", seq);
        Serial.print(F(" ID="));
        Serial.print(cfg.id);
        Serial.print(F(" STATE="));
        Serial.println(state.currentOn ? F("ON") : F("OFF"));
    }
    printPrefix("STATUS_DONE", seq);
    Serial.println();
}

void SerialProtocol::printSelfTest(uint16_t seq, const ParsedCommand& cmd) const {
    if (cmd.hasTestOutputs && cmd.testOutputs) {
        printErr(seq, "UNSUPPORTED_SELFTEST_OPTION");
        return;
    }

    const char* code = "NONE";
    bool warn = false;
    const bool pass = _power.selfTest(&code, &warn);
    printPrefix("SELFTEST", seq);
    Serial.print(F(" RESULT="));
    Serial.print(pass ? (warn ? F("WARN") : F("PASS")) : F("FAIL"));
    if (strcmp(code, "NONE") != 0) {
        Serial.print(F(" CODE="));
        Serial.print(code);
    }
    Serial.print(F(" DEVICE_COUNT="));
    Serial.println(_power.deviceCount());
}

void SerialProtocol::copyList(DeviceIdList& dst, const DeviceIdList& src) {
    memcpy(&dst, &src, sizeof(dst));
}

void SerialProtocol::printDeviceListField(const char* key, const DeviceIdList& list) const {
    if (list.count == 0) {
        return;
    }
    Serial.print(' ');
    Serial.print(key);
    Serial.print('=');
    for (uint8_t i = 0; i < list.count; ++i) {
        if (i > 0) {
            Serial.print(',');
        }
        Serial.print(list.items[i]);
    }
}
