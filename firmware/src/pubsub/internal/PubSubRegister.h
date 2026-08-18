#pragma once

#include "PubSubMeta.h"

// ==============================================================================

#define PUBSUB_TOPIC_STORAGE(name) name##_storage

#define PUBSUB_REGISTER_TOPIC_SIZE(T, name, depth)                          \
    inline PubSub::TopicStorage<T, depth> PUBSUB_TOPIC_STORAGE(name);       \
    struct name##_topic                                                     \
    {                                                                       \
        typedef T message_type;                                             \
        typedef PubSub::TopicStorage<T, depth> storage_type;                \
                                                                            \
        static storage_type &store() { return PUBSUB_TOPIC_STORAGE(name); } \
        static const char *topic_name() { return #name; }                   \
    };

#define PUBSUB_REGISTER_TOPIC(T, name) \
    PUBSUB_REGISTER_TOPIC_SIZE(T, name, PubSub::DEFAULT_MESSAGE_COUNT)

// ==============================================================================

#define PUBSUB_REGISTER_RPC(T, name)                                                                          \
    inline PubSub::TopicStorage<PubSub::RPCRequestData<T>, PubSub::DEFAULT_MESSAGE_COUNT> name##_req_storage; \
    struct name##_req_topic                                                                                   \
    {                                                                                                         \
        typedef PubSub::RPCRequestData<T> message_type;                                                       \
        typedef PubSub::TopicStorage<PubSub::RPCRequestData<T>, PubSub::DEFAULT_MESSAGE_COUNT> storage_type;  \
                                                                                                              \
        static storage_type &store() { return name##_req_storage; }                                           \
        static const char *topic_name() { return "req_" #name; }                                              \
    };                                                                                                        \
    inline PubSub::TopicStorage<PubSub::RPCResponseData, PubSub::DEFAULT_MESSAGE_COUNT> name##_res_storage;   \
    struct name##_res_topic                                                                                   \
    {                                                                                                         \
        typedef PubSub::RPCResponseData message_type;                                                         \
        typedef PubSub::TopicStorage<PubSub::RPCResponseData, PubSub::DEFAULT_MESSAGE_COUNT> storage_type;    \
                                                                                                              \
        static storage_type &store() { return name##_res_storage; }                                           \
        static const char *topic_name() { return "res_" #name; }                                              \
    };
