#pragma once

#include "internal/PubSubMeta.h"
#include <lib/debug/sys_assert.h>

namespace PubSub
{
    template <typename Topic>
    class Publisher
    {
    public:
        Publisher()
        {
            auto &s = Topic::storage;

            SYS_ASSERT_MSG(s.owner == nullptr, "Topic '%s' is already owned by another publisher", Topic::topic_name);

            s.owner = this;
        }

        void publish(const typename Topic::message_type &data)
        {
            auto &s = Topic::storage;

            SYS_ASSERT_MSG(s.owner == this, "Publisher does not own topic '%s'", Topic::topic_name);

            const uint32_t seq = s.write_sequence.load(std::memory_order_relaxed);

            s.slots[FAST_MODULO(seq, s.depth)] = data;
            s.write_sequence.store(seq + 1, std::memory_order_release);
        }
    };
}