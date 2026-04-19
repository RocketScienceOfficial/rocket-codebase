#pragma once

#include <cstdint>

namespace PubSub
{
    struct TopicMetadataBase
    {
        const uint16_t size;
        const uint8_t depth;
        const char *name;
    };

    template <typename T>
    struct TopicMetadata : TopicMetadataBase
    {
    };

    using TopicId = const TopicMetadataBase *;

    template <typename T>
    struct RPCRequestData
    {
        uint8_t src;
        T data;
    };

    struct RPCResponseData
    {
        uint8_t src;
        uint8_t success;
    };
}