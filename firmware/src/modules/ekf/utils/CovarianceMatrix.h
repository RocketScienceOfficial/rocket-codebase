#pragma once

#include <cstddef>

template <size_t N>
class CovarianceMatrix
{
public:
    inline float &operator()(size_t row, size_t col)
    {
        return row <= col ? m_Data[row * N - (row - 1) * row / 2 + col - row] : m_Data[col * N - (col - 1) * col / 2 + row - col];
    }

    inline const float &operator()(size_t row, size_t col) const
    {
        return row <= col ? m_Data[row * N - (row - 1) * row / 2 + col - row] : m_Data[col * N - (col - 1) * col / 2 + row - col];
    }

    void resetCompletely()
    {
        for (size_t i = 0; i < sizeof(m_Data) / sizeof(m_Data[0]); i++)
        {
            m_Data[i] = 0.0f;
        }
    }

private:
    float m_Data[(N + 1) * N / 2];
};