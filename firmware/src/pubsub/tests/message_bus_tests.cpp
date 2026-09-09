#include <gtest/gtest.h>
#include <atomic>
#include <cstdint>
#include <thread>

#include <pubsub/Publisher.h>
#include <pubsub/Subscriber.h>
#include <pubsub/RPCHandler.h>
#include <pubsub/RPCRequest.h>
#include <pubsub/Topics.h>

namespace
{
    struct RoundTripMsg
    {
        int value;
        float extra;
    };
    PUBSUB_REGISTER_TOPIC_SIZE(RoundTripMsg, test_round_trip, 4)
}

TEST(MessageBus, publish_then_poll_round_trip)
{
    PubSub::Publisher<test_round_trip_topic> pub;
    PubSub::Subscriber<test_round_trip_topic> sub;

    EXPECT_FALSE(sub.poll());

    pub.publish({42, 3.5f});

    ASSERT_TRUE(sub.poll());
    EXPECT_EQ(sub.get().value, 42);
    EXPECT_FLOAT_EQ(sub.get().extra, 3.5f);

    EXPECT_FALSE(sub.poll());
}

namespace
{
    struct EmptyPollMsg
    {
        int value;
    };
    PUBSUB_REGISTER_TOPIC_SIZE(EmptyPollMsg, test_empty_poll, 4)
}

TEST(MessageBus, poll_with_no_new_data_leaves_last_value_untouched)
{
    PubSub::Publisher<test_empty_poll_topic> pub;
    PubSub::Subscriber<test_empty_poll_topic> sub;

    pub.publish({7});
    ASSERT_TRUE(sub.poll());
    EXPECT_EQ(sub.get().value, 7);

    EXPECT_FALSE(sub.poll());
    EXPECT_EQ(sub.get().value, 7);

    EXPECT_FALSE(sub.poll());
    EXPECT_EQ(sub.get().value, 7);
}

namespace
{
    struct LatestMsg
    {
        int seq;
    };
    PUBSUB_REGISTER_TOPIC_SIZE(LatestMsg, test_latest, 4)
}

TEST(MessageBus, poll_latest_returns_newest_and_catches_up)
{
    PubSub::Publisher<test_latest_topic> pub;
    PubSub::Subscriber<test_latest_topic> sub;

    for (int i = 0; i < 3; i++)
    {
        pub.publish({i});
    }

    ASSERT_TRUE(sub.pollLatest());
    EXPECT_EQ(sub.get().seq, 2);

    EXPECT_FALSE(sub.poll());
    EXPECT_FALSE(sub.pollLatest());

    pub.publish({99});

    ASSERT_TRUE(sub.pollLatest());
    EXPECT_EQ(sub.get().seq, 99);
}

namespace
{
    struct SlowMsg
    {
        int seq;
    };
    PUBSUB_REGISTER_TOPIC_SIZE(SlowMsg, test_too_slow, 4)
}

TEST(MessageBusDeathTest, poll_when_lapped_by_slow_subscriber_asserts)
{
    PubSub::Publisher<test_too_slow_topic> pub;
    PubSub::Subscriber<test_too_slow_topic> sub;

    for (int i = 0; i < 5; i++) // depth is 4; one extra publish laps the reader
    {
        pub.publish({i});
    }

    EXPECT_DEATH(sub.poll(), "");
}

namespace
{
    struct DoubleOwnerMsg
    {
        int value;
    };
    PUBSUB_REGISTER_TOPIC_SIZE(DoubleOwnerMsg, test_double_owner, 4)
}

TEST(MessageBusDeathTest, second_publisher_construction_asserts)
{
    PubSub::Publisher<test_double_owner_topic> pub1;

    EXPECT_DEATH({ PubSub::Publisher<test_double_owner_topic> pub2; }, "");
}

namespace
{
    struct RpcTestRequest
    {
        int32_t requestValue;
    };
    PUBSUB_REGISTER_RPC(RpcTestRequest, test_rpc)
}

