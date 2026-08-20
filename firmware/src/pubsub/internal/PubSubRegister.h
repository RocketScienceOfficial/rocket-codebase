#pragma once

#include "PubSubMeta.h"

namespace PubSub
{
    template <typename Topic>
    class Publisher;

    template <typename Topic, typename RetryHook, typename TooSlowHook>
    class Subscriber;
}

#define PUBSUB_REGISTER_TOPIC_SIZE(T, name, depth)            \
    struct name##_topic                                       \
    {                                                         \
        typedef T message_type;                               \
                                                              \
        static inline const char *topic_name = #name;         \
                                                              \
        template <typename, typename, typename>               \
        friend class PubSub::Subscriber;                      \
        template <typename>                                   \
        friend class PubSub::Publisher;                       \
                                                              \
    private:                                                  \
        static inline PubSub::TopicStorage<T, depth> storage; \
    };

#define PUBSUB_REGISTER_TOPIC(T, name) \
    PUBSUB_REGISTER_TOPIC_SIZE(T, name, PubSub::DEFAULT_MESSAGE_COUNT)

#define PUBSUB_REGISTER_RPC(T, name)                                                                 \
    PUBSUB_REGISTER_TOPIC_SIZE(PubSub::RPCRequestData<T>, name##_req, PubSub::DEFAULT_MESSAGE_COUNT) \
    PUBSUB_REGISTER_TOPIC_SIZE(PubSub::RPCResponseData, name##_res, PubSub::DEFAULT_MESSAGE_COUNT)
