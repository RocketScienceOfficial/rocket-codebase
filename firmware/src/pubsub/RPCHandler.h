#pragma once

#include "MessageBus.h"
#include "Subscriber.h"
#include "Publisher.h"
#include <lib/debug/sys_assert.h>

namespace PubSub
{
    template <typename T>
    class RPCHandler
    {
    public:
        explicit RPCHandler(const TopicMetadata<RPCRequestData<T>> *req_meta, const TopicMetadata<RPCResponseData> *res_meta) : m_RequestSubscriber(req_meta), m_ResponsePublisher(res_meta), m_RequestAvailable(false), m_ShouldRespond(false)
        {
        }

        bool requestAvailable()
        {
            m_RequestAvailable = m_RequestSubscriber.poll();

            if (m_RequestAvailable)
            {
                m_ShouldRespond = true;
            }

            return m_RequestAvailable;
        }

        const T &getRequestData() const
        {
            SYS_ASSERT_MSG(m_RequestAvailable, "No RPC request data available");

            return m_RequestSubscriber.get().data;
        }

        void sendResponse(bool success)
        {
            SYS_ASSERT_MSG(m_ShouldRespond, "No RPC request available to respond to");

            m_ResponsePublisher.publish({.src = m_RequestSubscriber.get().src, .success = (uint8_t)(success ? 1 : 0)});
            m_ShouldRespond = false;
        }

    private:
        Subscriber<RPCRequestData<T>> m_RequestSubscriber;
        Publisher<RPCResponseData> m_ResponsePublisher;
        bool m_RequestAvailable;
        bool m_ShouldRespond;
    };
}