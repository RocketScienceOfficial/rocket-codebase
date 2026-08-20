#pragma once

#include "internal/PubSubMeta.h"
#include <lib/debug/sys_assert.h>

namespace PubSub
{
    struct NoOpRetryHook
    {
        static void afterCopy() { (void)0; }
    };

    struct DefaultTooSlowHook
    {
        static void onTooSlow(const char *topicName, uint32_t readSeq, uint32_t writeSeq)
        {
            SYS_ASSERT_MSG(false, "Subscriber is too slow and has missed messages on topic '%s' (read_sequence: %u, write_sequence: %u)", topicName, readSeq, writeSeq);
        }
    };

    template <typename Topic, typename RetryHook = NoOpRetryHook, typename TooSlowHook = DefaultTooSlowHook>
    class Subscriber
    {
    public:
        bool poll()
        {
            return copyData(false);
        }

        bool pollLatest()
        {
            return copyData(true);
        }

        const typename Topic::message_type &get() const
        {
            return m_Data;
        }

    private:
        typename Topic::message_type m_Data{};
        uint32_t m_ReadSequence = 0;

        bool copyData(bool latest)
        {
            auto &s = Topic::storage;
            static constexpr size_t depth = s.depth;

            uint32_t write_seq = 0;

            do
            {
                write_seq = s.write_sequence.load(std::memory_order_acquire);

                if (m_ReadSequence == write_seq)
                {
                    return false;
                }

                if (latest)
                {
                    m_ReadSequence = write_seq - 1;
                }
                else
                {
                    if (write_seq - m_ReadSequence > depth)
                    {
                        TooSlowHook::onTooSlow(Topic::topic_name, m_ReadSequence, write_seq);

                        m_ReadSequence = write_seq - depth + 1;
                    }
                }

                m_Data = s.slots[FAST_MODULO(m_ReadSequence, depth)];
                RetryHook::afterCopy();

            } while (s.write_sequence.load(std::memory_order_acquire) - m_ReadSequence > depth);

            m_ReadSequence++;

            return true;
        }
    };
}