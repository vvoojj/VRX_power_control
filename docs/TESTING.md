# Testing

## Build

```sh
pio run -e powerctl
```

## Upload

```sh
pio run -e powerctl -t upload
```

## Serial Monitor

```sh
pio device monitor -e powerctl
```

## Manual Serial Commands

```text
@powerctl PING SEQ=1
@powerctl LIST SEQ=2
@powerctl GET_STATUS SEQ=3
@powerctl SET ON=rx3301 SEQ=4
@powerctl SET OFF=rx3301 SEQ=5
@powerctl SET ON=rx3301 OFF=sk1200,mm238rw,rx8200 SEQ=6
@powerctl SET_GROUP GROUP=vrx STATE=OFF SEQ=7
@powerctl ALL_OFF SEQ=8
@powerctl SELFTEST SEQ=9
```

Expected behavior:

- `PING` returns `PONG`.
- `LIST` prints one `DEVICE` line per configured row plus `LIST_DONE`.
- `GET_STATUS` prints one `DEVICE_STATUS` line per configured row plus `STATUS_DONE`.
- `SET`, `SET_GROUP`, and `ALL_OFF` return `ACK`, physically switch outputs, then return `DONE`.
- `SELFTEST` is safe by default and does not toggle outputs.

Warnings:

- `SET`, `SET_GROUP`, and `ALL_OFF` physically change output pins and may switch external devices.
- Arduino pins must drive external MOSFET/load-switch/relay enable inputs, not device power directly.
- Do not connect high-current device loads directly to Arduino pins.

## Native Tests

```sh
pio test -e native
```

Current native tests cover:

- Parser support for `PING`, `SET`, lists, `SET_GROUP`, and `SELFTEST TEST_OUTPUTS=1`.
- Conflict detection for `ON=rx3301 OFF=rx3301`.
- Active-high and active-low output mapping.
- Sense pin logic.
- Output pin validation for Nano-safe pins.
