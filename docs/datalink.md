# DataLink Protocol and Code Generation

> A single source of truth for every message exchanged in the system. Messages are defined once in XML and
> code-generated into C, C#, and Python, so firmware, ground apps, and the simulator can never drift out of sync.

---

## 1. Motivation

The stack spans four very different runtimes: embedded C/C++ firmware on RP2040 and ESP32, C# desktop apps in
Unity, and a Python simulation hub. They all need to exchange the same telemetry, command, and logging messages.
Hand-maintaining three parallel definitions of every message is the classic way to ship a silent wire-format bug:
one side adds a field, another forgets, and the bytes no longer line up.

DataLink removes that failure mode by design. Each message exists exactly once, as an XML definition. A generator
turns those definitions into idiomatic code for each language. Changing a field is a one-line edit that propagates
everywhere on the next build.

The protocol is also shaped by the firmware constraints (see [main.md](main.md)):

- **No dynamic allocation.** Every message is a fixed-size value type. The wire payload is capped at 254 bytes.
- **Deterministic layout.** No reflection, no run-time schema negotiation. The byte layout is fixed at generation time.
- **Cheap on an MCU.** Packing and unpacking are a bounded `memcpy` plus a table-driven CRC.

---

## 2. Schema format

Schemas live in [datalink/schemas/](../datalink/schemas/) as XML. Each file is a `<protocol>` containing an
optional `<enums>` block and a `<messages>` block. The generator reads every `*.xml` file in that directory and
merges the results, so messages can be grouped into files by domain (`telemetry.xml`, `database.xml`, `dev.xml`,
and shared definitions in `common.xml`).

### 2.1 Messages

```xml
<message name="telemetry_response" id="2">
    <field name="cmd" type="uint8">Based on telemetry_cmd</field>
    <field name="cmd_seq" type="uint8"></field>
</message>
```

A message has a unique `name`, a unique numeric `id` (the wire identifier), and a list of typed fields. Field text
becomes a code comment. A message may have zero fields (a pure signal, for example `radio_module_tx_done`).

Supported field types and their sizes:

| Type | Bytes | C | C# | Python (`struct`) |
|---|---|---|---|---|
| `uint8` / `int8` | 1 | `uint8_t` / `int8_t` | `byte` / `sbyte` | `B` / `b` |
| `uint16` / `int16` | 2 | `uint16_t` / `int16_t` | `ushort` / `short` | `H` / `h` |
| `uint32` / `int32` | 4 | `uint32_t` / `int32_t` | `uint` / `int` | `I` / `i` |
| `float` | 4 | `float` | `float` | `f` |
| `double` | 8 | `double` | `double` | `d` |

### 2.2 Enums

```xml
<enum name="state_machine_state">
    <value name="SM_STATE_STANDING"/>
    <value name="SM_STATE_ARMED"/>
    ...
</enum>

<enum name="telemetry_data_state_flags" type="flags">
    <value name="TELEMETRY_DATA_CONTROL_3V3_ENABLED"/>
    ...
</enum>
```

A normal enum assigns sequential values (0, 1, 2, ...). A `type="flags"` enum assigns bit positions
(1, 2, 4, 8, ...) so values can be OR-ed into a single field. Generated names are prefixed with `DATALINK_` for
normal enums and `DATALINK_FLAGS_` for flag enums.

---

## 3. The generator

[datalink/gen.py](../datalink/gen.py) is a small, dependency-free Python script. It runs in four stages.

### 3.1 Parse and validate

All schemas are parsed, then validated before any code is emitted. The build fails loudly on:

- duplicate message ids or message names,
- duplicate field names within a message,
- duplicate enum names or duplicate values within an enum,
- a message whose total field size exceeds the 254-byte payload limit.

### 3.2 Field reordering (the key trick)

Before emitting code the generator **sorts each message's fields by size, largest first**. This is what lets the
three languages share an identical byte layout with zero padding:

- The **C** path packs and unpacks with a single `memcpy` of the struct. For that to be correct, the struct must
  have no compiler-inserted padding. Ordering fields from largest to smallest satisfies natural alignment for the
  primitive types used here, so the struct is naturally tight.
- The **C#** and **Python** paths write each field at an explicit byte offset, computed in the same reordered
  sequence.

Because all three follow the same order and the same offsets, the bytes line up exactly.

Example: `telemetry_data_obc` is declared in a human-friendly order (quaternion first, then GPS), but is emitted
in size order:

```
declared:  qw,qx,qy,qz (i16) | velocity,batV,batPct,...    | lat,lon (i32) | alt | ...
emitted:   lat, lon (i32)    | qw,qx,qy,qz,velocity,batV,alt (i16/u16) | seven u8 fields
total:     8 + 8 + 6 + 7  =  29 bytes
```

### 3.3 Render

Each language has a template in [datalink/templates/](../datalink/templates/) that contains a hand-written
runtime (framing, CRC, COBS) plus a `{{{DATA}}}` placeholder. The generator substitutes the generated structs,
pack/unpack routines, and enums into that placeholder and writes the final file (`datalink.h` + `datalink.c`,
`datalink.cs`, or `datalink.py`).

