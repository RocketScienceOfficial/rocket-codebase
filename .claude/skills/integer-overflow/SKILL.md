---
name: integer-overflow
description: >
  Systematically audits C/C++ firmware code for arithmetic and integer overflow bugs that are
  NOT about buffer sizing — tick/timestamp rollover, fixed-point or scaled-integer conversion
  overflow, CRC/checksum arithmetic, and signed/unsigned comparison traps outside of indexing
  contexts. Use whenever the user asks to check arithmetic, timing, scheduling, or checksum code
  for overflow — including "check this for overflow", "is this arithmetic safe", "review this
  timing/scheduling code", "check this CRC/checksum", "will this wrap around", "signed/unsigned
  issue here?". Also use proactively when editing code that computes with millisecond/microsecond
  tick values, multiplies a value by a scale factor into a fixed-width integer, computes or
  verifies a CRC/checksum, or compares a signed quantity against an unsigned one. For overflow
  that feeds a buffer size, index, or copy length, use buffer-safety instead — this skill covers
  the arithmetic that ships wrong values, not out-of-bounds memory access.
---

# Integer / Arithmetic Overflow Audit

You are auditing C/C++ embedded firmware for arithmetic that silently produces the wrong value under overflow or wraparound — as opposed to arithmetic that corrupts memory (that's `buffer-safety`'s territory: array indices, `memcpy` lengths, narrowing casts feeding a buffer size). This skill is for overflow that ships bad telemetry, breaks scheduling, or corrupts a stored/transmitted value without ever touching memory out of bounds.

This matters more here than in a typical embedded project: this hardware can sit powered on the pad for extended pre-launch holds, so millisecond-tick rollover (~49.7 days for a `uint32_t` counter) is a real operational risk, not a theoretical one.

## What to look for

Work through the code systematically. For each category, find every instance — don't stop at the first hit.

### 1. Tick / timestamp rollover

`hal_time_get_ms_since_boot()` and `osal_systime_get_ms()` return `uint32_t` milliseconds since boot — they wrap to 0 after ~49.7 days. Any code that compares two raw tick values directly, instead of computing a signed delta, breaks for one comparison window at every wraparound:

```c
// Red flag — direct relational comparison of two raw tick values
if (nextDue <= now) { ... }          // breaks once nextDue and now straddle the wrap
uint32_t elapsed = now - lastTime;   // fine on its own (unsigned subtraction wraps correctly)...
if (elapsed > someSignedThreshold) { ... }  // ...but comparing against a signed value can misbehave
```

This exact pattern exists today in the scheduler this codebase generates for every firmware build — `firmware/src/modules/runner/runner_gen.py` emits `(soonest > now) ? (soonest - now) : 0` and `if (nextDue[i] <= now) { ... }` into every board's runner. Flag any direct `<`, `<=`, `>`, `>=` between two tick values. The wraparound-safe form is a signed delta: `(int32_t)(now - nextDue) >= 0`.

Also check: is the tick value ever narrowed (`uint32_t` → `uint16_t`) before comparison? That moves the wraparound period from 49.7 days to ~65.5 seconds.

### 2. Fixed-point / scaled-integer conversion overflow

This codebase encodes some values as scaled integers to save space on the wire or in flash — e.g. `DatabaseFrame::batteryVoltage100` (`firmware/src/modules/database/DatabaseFlashConfig.h`) stores voltage × 100 in a `uint16_t`. Any `value * SCALE` written into a fixed-width field is suspect:

```c
uint16_t batteryVoltage100 = (uint16_t)(voltage * 100.0f);  // fine while voltage stays under ~655V
```

Check whether the source value's real-world range is actually bounded below what the scaled type can hold — sensor faults, uncalibrated ADC reads, or a unit mismatch (volts vs. millivolts passed in) can push the pre-scale value well outside the assumed range. Flag scale-then-narrow conversions with no range check or clamp before the cast.

### 3. CRC / checksum arithmetic

```c
raw.crc = datalink_crc16_mcrf4xx_calculate((const uint8_t *)&raw, sizeof(raw) - 2);
```

This pattern (`firmware/src/modules/database/DatabaseWriter.cpp`, `DatabaseReader.cpp`, `DatabaseMetadataController.cpp`) is correct today because the struct is `__attribute__((packed))` and `crc` is a trailing `uint16_t`. But the `- 2` is a magic number, not something the compiler ties to the struct's actual shape. Flag every such call and verify, against the *current* struct definition: the trailing field really is the CRC, it's really 2 bytes wide, and the struct is still packed. If any of those drift independently (crc field widened, a field reordered, `__attribute__((packed))` dropped or a new struct copies the pattern without it), the size arithmetic silently covers the wrong byte range and the CRC stops meaning anything.

