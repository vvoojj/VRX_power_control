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

Implement:

```cpp
void SerialProtocol::beginTransmit(bool delayResponse) const {
    if (delayResponse) {
        delay(POWER_RESPONSE_DELAY_MS);
    }
    digitalWrite(POWER_RS485_DIRECTION_PIN, POWER_RS485_TX_ENABLE_LEVEL);
}
```

Do not add a delay to `endTransmit()`.

- [ ] **Step 5: Search for missed call sites.**

Run: `git grep -n "beginTransmit(" -- src/protocol`

Expected: `printBoot()` uses `false`, both command-response paths use `true`, and no call remains with the old zero-argument signature.

- [ ] **Step 6: Commit the protocol implementation.**

```bash
git add src/protocol/SerialProtocol.h src/protocol/SerialProtocol.cpp
git commit -m "feat: delay command responses"
```
