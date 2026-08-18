#pragma once

#include "Subscriber.h"
#include "Publisher.h"
#include <lib/debug/sys_assert.h>

namespace PubSub
{
    template <typename ReqTopic, typename ResTopic>
    class RPCHandler
    {
    public:
        bool requestAvailable()
        {
            m_RequestAvailable = m_RequestSubscriber.poll();

            if (m_RequestAvailable)
            {
                m_ShouldRespond = true;
            }

            return m_RequestAvailable;
        }

        const typename ReqTopic::message_type::data_type &getRequestData() const
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
        Subscriber<ReqTopic> m_RequestSubscriber;
        Publisher<ResTopic> m_ResponsePublisher;
        bool m_RequestAvailable = false;
        bool m_ShouldRespond = false;
    };
}
