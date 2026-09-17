#include <unity.h>

#include "protocol/SerialProtocol.h"

void setUp() {
    Serial = TestSerial();
    testPinWrites().clear();
}

void tearDown() {}

static void expectSilent(const std::string& input) {
    PowerController power(POWER_DEVICES, POWER_DEVICE_COUNT);
    SerialProtocol protocol(power);
    Serial.input = input;
    protocol.update();
    TEST_ASSERT_EQUAL_STRING("", Serial.output.c_str());
    TEST_ASSERT_EQUAL_UINT(0, testPinWrites().size());
    TEST_ASSERT_EQUAL_UINT(0, Serial.flushCount);
    TEST_ASSERT_FALSE(power.isBusy());
}

void test_other_device_rssi_is_silent() {
    expectSilent("@rx3301 RSSI\r\n");
}

void test_other_device_missing_command_is_silent() {
    expectSilent("@rx3301\n");
}

void test_unaddressed_traffic_is_silent() {
    expectSilent("RSSI\n@\n \t\nnoise @powerctl PING SEQ=1\n");
}

void test_address_must_match_exactly() {
    expectSilent("@powerctl2\n@powerctl2 PING SEQ=1\n@power\n@POWERCTL PING SEQ=1\n");
}

void test_other_device_cannot_switch_outputs() {
    expectSilent("@rx3301 SET ON=rx3301 SEQ=1\n");
}

void test_own_ping_preserves_whitespace_and_seq() {
    PowerController power(POWER_DEVICES, POWER_DEVICE_COUNT);
    SerialProtocol protocol(power);
    Serial.input = " \t@powerctl\tPING SEQ=42\r\n";
    protocol.update();
    TEST_ASSERT_EQUAL_STRING("@powerctl PONG SEQ=42\r\n", Serial.output.c_str());
    TEST_ASSERT_EQUAL_UINT(2, testPinWrites().size());
    TEST_ASSERT_EQUAL_UINT8(POWER_RS485_DIRECTION_PIN, testPinWrites()[0].first);
    TEST_ASSERT_EQUAL_UINT8(HIGH, testPinWrites()[0].second);
    TEST_ASSERT_EQUAL_UINT8(LOW, testPinWrites()[1].second);
    TEST_ASSERT_EQUAL_UINT(1, Serial.flushCount);
}

void test_own_malformed_command_still_reports_error() {
    PowerController power(POWER_DEVICES, POWER_DEVICE_COUNT);
    SerialProtocol protocol(power);
    Serial.input = "@powerctl\n@powerctl PING\n";
    protocol.update();
    TEST_ASSERT_EQUAL_STRING("@powerctl ERR SEQ=0 CODE=BAD_FORMAT\r\n"
                             "@powerctl ERR SEQ=0 CODE=BAD_SEQ\r\n", Serial.output.c_str());
}

void test_unrelated_oversized_line_is_silent() {
    expectSilent("@rx3301 " + std::string(POWER_LINE_BUFFER_SIZE * 3, 'x') + "\n");
}

void test_overflow_discards_command_like_tail_across_updates() {
    PowerController power(POWER_DEVICES, POWER_DEVICE_COUNT);
    SerialProtocol protocol(power);
    Serial.input = "@rx3301 " + std::string(POWER_LINE_BUFFER_SIZE - 8, 'x');
    protocol.update();
    TEST_ASSERT_EQUAL_STRING("", Serial.output.c_str());
    Serial.input += "@powerctl SET ON=rx3301 SEQ=7\r\n@powerctl PING SEQ=8\n";
    protocol.update();
    TEST_ASSERT_EQUAL_STRING("@powerctl PONG SEQ=8\r\n", Serial.output.c_str());
    TEST_ASSERT_FALSE(power.isBusy());
}

void test_truncated_foreign_address_is_silent() {
    expectSilent(std::string(POWER_LINE_BUFFER_SIZE - 10, ' ') + "@powerctl2 RSSI\n");
}

void test_own_overflow_reports_once_and_recovers() {
    PowerController power(POWER_DEVICES, POWER_DEVICE_COUNT);
    SerialProtocol protocol(power);
    Serial.input = "@powerctl " + std::string(POWER_LINE_BUFFER_SIZE * 3, 'x') +
                   "\n@powerctl PING SEQ=9\n";
    protocol.update();
    TEST_ASSERT_EQUAL_STRING("@powerctl ERR SEQ=0 CODE=LINE_TOO_LONG\r\n"
                             "@powerctl PONG SEQ=9\r\n", Serial.output.c_str());
    TEST_ASSERT_EQUAL_UINT(4, testPinWrites().size());
    TEST_ASSERT_EQUAL_UINT8(HIGH, testPinWrites()[0].second);
    TEST_ASSERT_EQUAL_UINT8(LOW, testPinWrites()[1].second);
    TEST_ASSERT_EQUAL_UINT(2, Serial.flushCount);
}

int main() {
    UNITY_BEGIN();
    RUN_TEST(test_other_device_rssi_is_silent);
    RUN_TEST(test_other_device_missing_command_is_silent);
    RUN_TEST(test_unaddressed_traffic_is_silent);
    RUN_TEST(test_address_must_match_exactly);
    RUN_TEST(test_other_device_cannot_switch_outputs);
    RUN_TEST(test_own_ping_preserves_whitespace_and_seq);
    RUN_TEST(test_own_malformed_command_still_reports_error);
    RUN_TEST(test_unrelated_oversized_line_is_silent);
    RUN_TEST(test_overflow_discards_command_like_tail_across_updates);
    RUN_TEST(test_truncated_foreign_address_is_silent);
    RUN_TEST(test_own_overflow_reports_once_and_recovers);
    return UNITY_END();
}
