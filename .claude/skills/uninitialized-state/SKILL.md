---
name: uninitialized-state
description: >
  Systematically audits C/C++ firmware code for the static-allocation analog of use-after-free:
  reading state before it has been validly set. Covers execution-pool init ordering (a module's
  run() reading pubsub data before any publisher has run), uninitialized C++ class/struct members,
  and struct padding or platform-width-dependent fields leaking non-deterministic content into
  data that's memcpy'd or written wholesale to flash or the wire. Use whenever the user asks to
  check initialization, init order, uninitialized members, or "is this safe to read here" —
  including "check this module's init", "review this constructor", "any uninitialized members?",
  "is this struct safe to memcpy/write to flash?". Also use proactively when editing a module's
  constructor or init(), a run.json/execution-pool config, or code that writes a struct wholesale
  to flash or the wire via memcpy or a CRC over sizeof(struct).
---

# Uninitialized State / Init-Order Audit

This firmware forbids heap allocation — everything is static, global, or stack-allocated, so there's no `malloc`/`free` and no use-after-free. But the same failure *shape* still exists here: code reading state before it has been validly established. Three concrete forms of that in this codebase:

1. A module's `run()` reads a pubsub topic before any publisher has called `publish()` on it.
2. A C++ class member is never assigned by the constructor or an in-class default.
3. A struct written wholesale to flash or the wire (via `memcpy` or a CRC over `sizeof(struct)`) contains bytes — padding, or a platform-width-dependent field — whose content isn't fully determined by the fields the author thinks they set.

## What to look for

### 1. Cross-pool execution-order and stale-default pubsub reads

Each board's `run.json` groups modules into pools (`wq_spi`, `wq_com`, `wq_slow`, etc.), and `firmware/src/modules/runner/runner_gen.py` generates one RTOS task per pool. Within a pool, `init()` is called on every module in `run.json` array order before that pool's loop starts — ordering *within* a pool is deterministic. But **each pool is spawned as an independent RTOS task**, and all tasks are created before `osal_task_start_scheduler()` runs (`gen_spawn`/`gen_main` in `runner_gen.py`). There is no guarantee that pool A's modules have finished `init()` — or even run once — before pool B's modules start calling `run()`.

This matters because of how `PubSub::Subscriber` behaves (`firmware/src/pubsub/Subscriber.h`): `poll()`/`pollLatest()` return `false` if nothing has been published yet, but `get()` always returns *something* — a default-constructed (`{}`, effectively zero) `m_Data`. That zero often looks like a plausible value (a GPS position of `(0,0,0)`, a pressure of `0`), not a crash or an obvious garbage pattern.

```cpp
// Red flag — get() called without checking poll()'s return
m_GPSSubscriber.poll();
auto pos = m_GPSSubscriber.get();   // if poll() returned false, pos is silently the zero default
useForNavigation(pos);
```

Check, for every `Subscriber<Topic>` member: is every `.get()` call gated by a checked `poll()`/`pollLatest()` return? If a module in a fast/early pool depends on a topic published only by a module in a different pool, is there any guarantee (a required first-publish, a "valid" flag in the message itself) that the zero default can't be mistaken for real data during the startup race?

### 2. Static module-instance construction vs. `hw_init()`

`runner_gen.py`'s `gen_header()` emits every module instance as a **file-scope `static`** object (`static EKFModule EKFModuleInstance;`, with constructor args from `run.json`), all in one generated translation unit, constructed in `run.json` loop/module order. These constructors run during C++ static initialization — **before** `core_main()` calls `hw_init()`:

```cpp
void core_main()
{
    hw_init();       // board pin/bus setup happens here
    start_tasks();   // ...but every module's constructor already ran, before this line
}
```

Any module constructor that calls a HAL function, reads a board config value that depends on `hw_init()` having run, or otherwise touches hardware state is reading before that state is valid. This should never happen — the module system's contract is that constructors only capture config (see `BuzzerModule`'s constructor, which just stores its arguments), and real setup happens in `init()`. Flag any constructor that does more than store its arguments.

### 3. Uninitialized class members

Every private member needs to be set either in the constructor's initializer list or via an in-class default. `BuzzerModule` (`firmware/src/modules/buzzer/BuzzerModule.h`) does this correctly — constructor-supplied config goes in the initializer list, everything else has an inline default (`= NULL`, `= 0`, `= false`):

```cpp
// Correct — every member has a value from somewhere
const hal_pwm_timer_t m_BuzzerTimer;         // set via initializer list
const BuzzerTone *m_CurrentTone = NULL;      // in-class default
uint32_t m_BuzzerToneStartTime = 0;          // in-class default
```

```cpp
// Red flag — no initializer list entry, no in-class default
class FooModule
{
    ...
    uint32_t m_LastUpdateTime;   // read in run() before ever being written — indeterminate value
};
```

Check every member declaration in every module/class header against the constructor's initializer list. A member with neither an in-class default nor a constructor entry starts with indeterminate content, and reads as a plausible-looking (but wrong) number, not an obvious fault.

