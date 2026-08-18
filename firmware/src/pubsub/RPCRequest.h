#pragma once

#include "Subscriber.h"
#include "Publisher.h"
#include <lib/debug/sys_assert.h>

namespace PubSub
{
    template <typename ReqTopic, typename ResTopic>
    class RPCRequest
    {
    public:
        void call(const typename ReqTopic::message_type::data_type &requestData, uint8_t src)
        {
            m_RequestPublisher.publish({.src = src, .data = requestData});
        }

        bool finished()
        {
            m_ResponseAvailable = m_ResponseSubscriber.poll();

            return m_ResponseAvailable;
        }

        bool isSuccess() const
        {
            SYS_ASSERT_MSG(m_ResponseAvailable, "No RPC response data available");

            return m_ResponseSubscriber.get().success != 0;
        }

        uint8_t getResponseSource() const
        {
            SYS_ASSERT_MSG(m_ResponseAvailable, "No RPC response data available");

            return m_ResponseSubscriber.get().src;
        }

    private:
        Publisher<ReqTopic> m_RequestPublisher;
        Subscriber<ResTopic> m_ResponseSubscriber;
        bool m_ResponseAvailable = false;
    };
}