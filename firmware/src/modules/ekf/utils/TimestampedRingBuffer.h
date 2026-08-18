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
        SYS_CHECK_MSG(!empty(), return m_Buffer[m_TailSeq % N], "Buffer underflow");
        SYS_CHECK_MSG(m_HeadSeq - m_TailSeq <= N, m_TailSeq = m_HeadSeq - N, "Buffer overflow");

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
        SYS_CHECK_MSG(index < size(), index = empty() ? 0 : size() - 1, "Index out of bounds");

        size_t idx = (m_TailSeq + index) % N;

        SYS_ASSERT_MSG(m_TimestampsBuffer[idx] != 0, "No data at index");

        return m_Buffer[idx];
    }

    const T &getNewest() const
    {
        SYS_CHECK(!empty(), return m_Buffer[m_TailSeq % N]);

        return m_Buffer[(m_HeadSeq - 1) % N];
    }

    uint32_t peekTimestamp() const
    {
        SYS_ASSERT(!empty());

        return m_TimestampsBuffer[m_TailSeq % N];
    }

private:
    T m_Buffer[N]{};
    uint32_t m_TimestampsBuffer[N]{};
    size_t m_HeadSeq = 0;
    size_t m_TailSeq = 0;
};