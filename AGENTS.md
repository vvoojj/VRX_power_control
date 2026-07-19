# AGENTS.md

## Project

- Separate PlatformIO Arduino Nano ATmega328P power controller project.
- This is not the ReadyToSky/RUSH/Matek VRX channel-switching firmware.
- This project only controls external power/enable lines for devices.

## Required Reading

- Before modifying firmware, read `docs/PROJECT.md`, `docs/PROTOCOL.md`, and `docs/TESTING.md`.

## Build

- Build firmware with `pio run -e powerctl`.
- Run native tests with `pio test -e native` when available.
- Run `pio run -e powerctl` before final response.

## Hardware Rules

- Do not power external devices directly from Arduino GPIO pins.
- GPIO pins must drive MOSFET/load-switch/relay enable inputs.
- Keep all pin assignments centralized in `src/config/PowerDeviceConfig.h`.
- Default boot state must be safe OFF unless a config row explicitly says otherwise.
- Initialize outputs to inactive level before `pinMode(..., OUTPUT)` where possible.

## Protocol Rules

- Keep addressed protocol format: `@DEVICE_ID COMMAND KEY=VALUE ...`.
- Keep protocol compatible with future ESP32-S3 controller and RS485 shared bus.
- Ignore commands for other device IDs.
- Every response should echo `SEQ` when present.
- Do not use Arduino `String` in parser/protocol code unless there is a concrete reason.

## Architecture Rules

- Keep the device list config-driven.
- Adding/removing devices should mostly mean editing one row in `POWER_DEVICES[]`.
- Do not add device-specific if/else switching logic.
- Default `SELFTEST` must not toggle hardware.
- Unsafe output tests must require an explicit option and be clearly documented.
