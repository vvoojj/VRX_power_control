#include <unity.h>

#ifndef HIGH
#define HIGH 0x1
#endif

#include "config/PowerDeviceConfig.h"
#include "power/PowerDevice.h"
#include <string.h>

void setUp() {}

void tearDown() {}

void test_active_high_output_mapping() {
    TEST_ASSERT_TRUE(powerOutputHighForState(true, true));
    TEST_ASSERT_FALSE(powerOutputHighForState(true, false));
}

void test_active_low_output_mapping() {
    TEST_ASSERT_FALSE(powerOutputHighForState(false, true));
    TEST_ASSERT_TRUE(powerOutputHighForState(false, false));
}

void test_sense_mapping() {
    TEST_ASSERT_TRUE(powerSenseOnForLevel(true, true));
    TEST_ASSERT_FALSE(powerSenseOnForLevel(true, false));
    TEST_ASSERT_FALSE(powerSenseOnForLevel(false, true));
    TEST_ASSERT_TRUE(powerSenseOnForLevel(false, false));
}

void test_valid_output_pins_exclude_uart_and_input_only_analog() {
    TEST_ASSERT_FALSE(powerIsValidOutputPin(0));
    TEST_ASSERT_FALSE(powerIsValidOutputPin(1));
    TEST_ASSERT_TRUE(powerIsValidOutputPin(2));
    TEST_ASSERT_TRUE(powerIsValidOutputPin(19));
    TEST_ASSERT_FALSE(powerIsValidOutputPin(20));
    TEST_ASSERT_FALSE(powerIsValidOutputPin(21));
}

void test_first_display_relay_is_rx3301() {
    TEST_ASSERT_EQUAL_STRING("rx3301", POWER_DEVICES[0].id);
    TEST_ASSERT_EQUAL_STRING("vrx", POWER_DEVICES[0].group);
    TEST_ASSERT_EQUAL_UINT8(2, POWER_DEVICES[0].outputPin);
    TEST_ASSERT_FALSE(POWER_DEVICES[0].defaultOn);
}

int main() {
    UNITY_BEGIN();
    RUN_TEST(test_active_high_output_mapping);
    RUN_TEST(test_active_low_output_mapping);
    RUN_TEST(test_sense_mapping);
    RUN_TEST(test_valid_output_pins_exclude_uart_and_input_only_analog);
    RUN_TEST(test_first_display_relay_is_rx3301);
    return UNITY_END();
}
