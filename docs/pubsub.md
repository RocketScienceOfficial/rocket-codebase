# Publish / Subscribe Message Bus

> The only way firmware modules talk to each other. A lock-free, statically allocated topic bus with one
> producer and many consumers per topic, built for hard real-time use with no heap and no mutexes.

---

## 1. Motivation and design goals

The firmware is a set of independent modules (`sensors`, `ekf`, `state_machine`, `ign`, `telemetry`, and so on,
see [main.md](main.md)). If they called each other directly, the system would become a tangle of dependencies
that is impossible to test or reason about, and one slow module could stall another.

The pub/sub bus enforces the opposite: modules never reference each other. A module declares the topics it
publishes and the topics it subscribes to, and that is the entire contract. The design goals are:

- **Decoupling.** Producers and consumers know only a topic, never each other.
- **No dynamic allocation.** All storage is reserved at compile time, as static data on each topic's type.
  Nothing is allocated at run time.
- **Lock-free and non-blocking.** A slow subscriber must never block a publisher; a publisher running at 500 Hz
  must never wait on anyone.
- **Determinism.** Fixed per-topic buffer sizes, bounded copy operations.

---

## 2. Topics

Each topic is declared once in [Topics.h](../firmware/src/pubsub/Topics.h) with one of two macros (defined in
[PubSubRegister.h](../firmware/src/pubsub/internal/PubSubRegister.h)):

```cpp
// default ring depth (2 slots)
PUBSUB_REGISTER_TOPIC(Messages::SensorsIMU, sensors_imu_1)

// explicit ring depth (must be a power of two)
PUBSUB_REGISTER_TOPIC_SIZE(Messages::DatalinkMessage, uart_rx, 4)
```

Each call expands to a distinct struct type, `<name>_topic`, carrying a `message_type` alias, a `topic_name`
string, and its own private static storage:

```cpp
struct sensors_imu_1_topic
{
    typedef Messages::SensorsIMU message_type;
    static inline const char *topic_name = "sensors_imu_1";
private:
    static inline PubSub::TopicStorage<Messages::SensorsIMU, PubSub::DEFAULT_MESSAGE_COUNT> storage;
};
```