### 4. Signed/unsigned comparison traps (outside indexing)

`buffer-safety` covers signed/unsigned mistakes that become an index or a length. This category is everything else: physical quantities, deltas, thresholds.

```c
int pressure;  // DatabaseFrame::pressure is signed
...
uint32_t threshold = get_configured_threshold();
if (pressure - lastPressure > threshold) { ... }  // pressure - lastPressure is int; comparing against
                                                     // a uint32_t promotes the int side to unsigned,
                                                     // so a negative delta becomes a huge positive one
```

Look for a signed value (sensor readings that can go negative, deltas, `int pressure`-style fields) compared against or arithmetic'd with an unsigned one. The usual-arithmetic-conversions rule silently converts the signed operand to unsigned, which is rarely what the author intended.

## How to work through the code

This is AI-native review: you have Grep and can enumerate every candidate site across the whole tree before reasoning about any single one, which a human skimming a diff cannot. Do that first.

1. **Enumerate candidates with Grep before reading anything closely** — don't rely on the file(s) the user happened to point at. Useful patterns:
   - Tick usage: `hal_time_get_ms_since_boot|osal_systime_get_ms|_ms_since_boot|get_us_since_boot`
   - CRC/checksum call sites: `crc|checksum` (case-insensitive)
   - Scaled-integer fields: field names/comments suggesting a scale factor (`*100`, `*1000`, `Voltage100`-style naming), or a multiply immediately before a narrowing cast
   - Mixed signed/unsigned: search for `int ` / `int32_t` field and local declarations, then check every comparison or arithmetic op involving them against an unsigned operand
2. **Exclude vendored submodules** — `firmware/src/lib/radio/RadioLib`, `firmware/src/modules/oled/u8g2`, `firmware/platform/pico/pico-sdk`, `firmware/platform/common/freertos/FreeRTOS-Kernel` (per CLAUDE.md's submodule list) are third-party code this team doesn't maintain. Don't spend the audit budget there unless the user specifically asks about vendor code.
3. **Trace where the value goes** — a rollover or overflow only matters once you know what consumes the bad value: a scheduling decision, a stored/transmitted field, a safety-relevant comparison (e.g. altitude, pressure, battery voltage feeding a state-machine transition).
4. **Don't flag false positives** — if a delta is already computed as a signed difference and compared correctly, say so.
5. **Suggest concrete fixes** — e.g. the actual signed-delta comparison to replace a direct tick comparison, not "handle wraparound."

## How to structure your findings

```
[SEVERITY] <File>:<Line> — <Category>
  Code: `<the offending line(s)>`
  Risk: <what goes wrong and under what condition>
  Fix:  <concrete replacement or guard to add>
```

**Severity levels:**

- **CRITICAL** — Wraparound/overflow on a path that runs unconditionally on every boot (e.g. the core scheduler) with no guard. Will eventually misbehave in the field with certainty, not just under an edge case.
- **HIGH** — Overflow plausible under realistic long-duration operation (a pad hold, a long flight, a sensor fault pushing values out of the assumed range).
- **MEDIUM** — Requires a specific edge case (unit mismatch, uncalibrated input) that's possible but not likely.
- **LOW** — Fragile arithmetic (magic-number size math, an assumption not enforced by the type system) with no demonstrated path to a wrong value today.

After listing all findings, add:

```
## Summary
- CRITICAL: N  HIGH: N  MEDIUM: N  LOW: N
- Most dangerous area: <module or function>
- Recommended first fix: <the single most urgent change>
```

## Embedded context reminders

- No exceptions and no signed-overflow-as-defined-behavior guarantee in C — signed overflow is undefined behavior, not a wraparound you can rely on, even though this section talks about "wraparound" for the unsigned cases above.
- This hardware can sit powered on the pad for extended holds — treat the 49.7-day tick wraparound as a real operational scenario, not a theoretical one.
- `python tools/audit.py`'s mechanical check does not catch anything in this file — it only greps for `malloc`/`new`/STL/exception keywords and checks board-level bus ownership. Everything above requires this semantic pass.

## Related skills

- **buffer-safety** — overflow that feeds an array index, a `memcpy` length, or a buffer size. If the overflowing value's only consequence is an out-of-bounds access, that's buffer-safety's category, not this one.
- **uninitialized-state** — struct layout and initialization hazards. If a value is wrong because it was never set (rather than because arithmetic on it wrapped), that's uninitialized-state's category.
