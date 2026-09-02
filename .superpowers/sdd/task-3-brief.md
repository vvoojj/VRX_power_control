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

Run: `git diff --check; git status --short`

Expected: no whitespace errors; only the intended source and documentation changes remain, apart from any pre-existing unrelated worktree changes.

- [ ] **Step 6: Commit documentation updates.**

```bash
git add docs/PROTOCOL.md docs/TESTING.md
git commit -m "docs: describe response timing"
```