**Topic identity is the C++ type itself** — there's no runtime registry, no lookup table, and nothing to
resolve at link time. `Publisher<T>`/`Subscriber<T>` (see [§6](#6-public-api)) template directly on that type
and reach its `storage` member.

By convention the message struct is PascalCase (`SensorsIMU`) and the topic instance is snake_case
(`sensors_imu_1`). Communication topics carry `datalink_message_t` directly (aliased as
`Messages::DatalinkMessage`), so the [DataLink](datalink.md) wire types and the internal bus share the same
structs.

Static limits (from [PubSubMeta.h](../firmware/src/pubsub/internal/PubSubMeta.h)):

| Constant | Value | Meaning |
|---|---|---|
| `MAX_MESSAGE_SIZE` | 300 | maximum bytes per message |
| `MAX_MESSAGE_COUNT` | 16 | maximum ring depth |
| `DEFAULT_MESSAGE_COUNT` | 2 | ring depth when unspecified |

Ring depths are constrained to powers of two (enforced by `static_assert`) so the slot index is a fast bitwise
AND (`FAST_MODULO`, [fast_math.h](../firmware/src/lib/maths/fast_math.h)) rather than a division. There's no
shared buffer or arena to size — each topic's storage (`T slots[depth]`) is its own `static inline` array,
sized only for that one topic. No central pool, no maximum topic count to run out of.

---

## 3. There is no bus object

Earlier versions of this design centralized everything in a `MessageBus` singleton (a topic table, a shared
payload arena, a two-phase setup/freeze lifecycle). That's gone. Today each topic *is* its storage — a
`static inline PubSub::TopicStorage<T, Depth>` member sitting directly on the topic struct from
[§2](#2-topics) — so there is nothing to register, advertise, or freeze. Storage exists, zero-initialized,
before `main()`/`core_main()` even runs.

The only thing enforced at runtime is single-producer ownership, and it happens in the `Publisher` constructor
itself ([Publisher.h](../firmware/src/pubsub/Publisher.h)):

```cpp
Publisher()
{
    auto &s = Topic::storage;
    SYS_ASSERT_MSG(s.owner == nullptr, "Topic '%s' is already owned by another publisher", Topic::topic_name);
    s.owner = this;
}
```

The first `Publisher<Topic>` constructed for a given topic claims it; a second one trips the assert. A
`Subscriber<Topic>` does nothing at construction beyond zero-initializing its own read cursor — there's no
equivalent check on the read side, since the ring buffer is designed for multiple consumers.

---

## 4. Ring buffer and sequence numbers

Each topic's `TopicStorage<T, Depth>` ([PubSubMeta.h](../firmware/src/pubsub/internal/PubSubMeta.h)) is a
single-producer, multiple-consumer ring buffer:

```cpp
template <typename T, size_t Depth>
struct TopicStorage
{
    T slots[Depth];
    std::atomic<uint32_t> write_sequence;
    const void *owner;
};
```

Every `Subscriber` instance carries its own `m_ReadSequence`. The monotonic `write_sequence` only ever
increases; the slot for any sequence number is `FAST_MODULO(sequence, Depth)` — a plain bitwise AND, since
`Depth` is enforced to be a power of two.

**Publishing** ([Publisher.h](../firmware/src/pubsub/Publisher.h)) writes into the current slot and then
advances the sequence:

```cpp
const uint32_t seq = s.write_sequence.load(std::memory_order_relaxed);
s.slots[FAST_MODULO(seq, s.depth)] = data;
s.write_sequence.store(seq + 1, std::memory_order_release);
```

The payload is written before the sequence is advanced with release ordering, so a consumer that observes the
new sequence (with acquire ordering) is guaranteed to see the fully written slot. The load of the current
sequence only needs `relaxed` ordering — with a single enforced producer, nothing else ever writes it.

**Subscribing** ([Subscriber.h](../firmware/src/pubsub/Subscriber.h)) compares `m_ReadSequence` against the
topic's `write_sequence`:

- `poll()` reads the next unread message in order.
- `pollLatest()` jumps straight to the newest message (`m_ReadSequence = write_seq - 1`) and discards anything
  older, which is what most fast-loop consumers want (the EKF cares about the latest IMU sample, not a
  backlog).

A single producer per topic (enforced as described in [§3](#3-there-is-no-bus-object)) means writes never race
each other. Only the producer-to-consumer handoff needs synchronization, and that is exactly what the
acquire/release pair on `write_sequence` provides. No mutexes are involved anywhere.

---

## 5. Overrun and torn-read handling

Because the publisher never blocks, a subscriber that falls behind by more than the ring depth will have its
oldest unread slots overwritten. This is detected, not ignored, by two injectable hook types on
`Subscriber<Topic, RetryHook, TooSlowHook>` (both take safe defaults in normal use):

- In an ordered `poll()`, if `write_sequence - m_ReadSequence` exceeds the ring depth, `TooSlowHook::onTooSlow`
  runs (the default, `DefaultTooSlowHook`, raises `SYS_ASSERT_MSG`) and the read cursor is skipped forward to
  the oldest slot still present (`write_seq - depth + 1`). The publisher is never affected.
- After copying a slot, `RetryHook::afterCopy` runs (a no-op by default), and the loop re-checks
  `write_sequence` against `m_ReadSequence`; if the producer lapped the buffer during the copy, it retries.
  This seqlock-style check guarantees a consumer never returns a torn message that was half-overwritten
  mid-read.

Both hooks exist for testing ([message_bus_tests.cpp](../firmware/src/pubsub/tests/message_bus_tests.cpp)) —
they let a test simulate a slow subscriber or inject a write between the copy and the retry check without
tripping a real assertion.

The practical guidance is to size a topic's ring depth for its slowest consumer's jitter, or to use
`pollLatest()` when only the freshest value matters.

---

## 6. Public API

Modules use two thin templates, both parameterized directly on the **topic type** — not the message type —
via the `PUBSUB_ID(name)` macro, which expands to `PubSub::Topics::<name>_topic`:

```cpp
// Publisher: claims ownership of the topic on construction (see §3). No constructor argument.
PubSub::Publisher<PUBSUB_ID(sensors_imu_1)> imuPub;
imuPub.publish(sample);

// Subscriber: holds its own read cursor and a local copy of the last message.
PubSub::Subscriber<PUBSUB_ID(sensors_imu_1)> imuSub;
if (imuSub.pollLatest()) {
    const auto &s = imuSub.get();  // Messages::SensorsIMU
    // ... use s ...
}
```

Equivalently, `PubSub::Publisher<PubSub::Topics::sensors_imu_1_topic>` — `PUBSUB_ID` is just a shorter spelling
of the same type. `Subscriber::get()` returns a reference to the copy made by the last successful
`poll`/`pollLatest`, so the data stays valid and stable for the rest of the module's tick even if the publisher
writes again.

---

## 7. RPC layer

Some interactions are request/response rather than streaming: arm the igniter, fire a channel, enable a power
rail. These are layered on top of two ordinary topics, a request topic and a response topic, using the wrapper
types in [PubSubMeta.h](../firmware/src/pubsub/internal/PubSubMeta.h):

```cpp
template <typename T> struct RPCRequestData { uint8_t src; T data; };
struct RPCResponseData { uint8_t src; uint8_t success; };
```

A pair of topics is declared in one shot with `PUBSUB_REGISTER_RPC(T, name)` (also in
[PubSubRegister.h](../firmware/src/pubsub/internal/PubSubRegister.h)), producing `<name>_req_topic` and
`<name>_res_topic`. `PUBSUB_RPC_ID(name)` expands to both, comma-separated, as the two template arguments
`RPCRequest`/`RPCHandler` take:

```cpp
PubSub::RPCRequest<PUBSUB_RPC_ID(command_arm)> m_ArmRequest;   // caller side
PubSub::RPCHandler<PUBSUB_RPC_ID(command_arm)> m_ArmHandler;   // responder side
```

The caller uses [RPCRequest](../firmware/src/pubsub/RPCRequest.h) and the responder uses
[RPCHandler](../firmware/src/pubsub/RPCHandler.h). Both are non-blocking and polled, so they fit the fixed-rate
run loop:

```cpp
// Caller
rpc.call(payload, myId);          // publishes the request, tagged with a source id
if (rpc.finished() && rpc.isSuccess()) { ... }   // polled in a later tick

// Responder
if (handler.requestAvailable()) {
    auto &req = handler.getRequestData();
    handler.sendResponse(/* success = */ true);   // echoes the caller's src id back
}
```

The `src` id is carried through the request and echoed in the response, which lets several callers share one RPC
topic pair and still match each response to the caller that made it. Examples in the system are `command_arm`,
`command_ignite`, and `command_set_voltage`.

---

## 8. Integration with the scheduler

Topic storage is static, so every topic is fully formed before the scheduler starts — there's no
initialization order to get right. The runner (see [main.md](main.md) and [hal_boards.md](hal_boards.md))
drives modules in per-board execution pools, each mapped to one RTOS task at its own priority, with each
module in a pool free to run at its own configured rate. Each module's `run()` polls its subscriptions, does
its work, and publishes its outputs, all within its time slot. Because no call blocks, a module in a low-rate
pool can never stall a high-rate one, and the only coupling between any two modules is the data on a topic.

This is what makes the firmware portable and testable: a module is a pure function of the topics it reads and
writes, and in SITL the same modules run unchanged against simulated inputs.
