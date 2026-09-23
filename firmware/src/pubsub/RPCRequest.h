#pragma once

#include "Subscriber.h"
#include "Publisher.h"
#include <lib/debug/sys_assert.h>

namespace PubSub
{
    // TODO: Add timeout support
    template <typename ReqTopic, typename ResTopic>
    class RPCRequest
    {
    public:
        bool call(const typename ReqTopic::message_type::data_type &requestData, uint8_t src)
        {
            // We enforce that only one call can be made at a time, as we don't have a way to match responses to requests
            if (m_DuringCall)
            {
                return false;
            }

            m_RequestPublisher.publish({.src = src, .data = requestData});
            m_DuringCall = true;

            return true;
        }

        bool finished()
        {
            m_ResponseAvailable = m_ResponseSubscriber.poll();

            if (m_ResponseAvailable)
            {
                m_DuringCall = false;
            }

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
        bool m_DuringCall = false;
    };
}