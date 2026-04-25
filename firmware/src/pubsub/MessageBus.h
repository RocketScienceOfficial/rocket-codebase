#pragma once

#include "internal/PubSubMeta.h"
#include <lib/maths/fast_math.h>
#include <cstddef>
#include <cstdint>

namespace PubSub
{
    constexpr size_t MAX_TOPICS = 32u;
    constexpr size_t MAX_MESSAGE_SIZE = 300u;
    constexpr size_t MAX_MESSAGE_COUNT = 16u;
    constexpr size_t DEFAULT_MESSAGE_COUNT = 2u;

    static_assert(MAX_MESSAGE_COUNT > 1 && FAST_MODULO(MAX_MESSAGE_COUNT, MAX_MESSAGE_COUNT) == 0, "MAX_MESSAGE_COUNT must be a power of 2");
    static_assert(DEFAULT_MESSAGE_COUNT > 1 && FAST_MODULO(DEFAULT_MESSAGE_COUNT, DEFAULT_MESSAGE_COUNT) == 0, "DEFAULT_MESSAGE_COUNT must be a power of 2");

    struct TopicHandle
    {
        uint8_t topic_index;
        uint32_t read_sequence;
    };

    class MessageBus
    {
    public:
        static void Init();
        static TopicHandle CreateHandle(TopicId id, const char *topic_name, size_t message_size);
        static void AdvertiseTopic(const TopicHandle *handle, size_t message_count);
        static void PublishData(TopicHandle *handle, const void *data, size_t size);
        static bool CopyData(TopicHandle *handle, void *buffer, size_t buffer_size, bool latest);
    };
}