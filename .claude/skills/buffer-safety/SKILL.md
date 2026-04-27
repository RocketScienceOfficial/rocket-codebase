---
name: buffer-safety
description: >
  Systematically audits C/C++ code for buffer overflow vulnerabilities and memory safety issues
  in embedded/firmware contexts. Use this skill whenever the user asks to review, audit, or check
  C/C++ code for safety — including requests like "check for buffer overflows", "is this safe?",
  "review this firmware code", "look for memory issues", "audit this module", "any unsafe patterns
  here?", or when touching code that handles arrays, strings, buffers, or pointer arithmetic. Also
  use proactively when editing firmware code that contains fixed-size arrays, C string operations,
  or raw pointer math — even if the user hasn't explicitly asked for a safety check.
---

# Buffer Safety Audit

You are performing a security-focused code review for buffer overflow and memory safety issues in C/C++ embedded firmware. The target environment uses **statically allocated memory only** — no heap, no dynamic allocation — so all buffers are fixed-size arrays, stack variables, or global/static storage. This shapes what matters: the danger is always overstepping a fixed boundary, never a heap corruption issue.

## What to look for

Work through the code systematically. For each category below, find every instance — don't stop at the first hit.

### 1. Unsafe C string functions

These functions trust the caller to have sized the destination correctly and will silently overflow if not:

| Dangerous | Safer replacement |
|-----------|-------------------|
| `strcpy(dst, src)` | `strncpy(dst, src, sizeof(dst) - 1)` + null-terminate |
| `strcat(dst, src)` | `strncat(dst, src, sizeof(dst) - strlen(dst) - 1)` |
| `sprintf(buf, fmt, ...)` | `snprintf(buf, sizeof(buf), fmt, ...)` |
| `gets(buf)` | `fgets(buf, sizeof(buf), stdin)` |
| `scanf("%s", buf)` | `scanf("%Ns", buf)` with explicit width |
| `vsprintf` | `vsnprintf` |

Flag every use. Even `strncpy` needs checking — it doesn't null-terminate if `src` is longer than `n`.

### 2. Array access with unchecked indices

Any array subscript `arr[i]` where `i` comes from external input, a loop, arithmetic, or a cast without a prior bounds check is suspect:

```c
// Red flag — i is untrusted
void process(uint8_t *data, size_t len) {
    buf[data[0]] = 1;  // data[0] could be 0..255; buf may be smaller
}
```

Look for:
- Indices derived from received packets, sensor reads, or user input
- Loops where the upper bound comes from a length field in external data
- Indexing with a signed integer (negative wraps to a huge unsigned)

### 3. Integer overflow leading to under-sized buffers or out-of-bounds access

Integer arithmetic that feeds a length or index can wrap silently:

```c
uint8_t len = recv_len + header_size;  // overflows if sum > 255
memcpy(buf, src, len);                 // copies the wrong amount
```

Watch for:
- Narrowing casts (`int → uint8_t`, `size_t → uint16_t`) on values used as lengths
- Addition/multiplication of user-controlled values without overflow checks
- Signed/unsigned mismatch in comparisons (`if (len < MAX)` where `len` is signed)

### 4. Off-by-one errors

The most common source of subtle overflow:

```c
char buf[16];
for (int i = 0; i <= 16; i++) buf[i] = 0;  // writes buf[16] — one past the end
strncpy(buf, src, 16);                       // no room for null terminator
```

Check:
- Loop bounds using `<=` vs `<` against array size
- `strncpy` / `memcpy` calls where the count equals `sizeof(buf)` with no null-terminator room
- "Last element" accesses like `arr[SIZE - 1]` — verify SIZE is the declared length

### 5. `memcpy` / `memmove` / `memset` with unvalidated lengths

```c
memcpy(dst, src, rx_length);  // if rx_length > sizeof(dst), overflow
```

Every `mem*` call needs either a static size (`sizeof(buf)`) or an explicit `if (len > sizeof(buf)) return;` guard before it.

### 6. Pointer arithmetic

Out-of-bounds pointer arithmetic is undefined behavior and corrupts adjacent memory:

```c
uint8_t *p = buf;
p += offset;       // offset must be checked: 0 <= offset < sizeof(buf)
*p = value;        // write through unchecked pointer
```

Look for pointer increment/decrement without bounds verification, especially in parsers or protocol decoders.

### 7. Format string vulnerabilities

```c
printf(user_string);         // user_string is the format — exploitable
snprintf(buf, n, user_str);  // same problem
```

The format argument must always be a string literal or a controlled format string. Flag any call where the format string is a variable.

### 8. Stack depth on embedded targets

This is embedded firmware — the stack is fixed and small (often 1–4 KB per RTOS task). Flag:
- Large local arrays or structs (`char tmp[512]` inside a function that lives on the call stack)
- Deep recursion
- `alloca()` or VLAs (variable-length arrays)

---

## How to structure your findings

For each issue found, report it in this exact format so the findings are easy to act on:

```
[SEVERITY] <File>:<Line> — <Category>
  Code: `<the offending line(s)>`
  Risk: <what goes wrong and under what condition>
  Fix:  <concrete replacement or guard to add>
```

**Severity levels:**

- **CRITICAL** — Direct overflow with attacker-controlled input or no guard at all. Must fix before any hardware test or field use.
- **HIGH** — Overflow possible under plausible conditions (e.g., unexpected message length, sensor out of range).
- **MEDIUM** — Overflow requires specific edge case; unlikely in practice but violates defensive coding principles.
- **LOW** — Code smell or potential concern; no realistic overflow path identified but worth cleaning up.

After listing all findings, add a short **Summary** section:

```
## Summary
- CRITICAL: N  HIGH: N  MEDIUM: N  LOW: N
- Most dangerous area: <module or function>
- Recommended first fix: <the single most urgent change>
```

---

## How to work through the code

1. **Read the full file or function first** — don't flag things piecemeal without understanding context. A bounds check three lines earlier can render a later access safe.
2. **Trace data flow** — follow untrusted data (packet fields, sensor values, external input) from where it enters to where it's used as a length, index, or copy size. The vulnerability is often not where the data arrives but where it's used two calls later.
3. **Don't flag false positives** — if a bound check genuinely exists and is correct, say so. Credibility matters: a report with 20 false alarms gets ignored.
4. **Suggest concrete fixes** — generic advice like "add bounds checking" is unhelpful. Write the actual guard or replacement call, including the correct size argument.
5. **Note what was checked** — end the report with one line like "Reviewed: `module_foo.c` lines 1–200, `module_foo.h`" so the user knows what the audit covered.

---

## Embedded context reminders

- No heap means no `malloc`/`free` — if you see them, flag separately as a policy violation, not just a safety issue.
- FreeRTOS task stacks are declared statically; note if a function's local frame might exceed typical task stack sizes.
- Circular buffer implementations (common in pub/sub systems) must verify that read/write indices are masked or range-checked on every access, not just on initialization.
- DMA transfers and hardware peripherals can write into buffers from interrupt context — check that buffer sizes account for the maximum peripheral transfer size, not just the typical case.