TEST(MessageBusRPC, request_handler_round_trip)
{
    PubSub::RPCHandler<test_rpc_req_topic, test_rpc_res_topic> handler;
    PubSub::RPCRequest<test_rpc_req_topic, test_rpc_res_topic> requester;

    EXPECT_FALSE(handler.requestAvailable());

    requester.call({123}, /*src=*/7);

    ASSERT_TRUE(handler.requestAvailable());
    EXPECT_EQ(handler.getRequestData().requestValue, 123);

    handler.sendResponse(true);

    ASSERT_TRUE(requester.finished());
    EXPECT_TRUE(requester.isSuccess());
    EXPECT_EQ(requester.getResponseSource(), 7);
}

namespace
{
    struct TornReadMsg
    {
        int seq;
    };
    PUBSUB_REGISTER_TOPIC_SIZE(TornReadMsg, test_torn_read, 4)

    // Simulates a writer lapping the reader mid-copy: fired from inside Subscriber::copyData right
    // after it copies a slot, before the post-copy re-check of write_sequence sees it. One-shot so
    // the second (retry) iteration of copyData doesn't re-arm itself.
    struct LapDuringCopyHook
    {
        inline static PubSub::Publisher<test_torn_read_topic> *pub = nullptr;

        static void afterCopy()
        {
            if (pub == nullptr)
            {
                return;
            }

            // depth is 4: 4 more publishes puts write_sequence 5 ahead of the read this raced with,
            // tripping the "> depth" re-check below.
            for (int i = 2; i <= 5; i++)
            {
                pub->publish({i});
            }
            pub = nullptr;
        }
    };
}

TEST(MessageBus, poll_latest_retries_when_lapped_between_copy_and_recheck)
{
    PubSub::Publisher<test_torn_read_topic> pub;
    PubSub::Subscriber<test_torn_read_topic, LapDuringCopyHook> sub;

    pub.publish({1});
    LapDuringCopyHook::pub = &pub;

    ASSERT_TRUE(sub.pollLatest());
    EXPECT_EQ(sub.get().seq, 5);
}

namespace
{
    struct ResyncMsg
    {
        int seq;
    };
    PUBSUB_REGISTER_TOPIC_SIZE(ResyncMsg, test_resync, 4)

    // In a release (NDEBUG) build SYS_ASSERT_MSG is a no-op, so the "too slow" branch in copyData
    // falls straight through to the resync line below it. This hook skips the assert the same way,
    // so that fallthrough can be observed without aborting this Debug test binary.
    struct SkipTooSlowAssertHook
    {
        static void onTooSlow(const char *, uint32_t, uint32_t) { (void)0; }
    };
}

TEST(MessageBus, poll_resyncs_to_oldest_valid_slot_after_being_lapped)
{
    PubSub::Publisher<test_resync_topic> pub;
    PubSub::Subscriber<test_resync_topic, PubSub::NoOpRetryHook, SkipTooSlowAssertHook> sub;

    for (int i = 0; i < 5; i++) // depth is 4; one extra publish laps the reader
    {
        pub.publish({i});
    }

    ASSERT_TRUE(sub.poll());
    EXPECT_EQ(sub.get().seq, 2); // write_seq(5) - depth(4) + 1, not the reader's original position

    // Confirms the subscriber didn't just recover once but is back to reading forward normally.
    ASSERT_TRUE(sub.poll());
    EXPECT_EQ(sub.get().seq, 3);
}

namespace
{
    struct BoundaryLatestMsg
    {
        int seq;
    };
    PUBSUB_REGISTER_TOPIC_SIZE(BoundaryLatestMsg, test_boundary_latest, 4)

    // Publishes exactly depth-1 messages from inside copyData, right after the slot copy. That
    // leaves write_sequence exactly `depth` ahead of the sequence just read, which is the point
    // where the writer's *next* slot is the one the reader has just copied out of. The data is
    // not overwritten yet, but write_sequence does not move until a publish completes, so the
    // reader cannot tell an idle writer apart from one that is halfway through that slot.
    struct LapToExactBoundaryHook
    {
        inline static PubSub::Publisher<test_boundary_latest_topic> *pub = nullptr;

