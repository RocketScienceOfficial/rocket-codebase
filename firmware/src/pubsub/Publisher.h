#pragma once

#include "MessageBus.h"

namespace PubSub
{
    template <typename T>
    class Publisher
    {
    public:
        explicit Publisher(const TopicMetadata<T> *metadata)
        {
            m_Handle = MessageBus::CreateHandle((TopicId)metadata, metadata->name, metadata->size);

            MessageBus::AdvertiseTopic(&m_Handle, metadata->depth > 0 ? (size_t)metadata->depth : DEFAULT_MESSAGE_COUNT);
        }

        void publish(const T &data)
        {
            MessageBus::PublishData(&m_Handle, &data, sizeof(T));
        }

    private:
        TopicHandle m_Handle;
    };
}