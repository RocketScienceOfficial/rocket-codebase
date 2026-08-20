#pragma once

#include <lib/maths/fast_math.h>
#include <atomic>
#include <cstddef>
#include <cstdint>

namespace PubSub
{
    constexpr size_t MAX_MESSAGE_SIZE = 300u;
    constexpr size_t MAX_MESSAGE_COUNT = 16u;
    constexpr size_t DEFAULT_MESSAGE_COUNT = 2u;

    static_assert(MAX_MESSAGE_COUNT > 1 && FAST_MODULO(MAX_MESSAGE_COUNT, MAX_MESSAGE_COUNT) == 0, "MAX_MESSAGE_COUNT must be a power of 2");
    static_assert(DEFAULT_MESSAGE_COUNT > 1 && FAST_MODULO(DEFAULT_MESSAGE_COUNT, DEFAULT_MESSAGE_COUNT) == 0, "DEFAULT_MESSAGE_COUNT must be a power of 2");

    // NOTE: This must have trivial types so it lands in static initialization
    template <typename T, size_t Depth>
    struct TopicStorage
    {
        static_assert(sizeof(T) <= MAX_MESSAGE_SIZE, "topic message exceeds MAX_MESSAGE_SIZE");
        static_assert(Depth > 1 && Depth <= MAX_MESSAGE_COUNT && FAST_MODULO(Depth, Depth) == 0, "topic depth must be a power of two, greater than 1 and at most MAX_MESSAGE_COUNT");
        static_assert(std::is_trivially_default_constructible_v<T>, "topic message type must be trivially default constructible");

        static constexpr size_t depth = Depth;

        T slots[Depth];
        std::atomic<uint32_t> write_sequence;
        const void *owner;
    };

    template <typename T>
    struct RPCRequestData
    {
        using data_type = T;

        uint8_t src;
        T data;
    };

    struct RPCResponseData
    {
        uint8_t src;
        uint8_t success;
    };
}
