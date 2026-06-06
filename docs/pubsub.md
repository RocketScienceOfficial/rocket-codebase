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
- **No dynamic allocation.** All storage is reserved at compile and init time. Nothing is allocated at run time.
- **Lock-free and non-blocking.** A slow subscriber must never block a publisher; a publisher running at 500 Hz
  must never wait on anyone.
- **Determinism.** Fixed topic count, fixed buffer sizes, bounded copy operations.

---

## 2. Topics

A topic is identified at compile time by a `TopicMetadata<T>` instance: a typed record carrying the message
size, a ring depth, and a name.

```cpp
struct TopicMetadataBase {
    const uint16_t size;
    const uint8_t  depth;
    const char    *name;
};

template <typename T> struct TopicMetadata : TopicMetadataBase {};

using TopicId = const TopicMetadataBase *;   // identity is the address of the metadata
```

The topic id is simply the address of its metadata object, so topic identity is resolved by the linker, with no
string lookups or registries at run time. Topics are declared once in
[Topics.h](../firmware/src/pubsub/Topics.h) with two macros:

```cpp
// default ring depth (2 slots)
PUBSUB_REGISTER_TOPIC(SensorsIMU, sensors_imu_1)

// explicit ring depth (must be a power of two)
PUBSUB_REGISTER_TOPIC_SIZE(DatalinkMessage, uart_rx, 4)
```

By convention the message struct is PascalCase (`SensorsIMU`) and the topic instance is snake_case
(`sensors_imu_1`). Communication topics carry `datalink_message_t` directly, so the [DataLink](datalink.md)
wire types and the internal bus share the same structs.

Static limits (from [MessageBus.h](../firmware/src/pubsub/MessageBus.h)):

| Constant | Value | Meaning |
|---|---|---|
| `MAX_TOPICS` | 32 | maximum number of distinct topics |
| `MAX_MESSAGE_SIZE` | 300 | maximum bytes per message |
| `MAX_MESSAGE_COUNT` | 16 | maximum ring depth |
| `DEFAULT_MESSAGE_COUNT` | 2 | ring depth when unspecified |

All message storage comes from a single `32 KB` static buffer. Ring depths are constrained to powers of two so
the slot index is a fast bitwise modulo rather than a division.

---

## 3. The bus

[MessageBus](../firmware/src/pubsub/MessageBus.cpp) owns all state in static storage: the topic table, the 32 KB
payload buffer, and a bump allocator offset. Its lifecycle has two phases.

**Setup phase.** During module construction, each `Publisher` and `Subscriber` calls `CreateHandle`, which finds
or registers the topic, and a `Publisher` additionally calls `AdvertiseTopic`, which reserves the topic's ring of
slots from the 32 KB buffer. `Init()` then freezes the bus: no topic may be created or advertised after that,
which catches accidental run-time registration with an assertion.

**Run phase.** Only `PublishData` and `CopyData` are called, and only on already-registered topics.

```cpp
static void     Init();
static TopicHandle CreateHandle(TopicId id, const char *name, size_t message_size);
static void     AdvertiseTopic(const TopicHandle *handle, size_t message_count);
static void     PublishData(TopicHandle *handle, const void *data, size_t size);
static bool     CopyData(TopicHandle *handle, void *buffer, size_t buffer_size, bool latest);
```

---

## 4. Ring buffer and sequence numbers

Each topic is a single-producer, multiple-consumer ring buffer. The producer side is tracked by one atomic
counter per topic:

```cpp
struct Topic {
    ...
    size_t  memory_offset;             // where this topic's slots live in the 32 KB buffer
    size_t  message_size;
    size_t  message_count;             // ring depth (power of two)
    std::atomic<uint32_t> write_sequence;
};
```

Every handle (held by a `Publisher` or `Subscriber`) carries its own `read_sequence`. The monotonic
`write_sequence` only ever increases; the slot for any sequence number is `sequence & (message_count - 1)`.

**Publishing** copies the message into the current write slot and then advances the sequence:

```cpp
size_t offset = topic->memory_offset + (write_sequence % message_count) * message_size;
memcpy(buffer + offset, data, size);
uint32_t old = write_sequence.fetch_add(1, memory_order_release);
```

The payload is written before the sequence is advanced with release ordering, so a consumer that observes the new
sequence (with acquire ordering) is guaranteed to see the fully written payload.

**Subscribing** compares the handle's `read_sequence` against the topic's `write_sequence`:

- `poll()` reads the next unread message in order.
- `pollLatest()` jumps straight to the newest message and discards anything older, which is what most fast-loop
  consumers want (the EKF cares about the latest IMU sample, not a backlog).

A single producer per topic means writes never race each other. Only the producer-to-consumer handoff needs
synchronization, and that is exactly what the acquire/release pair on `write_sequence` provides. No mutexes are
involved anywhere.

---

## 5. Overrun and torn-read handling

Because the publisher never blocks, a subscriber that falls behind by more than the ring depth will have its
oldest unread slots overwritten. This is detected, not ignored:

- In an ordered `poll()`, if `write_sequence - read_sequence` exceeds the ring depth, the bus raises an assertion
  in debug builds ("subscriber is too slow") and recovers by skipping the read cursor forward to the oldest slot
  still present. The publisher is never affected.
- After copying a slot, `CopyData` re-reads `write_sequence` and retries if the producer lapped the buffer during
  the copy. This seqlock-style check guarantees a consumer never returns a torn message that was half-overwritten
  mid-read.

The practical guidance is to size a topic's ring depth for its slowest consumer's jitter, or to use
`pollLatest()` when only the freshest value matters.

---

## 6. Public API

Modules never touch `MessageBus` directly. They use two thin templates.

```cpp
// Publisher: owns the topic, reserves its ring on construction.
PubSub::Publisher<SensorsIMU> imuPub{PUBSUB_ID(sensors_imu_1)};
imuPub.publish(sample);

// Subscriber: holds its own read cursor and a local copy of the last message.
PubSub::Subscriber<SensorsIMU> imuSub{PUBSUB_ID(sensors_imu_1)};
if (imuSub.pollLatest()) {
    const SensorsIMU &s = imuSub.get();
    // ... use s ...
}
```

`Subscriber::get()` returns a reference to the copy made by the last successful `poll`/`pollLatest`, so the data
stays valid and stable for the rest of the module's tick even if the publisher writes again.

---

## 7. RPC layer

Some interactions are request/response rather than streaming: arm the igniter, fire a channel, enable a power
rail. These are layered on top of two ordinary topics, a request topic and a response topic, using the wrapper
types in [PubSubMeta.h](../firmware/src/pubsub/internal/PubSubMeta.h):

```cpp
template <typename T> struct RPCRequestData { uint8_t src; T data; };
struct RPCResponseData { uint8_t src; uint8_t success; };
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

Topics are static, so the bus is fully formed before the scheduler starts. The runner (see [main.md](main.md))
drives modules in fixed-rate pools (`fast` at 500 Hz, `com` at 200 Hz, `slow` at 100 Hz). Each module's `run()`
polls its subscriptions, does its work, and publishes its outputs, all within its time slot. Because no call
blocks, a module in the 100 Hz pool can never stall the 500 Hz pool, and the only coupling between any two
modules is the data on a topic.

This is what makes the firmware portable and testable: a module is a pure function of the topics it reads and
writes, and in SITL the same modules run unchanged against simulated inputs.
