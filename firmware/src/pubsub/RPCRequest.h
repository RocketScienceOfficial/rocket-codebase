#pragma once

#include "MessageBus.h"
#include "Subscriber.h"
#include "Publisher.h"
#include <lib/debug/sys_assert.h>

namespace PubSub
{
    template <typename T>
    class RPCRequest
    {
    public:
        explicit RPCRequest(const TopicMetadata<RPCRequestData<T>> *req_meta, const TopicMetadata<RPCResponseData> *res_meta) : m_RequestPublisher(req_meta), m_ResponseSubscriber(res_meta), m_ResponseAvailable(false)
        {
        }

        void call(const T &requestData, uint8_t src)
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
        Publisher<RPCRequestData<T>> m_RequestPublisher;
        Subscriber<RPCResponseData> m_ResponseSubscriber;
        bool m_ResponseAvailable;
    };
}