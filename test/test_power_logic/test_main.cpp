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

void test_display_relays_match_current_receiver_order() {
    TEST_ASSERT_EQUAL_UINT8(7, POWER_DEVICE_COUNT);

    TEST_ASSERT_EQUAL_STRING("sk1200", POWER_DEVICES[0].id);
    TEST_ASSERT_EQUAL_UINT8(4, POWER_DEVICES[0].outputPin);
    TEST_ASSERT_EQUAL_STRING("rx3301", POWER_DEVICES[1].id);
    TEST_ASSERT_EQUAL_UINT8(5, POWER_DEVICES[1].outputPin);
    TEST_ASSERT_EQUAL_STRING("mm238rw", POWER_DEVICES[2].id);
    TEST_ASSERT_EQUAL_UINT8(6, POWER_DEVICES[2].outputPin);
    TEST_ASSERT_EQUAL_STRING("rx8200", POWER_DEVICES[3].id);
    TEST_ASSERT_EQUAL_UINT8(7, POWER_DEVICES[3].outputPin);
    TEST_ASSERT_EQUAL_STRING("ctrl_tx_1", POWER_DEVICES[4].id);
    TEST_ASSERT_EQUAL_UINT8(9, POWER_DEVICES[4].outputPin);
    TEST_ASSERT_EQUAL_STRING("ctrl_tx_2", POWER_DEVICES[5].id);
    TEST_ASSERT_EQUAL_UINT8(10, POWER_DEVICES[5].outputPin);
    TEST_ASSERT_EQUAL_STRING("ctrl_tx_3", POWER_DEVICES[6].id);
    TEST_ASSERT_EQUAL_UINT8(11, POWER_DEVICES[6].outputPin);

    for (uint8_t i = 0; i < POWER_DEVICE_COUNT; ++i) {
        TEST_ASSERT_FALSE(POWER_DEVICES[i].defaultOn);
    }
}

int main() {
    UNITY_BEGIN();
    RUN_TEST(test_active_high_output_mapping);
    RUN_TEST(test_active_low_output_mapping);
    RUN_TEST(test_sense_mapping);
    RUN_TEST(test_valid_output_pins_exclude_uart_and_input_only_analog);
    RUN_TEST(test_display_relays_match_current_receiver_order);
    return UNITY_END();
}