Generation is wired into each component's build, so generated files are never hand-edited. To run it manually:

```bash
python datalink/gen.py --lang c      --outdir <dir>
python datalink/gen.py --lang csharp --outdir <dir>
python datalink/gen.py --lang python --outdir <dir>
```

---

## 4. The in-memory message

Every payload is carried by one fixed-size container:

```c
typedef struct {
    uint8_t msg_id;
    uint8_t len;
    uint8_t payload[254];
} datalink_message_t;
```

For each schema message the generator emits a typed struct plus two functions (C shown):

```c
void datalink_pack_telemetry_response(const telemetry_response *frame, datalink_message_t *message);
int  datalink_unpack_telemetry_response(telemetry_response *frame, const datalink_message_t *message);
```

`pack` stamps `msg_id` and `len` and copies the fields into `payload`. `unpack` checks that the id and length
match the expected message before copying back out, returning an error otherwise. The C# and Python equivalents
are `Pack()` / `Unpack()` methods on a generated class or dataclass.

This struct is the boundary between *what* a message is and *how* it travels. The framing layer below only ever
sees `datalink_message_t`.

---

## 5. Wire framing

The same `datalink_message_t` can be serialized three ways, depending on the transport. All three are
little-endian; the C runtime enforces this with a `static_assert` on `__BYTE_ORDER__`.

### 5.1 Plain

```
[ msg_id ][ len ][ payload (len bytes) ]
```

The minimal form: id, length, payload, nothing else.

### 5.2 Serial (USB / UART)

For raw byte streams there is no packet boundary, so the serial form adds a magic byte, a CRC, and
**Consistent Overhead Byte Stuffing (COBS)** with a `0x00` frame terminator:

```
frame  = [ 0x7E ][ msg_id ][ len ][ payload ][ crc16 ]   (crc over the bytes before it)
on wire = COBS(frame) + 0x00
```

COBS removes every `0x00` byte from the encoded frame, which makes a single `0x00` an unambiguous end-of-frame
marker. A receiver can resynchronize after a glitch simply by scanning to the next zero byte. The CRC is
CRC-16/MCRF4XX (initial value `0xFFFF`, table-driven).

### 5.3 Radio (LoRa)

LoRa delivers discrete packets, so COBS is unnecessary, but the link is shared by three nodes (OBC, Radio Module,
GCS), so the radio form adds addressing and a sequence number:

```
[ 0x5A ][ seq ][ src_id ][ dest_id ][ msg_id ][ len ][ payload ][ crc16 ]
```

`src_id` / `dest_id` let the relay path (OBC to Radio Module to GCS) route and filter frames, and `seq` supports
duplicate detection and link-quality statistics (packet loss is reported back in `telemetry_data_gcs`). Integrity
is again CRC-16/MCRF4XX over everything before the trailing CRC.

| Form | Header | Integrity | Delimiting | Typical transport |
|---|---|---|---|---|
| Plain | id, len | none (transport provides) | length-prefixed | Internal use |
| Serial | 0x7E, id, len | CRC-16 | COBS + `0x00` | USB, UART, TCP |
| Radio | 0x5A, seq, src, dest, id, len | CRC-16 | discrete packet | LoRa |

---

## 6. Cross-language parity and tests

Because the three implementations are generated from one schema and share one byte layout, they are tested
against each other. Each language has a runtime test suite under [datalink/tests/](../datalink/tests/) (`c`,
`csharp`, `python`), and [datalink/run_tests.py](../datalink/run_tests.py) runs them all. These suites exercise
pack/unpack round-trips, CRC, and COBS so that a frame produced by one language decodes correctly in the others.

CI runs the three suites on every push (`datalink-c-tests`, `datalink-csharp-tests`, `datalink-python-tests`).

---

## 7. How it is used in the system

- **Firmware pub/sub.** Communication topics are typed directly as `datalink_message_t` (for example `uart_rx`,
  `serial_tx`, `lora_rx`). The communication modules (`com_serial`, `com_uart`, `com_lora`) move messages
  between these topics and the physical links, applying the serial or radio framing at the edge. See
  [pubsub.md](pubsub.md).
- **Apps.** The Unity telemetry and OBC apps use the generated C# classes (`apps/shared/datalink-unity-utils/`)
  to decode the same frames the firmware sends.
- **Simulator.** The Python hub generates `datalink.py` at build time and speaks the serial form over TCP to the
  firmware SITL process.

## 8. Adding or changing a message

1. Edit the relevant file in [datalink/schemas/](../datalink/schemas/) (add a `<message>` or a `<field>`).
2. Rebuild any component, or run `gen.py` manually. The generated code updates everywhere.
3. Never edit a generated `datalink.*` file by hand; the next build overwrites it.

Because every consumer is regenerated from the same schema, a forgotten field on one side becomes a compile-time
or test-time failure rather than a silent wire-format mismatch in flight.
