#pragma once

#include <cstdint>
#include <lib/debug/sys_assert.h>

template <typename T, size_t N>
class TimestampedRingBuffer
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
        SYS_ASSERT(!empty());
        SYS_ASSERT_MSG(m_TailSeq < m_HeadSeq, "Buffer underflow");
        SYS_ASSERT_MSG(m_TailSeq + N >= m_HeadSeq, "Buffer overflow");

        size_t idx = m_TailSeq % N;

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

    T &get(size_t index)
    {
        SYS_ASSERT_MSG(index < size(), "Index out of bounds");

        size_t idx = (m_TailSeq + index) % N;

        SYS_ASSERT_MSG(m_TimestampsBuffer[idx] != 0, "No data at index");

        return m_Buffer[idx];
    }

    const T &getNewest() const
    {
        SYS_ASSERT(!empty());

        return m_Buffer[(m_HeadSeq - 1) % N];
    }

    uint32_t peekTimestamp() const
    {
        SYS_ASSERT(!empty());

        return m_TimestampsBuffer[m_TailSeq % N];
    }

private:
    T m_Buffer[N];
    uint32_t m_TimestampsBuffer[N];
    size_t m_HeadSeq;
    size_t m_TailSeq;
};