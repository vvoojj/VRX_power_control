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
