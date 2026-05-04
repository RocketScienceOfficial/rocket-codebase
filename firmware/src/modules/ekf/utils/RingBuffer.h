#pragma once

#include <cstdint>
#include <lib/debug/sys_assert.h>

template <typename T, size_t N>
class RingBuffer
{
public:
    void push(const T &data, uint32_t timestamp)
    {
        size_t idx = m_HeadSeq % N;

        m_Buffer[idx] = data;
        m_TimestampsBuffer[idx] = timestamp;

        m_HeadSeq++;
    }

    const T &pop()
    {
        SYS_ASSERT_MSG(m_TailSeq < m_HeadSeq, "Buffer underflow");
        SYS_ASSERT_MSG(m_TailSeq >= m_HeadSeq - N, "Buffer overflow");

        size_t idx = m_TailSeq % N;
        SYS_ASSERT_MSG(m_TimestampsBuffer[idx] != 0, "No data at tail");

        m_TailSeq++;

        return m_Buffer[idx];
    }

    bool empty() const
    {
        return m_TailSeq >= m_HeadSeq;
    }

    size_t size() const
    {
        return m_HeadSeq - m_TailSeq;
    }

    uint32_t peekTimestamp() const
    {
        size_t idx = m_TailSeq % N;
        SYS_ASSERT_MSG(m_TimestampsBuffer[idx] != 0, "No data at tail");
        return m_TimestampsBuffer[idx];
    }

private:
    T m_Buffer[N];
    uint32_t m_TimestampsBuffer[N];
    size_t m_HeadSeq;
    size_t m_TailSeq;
};