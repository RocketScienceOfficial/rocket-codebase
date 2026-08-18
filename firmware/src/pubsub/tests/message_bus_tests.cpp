#include <gtest/gtest.h>
#include <cstdint>

#include <pubsub/MessageBus.h>
#include <pubsub/Publisher.h>
#include <pubsub/Subscriber.h>
#include <pubsub/RPCHandler.h>
#include <pubsub/RPCRequest.h>
#include <pubsub/Topics.h>

// sys_assert_handler() (firmware/src/lib/debug/sys_assert.c) calls abort(). This test build runs
// CMAKE_BUILD_TYPE=Debug, so SYS_ASSERT is live and invalid usage below genuinely aborts, which is
// what EXPECT_DEATH expects. In a release (NDEBUG) build these asserts are no-ops. hal_stdio_printf
// writes to stdout not stderr, so EXPECT_DEATH matches "" rather than the assert text.

// Never reuse a Topics.h production topic across tests: Publisher<T>/AdvertiseTopic enforces single
// ownership per topic id, and topic identity is the TopicMetadata's pointer.

namespace
{
    struct RoundTripMsg
    {
        int value;
        float extra;
    };
    inline constexpr PubSub::TopicMetadata<RoundTripMsg> g_roundTripTopic{sizeof(RoundTripMsg), 4, "test_round_trip"};
}

TEST(MessageBus, PublishThenPollRoundTrip)
{
    PubSub::Publisher<RoundTripMsg> pub(&g_roundTripTopic);
    PubSub::Subscriber<RoundTripMsg> sub(&g_roundTripTopic);

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
    inline constexpr PubSub::TopicMetadata<EmptyPollMsg> g_emptyPollTopic{sizeof(EmptyPollMsg), 4, "test_empty_poll"};
}

TEST(MessageBus, PollWithNoNewDataLeavesLastValueUntouched)
{
    PubSub::Publisher<EmptyPollMsg> pub(&g_emptyPollTopic);
    PubSub::Subscriber<EmptyPollMsg> sub(&g_emptyPollTopic);

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
    inline constexpr PubSub::TopicMetadata<LatestMsg> g_latestTopic{sizeof(LatestMsg), 4, "test_latest"};
}

TEST(MessageBus, PollLatestReturnsNewestAndCatchesUp)
{
    PubSub::Publisher<LatestMsg> pub(&g_latestTopic);
    PubSub::Subscriber<LatestMsg> sub(&g_latestTopic);

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
    inline constexpr PubSub::TopicMetadata<SlowMsg> g_slowTopic{sizeof(SlowMsg), 4, "test_too_slow"};
}

TEST(MessageBusDeathTest, PollWhenLappedBySlowSubscriberAsserts)
{
    PubSub::Publisher<SlowMsg> pub(&g_slowTopic);
    PubSub::Subscriber<SlowMsg> sub(&g_slowTopic);

    for (int i = 0; i < 5; i++) // message_count is 4; one extra publish laps the reader
    {
        pub.publish({i});
    }

    EXPECT_DEATH(sub.poll(), "");
}

namespace
{
    struct MismatchMsg
    {
        int value;
    };
}

TEST(MessageBusDeathTest, CreateHandleWithMismatchedMessageSizeAsserts)
{
    static const PubSub::TopicMetadataBase meta{sizeof(MismatchMsg), 4, "test_mismatched_size"};
    PubSub::TopicId id = &meta;

    PubSub::MessageBus::CreateHandle(id, "test_mismatched_size", sizeof(MismatchMsg));

    EXPECT_DEATH(
        PubSub::MessageBus::CreateHandle(id, "test_mismatched_size", sizeof(MismatchMsg) + 4),
        "");
}

namespace
{
    struct DoubleOwnerMsg
    {
        int value;
    };
}

TEST(MessageBusDeathTest, AdvertiseTopicSecondOwnerAsserts)
{
    static const PubSub::TopicMetadataBase meta{sizeof(DoubleOwnerMsg), 4, "test_double_owner"};
    PubSub::TopicId id = &meta;

    PubSub::TopicHandle h1 = PubSub::MessageBus::CreateHandle(id, "test_double_owner", sizeof(DoubleOwnerMsg));
    PubSub::MessageBus::AdvertiseTopic(&h1, 4);

    PubSub::TopicHandle h2 = PubSub::MessageBus::CreateHandle(id, "test_double_owner", sizeof(DoubleOwnerMsg));

    EXPECT_DEATH(PubSub::MessageBus::AdvertiseTopic(&h2, 4), "");
}

namespace
{
    struct BadDepthMsg
    {
        int value;
    };
}

TEST(MessageBusDeathTest, AdvertiseTopicNonPowerOfTwoCountAsserts)
{
    static const PubSub::TopicMetadataBase meta{sizeof(BadDepthMsg), 3, "test_bad_depth"};
    PubSub::TopicId id = &meta;

    PubSub::TopicHandle h = PubSub::MessageBus::CreateHandle(id, "test_bad_depth", sizeof(BadDepthMsg));

    EXPECT_DEATH(PubSub::MessageBus::AdvertiseTopic(&h, 3), "");
}

namespace
{
    struct RpcTestRequest
    {
        int32_t requestValue;
    };

    inline constexpr PubSub::TopicMetadata<PubSub::RPCRequestData<RpcTestRequest>> g_rpcReqTopic{
        sizeof(PubSub::RPCRequestData<RpcTestRequest>), 4, "test_rpc_req"};
    inline constexpr PubSub::TopicMetadata<PubSub::RPCResponseData> g_rpcResTopic{
        sizeof(PubSub::RPCResponseData), 4, "test_rpc_res"};
}

TEST(MessageBusRPC, RequestHandlerRoundTrip)
{
    PubSub::RPCHandler<RpcTestRequest> handler(&g_rpcReqTopic, &g_rpcResTopic);
    PubSub::RPCRequest<RpcTestRequest> requester(&g_rpcReqTopic, &g_rpcResTopic);

    EXPECT_FALSE(handler.requestAvailable());

    requester.call({123}, /*src=*/7);

    ASSERT_TRUE(handler.requestAvailable());
    EXPECT_EQ(handler.getRequestData().requestValue, 123);

    handler.sendResponse(true);

    ASSERT_TRUE(requester.finished());
    EXPECT_TRUE(requester.isSuccess());
    EXPECT_EQ(requester.getResponseSource(), 7);
}

// CopyData's torn-read retry loop is not covered: exercising it needs a publish to land between the
// reader's memcpy and its post-copy re-check of write_sequence, which requires real concurrent
// execution with no test hook available in MessageBus as currently written.