### 4. Struct layout leaking non-deterministic content into flash/wire data

Two distinct hazards, both about a struct's raw bytes being written or hashed wholesale (`memcpy`, or a CRC over `sizeof(struct)`):

**Padding.** If a struct is *not* `__attribute__((packed))` and any code does `memcpy(dst, &s, sizeof(s))` or CRCs the whole struct, compiler-inserted padding bytes are indeterminate — whatever was previously on the stack or in that static's memory — and get copied or hashed along with the real fields. This project already avoids this in `DatabaseFlashConfig.h` by declaring `DatabaseFrameRaw` and `DatabaseMetadataRaw` `__attribute__((__packed__))` — check that every struct treated this way elsewhere follows the same discipline, and flag any that doesn't.

**Platform-width-dependent fields.** Packing removes padding, but doesn't fix a field whose *width itself* varies by target. `DatabaseMetadata` (`firmware/src/modules/database/DatabaseFlashConfig.h`) is:

```cpp
struct __attribute__((__packed__)) DatabaseMetadata
{
    size_t savedFramesCount;
    size_t standingFramesCount;
};
```

`size_t` is 4 bytes on the RP2040/ESP32 hardware targets, but a host/SITL build (`make obc_sitl`, `make radio_module_sitl`) commonly compiles 64-bit, where `size_t` is 8 bytes. The same struct definition then has a different byte layout depending on which target compiled it — a CRC computed as `sizeof(DatabaseMetadataRaw) - 2` covers a different byte range, and any flash image or wire capture isn't portable between hardware and SITL. Flag any struct written raw to flash or the wire that uses `size_t`, `int`, `long`, or another platform-width-dependent type instead of a fixed-width one (`uint32_t`, `uint64_t`).

## How to work through the code

You have Grep and Read — use them to enumerate every candidate before reasoning about any one of them, rather than reviewing only the file the user pointed at.

1. **Find every `Subscriber<...>` member and its `.get()` call sites** — grep `PubSub::Subscriber<` for declarations, then check each corresponding `.get()` call is preceded by a checked `poll()`/`pollLatest()`.
2. **Find every module constructor** (`grep` for `Module(` definitions) and check the body does nothing but assign arguments to members — anything calling a `hal_`/`hw_`-prefixed function or touching global board state belongs in `init()`, not the constructor.
3. **Check every class/struct member declaration** against the constructor's initializer list and in-class defaults — a member present in neither is a finding.
4. **Find every wholesale struct copy/hash**: grep for `memcpy(.*&|sizeof\(` and for `crc|checksum` call sites, then for each one check: is the struct `__attribute__((packed))`? Does it contain `size_t`, `int`, `long`, or other non-fixed-width types?
5. **Exclude vendored submodules** — `firmware/src/lib/radio/RadioLib`, `firmware/src/modules/oled/u8g2`, `firmware/platform/pico/pico-sdk`, `firmware/platform/common/freertos/FreeRTOS-Kernel` (per CLAUDE.md's submodule list) are third-party code; don't spend audit budget there.
6. **Don't flag false positives** — a `Subscriber` whose topic is guaranteed to be published before any consumer's first tick (e.g. by pool ordering *within* the same pool, with the publisher strictly earlier in `run.json`) is fine; say so rather than flagging it.

## How to structure your findings

```
[SEVERITY] <File>:<Line> — <Category>
  Code: `<the offending line(s)>`
  Risk: <what goes wrong and under what condition>
  Fix:  <concrete replacement or guard to add>
```

**Severity levels:**

- **CRITICAL** — Guaranteed on every boot, not timing-dependent (e.g. a constructor unconditionally calling a HAL function before `hw_init()`).
- **HIGH** — Plausible race depending on RTOS scheduling/priority between pools, or a member read on a common code path before any write.
- **MEDIUM** — Only manifests under a specific condition (e.g. a struct-layout mismatch that only affects host/SITL builds, not real hardware) or a member read on a rare path.
- **LOW** — Missing in-class default or defensive-coding violation with no demonstrated read-before-write path today.

After listing all findings, add:

```
## Summary
- CRITICAL: N  HIGH: N  MEDIUM: N  LOW: N
- Most dangerous area: <module or function>
- Recommended first fix: <the single most urgent change>
```

## Embedded context reminders

- Static allocation means "was this ever validly set" replaces "was this freed" as the core question — there's no allocator to catch the mistake for you.
- `run.json` is the source of truth for pool membership and per-pool module order; read the relevant board's `run.json` before asserting an ordering guarantee holds or doesn't.
- `make audit`'s mechanical check doesn't catch anything here — it only greps for unsafe-construct keywords and checks board-level bus ownership.

## Related skills

- **buffer-safety** — if a `memcpy` is wrong because the *length* is unvalidated (could read/write past a bound), that's buffer-safety's category. This skill's memcpy angle is about the *content* being non-deterministic (padding, platform-width fields), not the size being wrong.
- **integer-overflow** — if a value is wrong because arithmetic on it wrapped or overflowed (not because it was never set), that's integer-overflow's category.
