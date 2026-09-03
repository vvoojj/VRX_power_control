# Power Controller Project

This is a separate PlatformIO Arduino project for an Arduino Nano ATmega328P power/enable controller. It is not the VRX channel-switching firmware.

The Arduino receives addressed UART commands from a future ESP32-S3 main controller, switches configured power-enable outputs, and reports final logical state.

## Hardware Model

- Target: Arduino Nano ATmega328P, 5V.
- Transport now: UART Serial at `115200` baud.
- RS485 transport: MAX485-compatible half-duplex bus using the same addressed protocol format.
- RS485 direction control: Arduino `D2` drives MAX485 `DE` and `/RE` together, HIGH for transmit and LOW for receive.
- Arduino GPIO pins drive MOSFET/load-switch/relay enable inputs only.
- Do not power external devices directly from Arduino pins.
- High-current VRX modules and transmitter boards require suitable external power supplies and switching hardware.

## Config-Driven Devices

Devices are configured in `src/config/PowerDeviceConfig.h` using `POWER_DEVICES[]`.

Each row contains:

```cpp
struct PowerDeviceConfig {
    const char* id;
    const char* group;
    uint8_t outputPin;
    bool activeHigh;
    uint16_t switchDelayMs;
    bool hasSensePin;
    uint8_t sensePin;
    bool senseActiveHigh;
    bool defaultOn;
};
```

Initial devices:

| ID | Group | Pin |
| --- | --- | --- |
| `rx3301` | `vrx` | `D4` |
| `readytosky12` | `vrx` | `D5` |
| `matek12` | `vrx` | `D6` |
| `rx3364pro` | `vrx` | `D7` |
| `tbs_fusion` | `vrx` | `D8` |
| `ctrl_tx_1` | `ctrl_tx` | `D9` |
| `ctrl_tx_2` | `ctrl_tx` | `D10` |
| `ctrl_tx_3` | `ctrl_tx` | `D11` |
| `skyzone_x` | `vrx` | `D13` |

`D2` is reserved for RS485 direction control. The first five `vrx` outputs (`D4..D8`) intentionally match the CrowPanel display order: RX3301, ReadyToSky, Matek, RX3364 Pro, then TBS Fusion. `D3` is free.

All initial devices default to OFF at boot.

## Adding Or Removing Devices

To add a device, add one row to `POWER_DEVICES[]` and update `POWER_DEVICE_COUNT`.

Keep IDs short and stable because the ESP32 uses them in commands.

Use pins `D4..D13` or `A0..A5` for outputs. Avoid `D0/D1` because they are UART. Avoid `D2` because it is RS485 direction control. Avoid `A6/A7` because they are input-only on Nano.

## Startup Safety

On boot, outputs are written to their OFF/inactive level before `pinMode(..., OUTPUT)` is called. This reduces output glitches.

Devices are not all enabled at boot unless their config row sets `defaultOn=true`.

## File Structure

- `platformio.ini`: PlatformIO environments.
- `src/main.cpp`: startup and loop.
- `src/config/PowerDeviceConfig.h`: device table and constants.
- `src/power/PowerDevice.*`: pure output/sense helper logic.
- `src/power/PowerController.*`: table-driven output state machine.
- `src/protocol/SerialProtocol.*`: addressed UART command protocol.
- `src/utils/Parser.*`: fixed-buffer command parser.
- `docs/PROTOCOL.md`: UART command reference.
- `docs/TESTING.md`: manual and native testing workflow.

## Build Commands

Build:

```sh
pio run -e powerctl
```

Upload:

```sh
pio run -e powerctl -t upload
```

Serial monitor:

```sh
pio device monitor -e powerctl
```

If upload fails on an old Nano bootloader, try enabling this in `platformio.ini`:

```ini
upload_speed = 57600
```