        static void afterCopy()
        {
            if (pub == nullptr)
            {
                return;
            }

            for (int i = 2; i <= 4; i++)
            {
                pub->publish({i});
            }
            pub = nullptr;
        }
    };
}

TEST(MessageBus, poll_latest_retries_when_writer_lands_exactly_depth_ahead)
{
    PubSub::Publisher<test_boundary_latest_topic> pub;
    PubSub::Subscriber<test_boundary_latest_topic, LapToExactBoundaryHook> sub;

    pub.publish({1});
    LapToExactBoundaryHook::pub = &pub;

    ASSERT_TRUE(sub.pollLatest());

    // Not 1: the copy of seq 1 came out of the slot the writer is about to reuse, so it must be
    // retried rather than returned. A `> depth` re-check would accept it.
    EXPECT_EQ(sub.get().seq, 4);
}

namespace
{
    struct BoundarySlowMsg
    {
        int seq;
    };
    PUBSUB_REGISTER_TOPIC_SIZE(BoundarySlowMsg, test_boundary_slow, 4)

    struct CountingTooSlowHook
    {
        inline static int calls = 0;

        static void onTooSlow(const char *, uint32_t, uint32_t) { calls++; }
    };
}

TEST(MessageBus, poll_treats_writer_exactly_depth_ahead_as_a_lap)
{
    PubSub::Publisher<test_boundary_slow_topic> pub;
    PubSub::Subscriber<test_boundary_slow_topic, PubSub::NoOpRetryHook, CountingTooSlowHook> sub;

    CountingTooSlowHook::calls = 0;

    for (int i = 0; i < 4; i++) // exactly depth publishes: the reader sits depth behind
    {
        pub.publish({i});
    }

    ASSERT_TRUE(sub.poll());

    // Seq 0 is still intact in its slot at this instant, but it is the writer's next target and
    // an in-flight write is invisible, so the usable backlog is depth-1 rather than depth. This
    // is the cost of a single sequence counter; it must be paired with the same comparison in
    // the post-copy re-check, or a reader parked here retries forever against an idle writer.
    EXPECT_EQ(CountingTooSlowHook::calls, 1);
    EXPECT_EQ(sub.get().seq, 1); // write_seq(4) - depth(4) + 1
}

namespace
{
    // Every word carries the same tag, so any blend of two publishes is detectable.
    struct TearProbe
    {
        uint32_t word[16];
    };
    PUBSUB_REGISTER_TOPIC_SIZE(TearProbe, test_concurrent_tear, 2)
}

TEST(MessageBusConcurrency, concurrent_publisher_never_yields_a_torn_message)
{
    // The single-threaded hooks above can only lap the reader between whole publishes. This one
    // runs a real writer alongside the reader so the copy can overlap a slot write in flight,
    // which is the only way the torn read actually happens. Reverting either comparison in
    // Subscriber.h to `>` produces on the order of a thousand torn reads here.
    PubSub::Publisher<test_concurrent_tear_topic> pub;
    PubSub::Subscriber<test_concurrent_tear_topic> sub;

    constexpr uint32_t PUBLISH_COUNT = 400000;

    std::atomic<bool> writerDone{false};

    std::thread writer([&]
    {
        for (uint32_t seq = 1; seq <= PUBLISH_COUNT; seq++)
        {
            TearProbe probe;

            for (uint32_t &word : probe.word)
            {
                word = seq;
            }

            pub.publish(probe);
        }

        writerDone.store(true, std::memory_order_release);
    });

    unsigned long reads = 0;
    unsigned long torn = 0;

    while (!writerDone.load(std::memory_order_acquire))
    {
        if (!sub.pollLatest())
        {
            continue;
        }

        const TearProbe &probe = sub.get();
        reads++;

        for (uint32_t word : probe.word)
        {
            if (word != probe.word[0])
            {
                torn++;
                break;
            }
        }
    }

    writer.join();

    EXPECT_EQ(torn, 0ul);
    EXPECT_GT(reads, 1000ul); // guards against the reader never observing anything at all
}
