#pragma once

#include "MessageBus.h"

namespace PubSub
{
    template <typename T>
    class Subscriber
    {
    public:
        explicit Subscriber(const TopicMetadata<T> *metadata)
        {
            m_Handle = MessageBus::CreateHandle((TopicId)metadata, metadata->name, metadata->size);
        }

        bool poll()
        {
            return MessageBus::CopyData(&m_Handle, &m_Data, sizeof(T), false);
        }

        bool pollLatest()
        {
            return MessageBus::CopyData(&m_Handle, &m_Data, sizeof(T), true);
        }

        const T &get() const
        {
            return m_Data;
        }

    private:
        TopicHandle m_Handle;
        T m_Data;
    };
}