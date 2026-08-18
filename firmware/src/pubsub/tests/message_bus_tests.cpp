#include <gtest/gtest.h>
#include <cstdint>

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
