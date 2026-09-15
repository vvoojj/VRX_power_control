#pragma once

#include <stdint.h>

#include "../power/PowerDevice.h"

#ifndef DEVICE_ID
#define DEVICE_ID "powerctl"
#endif

static const uint8_t POWER_MAX_DEVICES = 16;
static const uint8_t POWER_DEVICE_COUNT = 7;
static const uint16_t POWER_DEFAULT_SWITCH_DELAY_MS = 100;
static const uint8_t POWER_NO_SENSE_PIN = 255;
static const uint8_t POWER_RS485_DIRECTION_PIN = 2;
static const uint8_t POWER_RS485_TX_ENABLE_LEVEL = HIGH;

// Arduino GPIO controls MOSFET/load-switch/relay enable inputs only.
// Do not power high-current devices directly from Arduino pins.
// TODO: replace placeholder pins with final hardware wiring before deployment.
static const PowerDeviceConfig POWER_DEVICES[POWER_DEVICE_COUNT] = {
    {"sk1200", "vrx", 4, true, POWER_DEFAULT_SWITCH_DELAY_MS, false, POWER_NO_SENSE_PIN, true, false},
    {"rx3301", "vrx", 5, true, POWER_DEFAULT_SWITCH_DELAY_MS, false, POWER_NO_SENSE_PIN, true, false},
    {"mm238rw", "vrx", 6, true, POWER_DEFAULT_SWITCH_DELAY_MS, false, POWER_NO_SENSE_PIN, true, false},
    {"rx8200", "vrx", 7, true, POWER_DEFAULT_SWITCH_DELAY_MS, false, POWER_NO_SENSE_PIN, true, false},
    {"ctrl_tx_1", "ctrl_tx", 9, true, POWER_DEFAULT_SWITCH_DELAY_MS, false, POWER_NO_SENSE_PIN, true, false},
    {"ctrl_tx_2", "ctrl_tx", 10, true, POWER_DEFAULT_SWITCH_DELAY_MS, false, POWER_NO_SENSE_PIN, true, false},
    {"ctrl_tx_3", "ctrl_tx", 11, true, POWER_DEFAULT_SWITCH_DELAY_MS, false, POWER_NO_SENSE_PIN, true, false}
};
