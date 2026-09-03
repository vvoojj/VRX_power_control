# Power Controller Protocol

UART/RS485 runs at `115200` baud. Commands are one line each and use fixed addressed format for RS485 shared-bus support:

```text
@DEVICE_ID COMMAND KEY=VALUE KEY=VALUE ...
```

This controller listens to:

```text
@powerctl
```

Commands for other device IDs are ignored.

Every request should include `SEQ`; responses echo the same `SEQ`.

On Arduino Nano, MAX485 direction control uses `D2` connected to `DE` and `/RE` together. The firmware drives `D2` HIGH while transmitting a response and LOW while listening.

## ESP32 Integration

- Send one command per line ending with `\n`.
- Include `SEQ` on every command.
- For switching commands, wait for `ACK`, then wait for `DONE` or `ERR`.
- Do not send another `SET`, `SET_GROUP`, or `ALL_OFF` while the controller is busy.
- Use timeouts and retry at the ESP32 layer.
- Use `LIST` to discover configured devices.
- Use `GET_STATUS` after switching to refresh state.

## PING

Request:

```text
@powerctl PING SEQ=1
```

Response:

```text
@powerctl PONG SEQ=1
```

## LIST

Request:

```text
@powerctl LIST SEQ=2
```

Response is split per device to stay Nano-friendly:

```text
@powerctl DEVICE SEQ=2 ID=rx3301 GROUP=vrx STATE=OFF PIN=2
@powerctl DEVICE SEQ=2 ID=readytosky12 GROUP=vrx STATE=OFF PIN=3
@powerctl LIST_DONE SEQ=2 COUNT=9
```

## GET_STATUS

Request:

```text
@powerctl GET_STATUS SEQ=3
```

Response:

```text
@powerctl DEVICE_STATUS SEQ=3 ID=rx3301 STATE=OFF
@powerctl DEVICE_STATUS SEQ=3 ID=readytosky12 STATE=ON
@powerctl STATUS_DONE SEQ=3
```

## SET

Turn one device ON:

```text
@powerctl SET ON=rx3301 SEQ=10
```

Turn one device OFF:

```text
@powerctl SET OFF=rx3301 SEQ=11
```

Turn multiple devices ON:

```text
@powerctl SET ON=rx3301,readytosky12,matek12 SEQ=12
```

Turn multiple devices OFF:

```text
@powerctl SET OFF=rx3301,readytosky12 SEQ=13
```

Switch RX3301 exclusively from the other display VRXs:

```text
@powerctl SET ON=rx3301 OFF=readytosky12,matek12,rx3364pro,tbs_fusion SEQ=4
```

Responses:

```text
@powerctl ACK SEQ=4 CMD=SET
@powerctl DONE SEQ=4 ON=rx3301 OFF=readytosky12,matek12,rx3364pro,tbs_fusion
```

Errors:

```text
@powerctl ERR SEQ=14 CODE=UNKNOWN_DEVICE ID=bad_name
@powerctl ERR SEQ=14 CODE=CONFLICTING_STATE
@powerctl ERR SEQ=14 CODE=BUSY
```

## SET_GROUP

Examples:

```text
@powerctl SET_GROUP GROUP=vrx STATE=OFF SEQ=20
@powerctl SET_GROUP GROUP=ctrl_tx STATE=ON SEQ=21
```

Responses:

```text
@powerctl ACK SEQ=20 CMD=SET_GROUP
@powerctl DONE SEQ=20 GROUP=vrx STATE=OFF
```

Unknown group:

```text
@powerctl ERR SEQ=20 CODE=UNKNOWN_GROUP ID=bad_group
```

## ALL_OFF

Request:

```text
@powerctl ALL_OFF SEQ=30
```

Response:

```text
@powerctl ACK SEQ=30 CMD=ALL_OFF
@powerctl DONE SEQ=30 ALL=OFF
```

## SELFTEST

Safe default selftest:

```text
@powerctl SELFTEST SEQ=40
```

Response:

```text
@powerctl SELFTEST SEQ=40 RESULT=PASS DEVICE_COUNT=9
```

Default `SELFTEST` does not toggle outputs. It validates the config table for duplicate IDs, duplicate output pins, invalid pins, and valid device count.

Unsafe output selftest is not implemented:

```text
@powerctl SELFTEST TEST_OUTPUTS=1 SEQ=41
@powerctl ERR SEQ=41 CODE=UNSUPPORTED_SELFTEST_OPTION
```

## Error Codes

| Code | Meaning |
| --- | --- |
| `BAD_FORMAT` | Line could not be parsed. |
| `BAD_SEQ` | Missing `SEQ`. |
| `BAD_COMMAND` | Command or required fields are invalid. |
| `UNKNOWN_DEVICE` | Requested device ID is not configured. |
| `UNKNOWN_GROUP` | Requested group is not configured. |
| `CONFLICTING_STATE` | Same device was requested ON and OFF. |
| `BUSY` | A switching operation is already running. |
| `TOO_MANY_DEVICES` | Command exceeded fixed list capacity. |
| `UNSUPPORTED_SELFTEST_OPTION` | Unsafe selftest option is not implemented. |
