#include <Arduino.h>

#include "config/PowerDeviceConfig.h"
#include "power/PowerController.h"
#include "protocol/SerialProtocol.h"

static PowerController power(POWER_DEVICES, POWER_DEVICE_COUNT);
static SerialProtocol protocol(power);

void setup() {
    Serial.begin(115200);
    power.begin();
    protocol.begin();
    protocol.printBoot();
}

void loop() {
    power.update();
    protocol.update();
}
