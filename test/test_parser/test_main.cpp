#include <unity.h>

#include <string.h>

#include "utils/Parser.h"

void setUp() {}

void tearDown() {}

static bool parse(const char* input, ParsedCommand& cmd) {
    char line[POWER_LINE_BUFFER_SIZE];
    strncpy(line, input, sizeof(line) - 1);
    line[sizeof(line) - 1] = '\0';
    return Parser::parseLine(line, cmd);
}

void test_ping_parses() {
    ParsedCommand cmd;
    TEST_ASSERT_TRUE(parse("@powerctl PING SEQ=1", cmd));
    TEST_ASSERT_EQUAL_STRING("powerctl", cmd.device);
    TEST_ASSERT_EQUAL_STRING("PING", cmd.command);
    TEST_ASSERT_TRUE(cmd.hasSeq);
    TEST_ASSERT_EQUAL_UINT16(1, cmd.seq);
}

void test_set_on_one_device_parses() {
    ParsedCommand cmd;
    TEST_ASSERT_TRUE(parse("@powerctl SET ON=rush33 SEQ=2", cmd));
    TEST_ASSERT_EQUAL_UINT8(1, cmd.onList.count);
    TEST_ASSERT_EQUAL_STRING("rush33", cmd.onList.items[0]);
    TEST_ASSERT_EQUAL_UINT8(0, cmd.offList.count);
}

void test_set_off_one_device_parses() {
    ParsedCommand cmd;
    TEST_ASSERT_TRUE(parse("@powerctl SET OFF=rush33 SEQ=3", cmd));
    TEST_ASSERT_EQUAL_UINT8(0, cmd.onList.count);
    TEST_ASSERT_EQUAL_UINT8(1, cmd.offList.count);
    TEST_ASSERT_EQUAL_STRING("rush33", cmd.offList.items[0]);
}

void test_set_on_multiple_devices_parses() {
    ParsedCommand cmd;
    TEST_ASSERT_TRUE(parse("@powerctl SET ON=rush33,readytosky12 SEQ=4", cmd));
    TEST_ASSERT_EQUAL_UINT8(2, cmd.onList.count);
    TEST_ASSERT_EQUAL_STRING("rush33", cmd.onList.items[0]);
    TEST_ASSERT_EQUAL_STRING("readytosky12", cmd.onList.items[1]);
}

void test_set_on_and_off_parses() {
    ParsedCommand cmd;
    TEST_ASSERT_TRUE(parse("@powerctl SET ON=rush33 OFF=matek12 SEQ=5", cmd));
    TEST_ASSERT_EQUAL_UINT8(1, cmd.onList.count);
    TEST_ASSERT_EQUAL_UINT8(1, cmd.offList.count);
    TEST_ASSERT_EQUAL_STRING("rush33", cmd.onList.items[0]);
    TEST_ASSERT_EQUAL_STRING("matek12", cmd.offList.items[0]);
}

void test_conflicting_state_is_detected() {
    ParsedCommand cmd;
    TEST_ASSERT_TRUE(parse("@powerctl SET ON=rush33 OFF=rush33 SEQ=6", cmd));
    TEST_ASSERT_TRUE(cmd.conflictingState);
}

void test_set_group_parses() {
    ParsedCommand cmd;
    TEST_ASSERT_TRUE(parse("@powerctl SET_GROUP GROUP=vrx STATE=OFF SEQ=7", cmd));
    TEST_ASSERT_TRUE(cmd.hasGroup);
    TEST_ASSERT_EQUAL_STRING("vrx", cmd.group);
    TEST_ASSERT_TRUE(cmd.hasState);
    TEST_ASSERT_FALSE(cmd.stateOn);
}

void test_selftest_test_outputs_parses() {
    ParsedCommand cmd;
    TEST_ASSERT_TRUE(parse("@powerctl SELFTEST TEST_OUTPUTS=1 SEQ=8", cmd));
    TEST_ASSERT_TRUE(cmd.hasTestOutputs);
    TEST_ASSERT_TRUE(cmd.testOutputs);
}

int main() {
    UNITY_BEGIN();
    RUN_TEST(test_ping_parses);
    RUN_TEST(test_set_on_one_device_parses);
    RUN_TEST(test_set_off_one_device_parses);
    RUN_TEST(test_set_on_multiple_devices_parses);
    RUN_TEST(test_set_on_and_off_parses);
    RUN_TEST(test_conflicting_state_is_detected);
    RUN_TEST(test_set_group_parses);
    RUN_TEST(test_selftest_test_outputs_parses);
    return UNITY_END();
}
