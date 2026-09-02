# VRX Controller Response Delay Implementation Plan

> **For agentic workers:** REQUIRED SUB-SKILL: Use superpowers:subagent-driven-development (recommended) or superpowers:executing-plans to implement this plan task-by-task. Steps use checkbox (`- [ ]`) syntax for tracking.

**Goal:** Make the power controller wait 200 ms before transmitting every response to a received command.

**Architecture:** Keep response timing in `SerialProtocol` at the existing `beginTransmit()` boundary. Add one centralized `POWER_RESPONSE_DELAY_MS` constant, delay command responses once per response block, and provide an explicit no-delay path for boot output.

**Tech Stack:** PlatformIO, Arduino Nano ATmega328P, Arduino C++, Unity native tests.

## Global Constraints

- Keep addressed protocol format: `@DEVICE_ID COMMAND KEY=VALUE ...`.
- Every response should echo `SEQ` when present.
- Keep the device list config-driven and do not add device-specific switching logic.
- Default boot state must remain safe OFF.
- Run `pio test -e native` when available and `pio run -e powerctl` before completion.
- Delay once per response block, not once per line in a multi-line response.

---

## File Map

- Modify `src/config/PowerDeviceConfig.h`: define the centralized 200 ms response-delay constant.
- Modify `src/protocol/SerialProtocol.h`: expose the internal transmit helper's explicit delay parameter.
- Modify `src/protocol/SerialProtocol.cpp`: apply the delay to command responses and bypass it for boot output.
- Modify `docs/PROTOCOL.md`: document the response timing contract.
- Modify `docs/TESTING.md`: add timing checks to manual verification.
- Existing native tests remain unchanged because the protocol implementation depends on Arduino `Serial` and is excluded from the native environment.

### Task 1: Add the response timing configuration

**Files:**
- Modify: `src/config/PowerDeviceConfig.h:11-16`

**Interfaces:**
- Produces `POWER_RESPONSE_DELAY_MS` as a `uint16_t` constant with value `200`, consumed by `SerialProtocol`.

- [ ] **Step 1: Add the named constant beside the existing timing and transport constants.**

Add:

```cpp
static const uint16_t POWER_RESPONSE_DELAY_MS = 200;
```

Place it after `POWER_DEFAULT_SWITCH_DELAY_MS` so hardware switch timing and protocol response timing remain clearly separate.

- [ ] **Step 2: Confirm the constant is available to protocol code.**

Run: `rg "POWER_RESPONSE_DELAY_MS" src`

Expected after implementation: one definition in `src/config/PowerDeviceConfig.h`; the protocol use is added in Task 2.

- [ ] **Step 3: Commit the configuration change.**

```bash
git add src/config/PowerDeviceConfig.h
git commit -m "feat: configure response delay"
```

### Task 2: Delay every command response at the transmit boundary

**Files:**
- Modify: `src/protocol/SerialProtocol.h:37-38`
- Modify: `src/protocol/SerialProtocol.cpp:25-30,52-72,182-189`

**Interfaces:**
- Changes the private helper signature to `void beginTransmit(bool delayResponse) const`.
- Command handling calls `beginTransmit(true)`; boot output calls `beginTransmit(false)`.

- [ ] **Step 1: Update the helper declaration.**

Change the header declaration to:

```cpp
void beginTransmit(bool delayResponse) const;
```

- [ ] **Step 2: Make boot output bypass the response delay.**

Change `printBoot()` to call:

```cpp
beginTransmit(false);
```

- [ ] **Step 3: Route all command-response paths through the delayed boundary.**

In `handleLine()`, change both response calls to `beginTransmit(true)`: the parse-error path and the addressed-command path. This covers command-generated success and error responses because `handleCommand()` writes inside the already-open response block.

- [ ] **Step 4: Delay before enabling transmission.**

Replace the helper implementation with:

```cpp
void SerialProtocol::beginTransmit(bool delayResponse) const {
    if (delayResponse) {
        delay(POWER_RESPONSE_DELAY_MS);
    }
    digitalWrite(POWER_RS485_DIRECTION_PIN, POWER_RS485_TX_ENABLE_LEVEL);
}
```

Do not add a delay to `endTransmit()`. This preserves the existing `Serial.flush()` and RS485 receive transition behavior.

- [ ] **Step 5: Search for missed call sites.**

Run: `rg "beginTransmit\(" src/protocol`

Expected: `printBoot()` uses `false`, both command-response paths use `true`, and no call remains with the old zero-argument signature.

- [ ] **Step 6: Commit the protocol implementation.**

```bash
git add src/protocol/SerialProtocol.h src/protocol/SerialProtocol.cpp
git commit -m "feat: delay command responses"
```

### Task 3: Document and verify response timing

**Files:**
- Modify: `docs/PROTOCOL.md:17-20`
- Modify: `docs/TESTING.md:35-41`

**Interfaces:**
- Documents the externally observable 200 ms response delay without changing command syntax.

- [ ] **Step 1: Document the protocol timing rule.**

Add to `docs/PROTOCOL.md` after the response/SEQ rules:

```markdown
Responses are transmitted approximately 200 ms after the command line is received. The delay applies once per response block, so multi-line `LIST` and `GET_STATUS` responses are sent as one block after one delay. Boot output is not delayed by this rule.
```

- [ ] **Step 2: Document manual timing verification.**

Add to the expected behavior in `docs/TESTING.md`:

```markdown
- Command responses begin transmitting approximately 200 ms after receipt; multi-line responses wait once per command.
```

- [ ] **Step 3: Run native tests.**

Run: `pio test -e native`

Expected: all existing parser and power-logic tests pass. These tests do not measure Arduino serial timing.

- [ ] **Step 4: Build the firmware.**

Run: `pio run -e powerctl`

Expected: the `powerctl` environment builds successfully.

- [ ] **Step 5: Review the final diff.**

Run: `git diff HEAD~2 --check; git status --short`

Expected: no whitespace errors; only the intended source and documentation changes remain, apart from any pre-existing unrelated worktree changes.

- [ ] **Step 6: Commit documentation updates.**

```bash
git add docs/PROTOCOL.md docs/TESTING.md
git commit -m "docs: describe response timing"
```
