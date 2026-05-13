#include <cmath>
#include <cstddef>

template <typename T>
class RunningStats
{
private:
    size_t n = 0;
    T mean = 0;
    T M2 = 0;

public:
    void push(T x)
    {
        n++;

        T delta = x - mean;
        mean += delta / n;
        T delta2 = x - mean;

        M2 += delta * delta2;
    }

    size_t count() const
    {
        return n;
    }

    T getMean() const
    {
        return mean;
    }

    T variance() const
    {
        return (n > 1) ? M2 / (n - 1) : 0.0f;
    }

    T stddev() const
    {
        return sqrt(variance());
    }

    void reset()
    {
        n = 0;
        mean = 0;
        M2 = 0;
    }
};