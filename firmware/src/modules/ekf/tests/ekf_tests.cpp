#include <gtest/gtest.h>
#include <cmath>
#include <cstdint>
#include <set>

#include "../ekf.h"
#include "../EKFData.h"
#include "../EKFConfig.h"
#include "../utils/CovarianceMatrix.h"
#include "../utils/TimestampedRingBuffer.h"
#include "../utils/RunningStats.h"
#include <lib/geo/physical_constants.h>

// ===================== CovarianceMatrix =====================

TEST(CovarianceMatrix, packed_index_is_unique_in_bounds_and_symmetric)
{
    constexpr size_t N = 5;
    CovarianceMatrix<N> mat;
    mat.resetCompletely();

    const uintptr_t base = reinterpret_cast<uintptr_t>(&mat);
    const uintptr_t end = base + sizeof(mat);

    std::set<const float *> seenAddresses;

    for (size_t i = 0; i < N; i++)
    {
        for (size_t j = i; j < N; j++)
        {
            const float &ref = mat(i, j);
            uintptr_t addr = reinterpret_cast<uintptr_t>(&ref);

            EXPECT_GE(addr, base) << "i=" << i << " j=" << j;
            EXPECT_LT(addr, end) << "i=" << i << " j=" << j;
            EXPECT_TRUE(seenAddresses.insert(&ref).second) << "duplicate index at i=" << i << " j=" << j;
        }
    }

    EXPECT_EQ(seenAddresses.size(), (N + 1) * N / 2);

    // Assign distinct values through the (i <= j) half only.
    for (size_t i = 0; i < N; i++)
    {
        for (size_t j = i; j < N; j++)
        {
            mat(i, j) = static_cast<float>(i * 10 + j);
        }
    }

    // Every (row, col) pair -- including the "mirrored" row > col half -- must alias the same
    // storage as (min(row,col), max(row,col)).
    for (size_t row = 0; row < N; row++)
    {
        for (size_t col = 0; col < N; col++)
        {
            size_t lo = row < col ? row : col;
            size_t hi = row < col ? col : row;
            float expected = static_cast<float>(lo * 10 + hi);

            EXPECT_FLOAT_EQ(mat(row, col), expected) << "row=" << row << " col=" << col;
        }
    }
}

TEST(CovarianceMatrix, reset_completely_zeroes_all_storage)
{
    CovarianceMatrix<4> mat;

    for (size_t i = 0; i < 4; i++)
    {
        for (size_t j = i; j < 4; j++)
        {
            mat(i, j) = 42.0f;
        }
    }

    mat.resetCompletely();

    for (size_t i = 0; i < 4; i++)
    {
        for (size_t j = i; j < 4; j++)
        {
            EXPECT_FLOAT_EQ(mat(i, j), 0.0f) << "i=" << i << " j=" << j;
        }
    }
}

TEST(CovarianceMatrix, out_of_bounds_index_triggers_assert_death)
{
    CovarianceMatrix<5> mat;
    mat.resetCompletely();

    EXPECT_DEATH((void)mat(5, 0), "");
    EXPECT_DEATH((void)mat(0, 5), "");
    EXPECT_DEATH((void)mat(5, 5), "");
}

// ===================== TimestampedRingBuffer =====================

TEST(TimestampedRingBuffer, fifo_order_when_filled_to_capacity)
{
    constexpr size_t N = 4;
    TimestampedRingBuffer<int, N> buf;

    for (int i = 0; i < static_cast<int>(N); i++)
    {
        buf.push(i, static_cast<uint32_t>(100 + i));
    }

    EXPECT_EQ(buf.size(), N);

    for (int i = 0; i < static_cast<int>(N); i++)
    {
        EXPECT_EQ(buf.peekTimestamp(), static_cast<uint32_t>(100 + i));
        EXPECT_EQ(buf.pop(), i);
    }

    EXPECT_TRUE(buf.empty());
}

TEST(TimestampedRingBuffer, steady_state_slide_after_full_wraps_correctly)
{
    constexpr size_t N = 4;
    TimestampedRingBuffer<int, N> buf;

    for (int i = 0; i < static_cast<int>(N); i++)
    {
        buf.push(i, static_cast<uint32_t>(100 + i));
    }

    // Pop one, push one -- this is the realistic runtime pattern (consumer keeps pace) and
    // forces the ring to wrap past index N-1 back to index 0.
    EXPECT_EQ(buf.pop(), 0);
    buf.push(4, 104);

    EXPECT_EQ(buf.size(), N);
    EXPECT_EQ(buf.pop(), 1);
    EXPECT_EQ(buf.pop(), 2);
    EXPECT_EQ(buf.pop(), 3);
    EXPECT_EQ(buf.pop(), 4);
    EXPECT_TRUE(buf.empty());
}

TEST(TimestampedRingBuffer, get_returns_correct_elements_and_is_mutable)
{
    constexpr size_t N = 4;
    TimestampedRingBuffer<int, N> buf;

    buf.push(1, 10);
    buf.push(2, 20);
    buf.push(3, 30);

    EXPECT_EQ(buf.get(0), 1);
    EXPECT_EQ(buf.get(1), 2);
    EXPECT_EQ(buf.get(2), 3);

    buf.get(1) = 99;

    EXPECT_EQ(buf.pop(), 1);
    EXPECT_EQ(buf.pop(), 99);
    EXPECT_EQ(buf.pop(), 3);
}

TEST(TimestampedRingBuffer, getNewest_and_peekTimestamp_track_correct_slot)
{
    constexpr size_t N = 3;
    TimestampedRingBuffer<int, N> buf;

    buf.push(10, 500);
    EXPECT_EQ(buf.getNewest(), 10);
    EXPECT_EQ(buf.peekTimestamp(), 500u);

    buf.push(20, 600);
    EXPECT_EQ(buf.getNewest(), 20);
    EXPECT_EQ(buf.peekTimestamp(), 500u); // oldest unread element is still the first one pushed

    EXPECT_EQ(buf.pop(), 10);
    EXPECT_EQ(buf.peekTimestamp(), 600u);
    EXPECT_EQ(buf.getNewest(), 20);
}

TEST(TimestampedRingBuffer, empty_pop_triggers_assert_death)
{
    TimestampedRingBuffer<int, 4> buf;

    EXPECT_TRUE(buf.empty());
    EXPECT_DEATH((void)buf.pop(), "");
}

TEST(TimestampedRingBuffer, empty_getNewest_and_peekTimestamp_trigger_assert_death)
{
    TimestampedRingBuffer<int, 4> buf;

    EXPECT_DEATH((void)buf.getNewest(), "");
    EXPECT_DEATH((void)buf.peekTimestamp(), "");
}

TEST(TimestampedRingBuffer, get_at_size_index_triggers_assert_death)
{
    TimestampedRingBuffer<int, 4> buf;
    buf.push(42, 1000);

    ASSERT_EQ(buf.size(), 1u);
    EXPECT_DEATH((void)buf.get(1), "");
}

TEST(TimestampedRingBuffer, overrun_beyond_capacity_triggers_assert_on_pop)
{
    // This documents real (assert-guarded, so debug-build-only) behavior: push() itself never
    // blocks or errors when the ring wraps past an un-popped element, so exceeding capacity by
    // even one element is only caught the next time the consumer calls pop()/get()/getNewest().
    constexpr size_t N = 4;
    TimestampedRingBuffer<int, N> buf;

    for (int i = 0; i < static_cast<int>(N); i++)
    {
        buf.push(i, static_cast<uint32_t>(100 + i));
    }

    buf.push(99, 999); // N+1'th push with nothing popped yet -- tail is now lapped by one slot

    EXPECT_DEATH((void)buf.pop(), "");
}

// ===================== RunningStats =====================

TEST(RunningStats, zero_and_one_sample_variance_is_zero)
{
    RunningStats<float> stats;

    EXPECT_EQ(stats.count(), 0u);
    EXPECT_FLOAT_EQ(stats.variance(), 0.0f);
    EXPECT_FLOAT_EQ(stats.stddev(), 0.0f);

    stats.push(42.0f);

    EXPECT_EQ(stats.count(), 1u);
    EXPECT_FLOAT_EQ(stats.getMean(), 42.0f);
    EXPECT_FLOAT_EQ(stats.variance(), 0.0f);
}

TEST(RunningStats, welford_matches_known_dataset)
{
    RunningStats<float> stats;

    for (float v : {2.0f, 4.0f, 4.0f, 4.0f, 5.0f, 5.0f, 7.0f, 9.0f})
    {
        stats.push(v);
    }

    // Textbook Welford example: mean = 5, sample variance (n-1 divisor) = 32/7.
    EXPECT_EQ(stats.count(), 8u);
    EXPECT_NEAR(stats.getMean(), 5.0f, 1e-4f);
    EXPECT_NEAR(stats.variance(), 32.0f / 7.0f, 1e-3f);
    EXPECT_NEAR(stats.stddev(), std::sqrt(32.0f / 7.0f), 1e-3f);
}

TEST(RunningStats, reset_clears_state)
{
    RunningStats<float> stats;

    stats.push(1.0f);
    stats.push(2.0f);
    stats.push(3.0f);
    stats.reset();

    EXPECT_EQ(stats.count(), 0u);

    stats.push(10.0f);

    EXPECT_EQ(stats.count(), 1u);
    EXPECT_FLOAT_EQ(stats.getMean(), 10.0f);
    EXPECT_FLOAT_EQ(stats.variance(), 0.0f);
}

// ===================== EKF =====================

namespace
{
    void zeroNominalState(EKFNominalState &state)
    {
        state.attitude = {1.0f, 0.0f, 0.0f, 0.0f};
        state.pos = {0.0f, 0.0f, 0.0f};
        state.vel = {0.0f, 0.0f, 0.0f};
        state.mag = {0.0f, 0.0f, 0.0f};
        state.bias_gyro = {0.0f, 0.0f, 0.0f};
        state.bias_acc = {0.0f, 0.0f, 0.0f};
        state.bias_mag = {0.0f, 0.0f, 0.0f};
    }

    void setUniformCovariance(EKF &ekf, float value)
    {
        for (size_t i = 0; i < EKF_NUM_ERROR_STATES; i++)
        {
            ekf.getCovarianceElement(i, i) = value;
        }
    }
}

TEST(EKFClass, predict_state_keeps_attitude_normalized)
{
    EKF ekf;
    ekf.init();
    zeroNominalState(ekf.getState());

    EKFIMUData sample{};
    sample.delta_angle = {0.01f, -0.02f, 0.005f};
    sample.delta_velocity = {0.0f, 0.0f, 0.0f};
    sample.varAcc = 0.04f;
    sample.varGyro = 0.03f;
    sample.dt = 0.004f;

    for (int i = 0; i < 50; i++)
    {
        ekf.predictState(sample);
    }

    const EKFNominalState &s = ekf.getState();
    float norm2 = s.attitude.w * s.attitude.w + s.attitude.x * s.attitude.x +
                  s.attitude.y * s.attitude.y + s.attitude.z * s.attitude.z;

    EXPECT_NEAR(norm2, 1.0f, 1e-3f);
}

TEST(EKFClass, predict_state_integrates_constant_acceleration_at_rest)
{
    EKF ekf;
    ekf.init();
    zeroNominalState(ekf.getState());

    // Body-frame delta-velocity that exactly cancels gravity (NED, z-down) should hold a
    // stationary vehicle level: velocity/position stay at zero.
    EKFIMUData sample{};
    sample.delta_angle = {0.0f, 0.0f, 0.0f};
    sample.delta_velocity = {0.0f, 0.0f, static_cast<float>(-EARTH_GRAVITY) * 0.004f};
    sample.varAcc = 0.04f;
    sample.varGyro = 0.03f;
    sample.dt = 0.004f;

    for (int i = 0; i < 100; i++)
    {
        ekf.predictState(sample);
    }

    const EKFNominalState &s = ekf.getState();
    EXPECT_NEAR(s.vel.x, 0.0f, 1e-3f);
    EXPECT_NEAR(s.vel.y, 0.0f, 1e-3f);
    EXPECT_NEAR(s.vel.z, 0.0f, 1e-3f);
    EXPECT_NEAR(s.pos.x, 0.0f, 1e-3f);
    EXPECT_NEAR(s.pos.y, 0.0f, 1e-3f);
    EXPECT_NEAR(s.pos.z, 0.0f, 1e-3f);
}

TEST(EKFClass, covariance_prediction_stays_finite_and_nonnegative)
{
    EKF ekf;
    ekf.init();
    zeroNominalState(ekf.getState());
    setUniformCovariance(ekf, 0.01f);

    EKFIMUData sample{};
    sample.delta_angle = {0.001f, 0.0f, 0.0f};
    sample.delta_velocity = {0.0f, 0.0f, 0.0f};
    sample.varAcc = 0.04f;
    sample.varGyro = 0.03f;
    sample.dt = 0.004f;

    for (int i = 0; i < 10; i++)
    {
        ekf.predictState(sample);
        ekf.predictCovariance(sample);
    }

    for (size_t i = 0; i < EKF_NUM_ERROR_STATES; i++)
    {
        float v = ekf.getCovarianceElement(i, i);
        EXPECT_TRUE(std::isfinite(v)) << "diagonal index " << i;
        EXPECT_GE(v, 0.0f) << "diagonal index " << i;
    }
}

TEST(EKFClass, fuse_baro_within_gate_is_accepted_and_moves_state)
{
    EKF ekf;
    ekf.init();
    zeroNominalState(ekf.getState());
    setUniformCovariance(ekf, 1.0f); // generous uncertainty -> generous gate

    EKFBaroMeasurement meas;
    meas.height = 1.0f;
    meas.var = 1.0f;

    bool accepted = ekf.fuseBaroHeight(meas, EKF_GATE_THRESHOLD_BARO);

    EXPECT_TRUE(accepted);
    EXPECT_GT(std::fabs(ekf.getState().pos.z), 1e-6f) << "state should move toward the measurement";
}

TEST(EKFClass, fuse_baro_outside_gate_is_rejected_and_state_unchanged)
{
    EKF ekf;
    ekf.init();
    zeroNominalState(ekf.getState());
    setUniformCovariance(ekf, 0.0001f); // tight uncertainty -> tight gate

    EKFBaroMeasurement meas;
    meas.height = 1000.0f; // far outside any reasonable gate given the tiny uncertainty
    meas.var = 0.0001f;

    bool accepted = ekf.fuseBaroHeight(meas, EKF_GATE_THRESHOLD_BARO);

    EXPECT_FALSE(accepted);
    EXPECT_FLOAT_EQ(ekf.getState().pos.z, 0.0f);
}

TEST(EKFClass, fuse_gps_position_within_gate_is_accepted)
{
    EKF ekf;
    ekf.init();
    zeroNominalState(ekf.getState());
    setUniformCovariance(ekf, 1.0f);

    EKFGPSPosMeasurement meas;
    meas.pos = {1.0f, 1.0f, 1.0f};
    meas.var_hor = 1.0f;
    meas.var_ver = 1.0f;

    bool accepted = ekf.fuseGPSPosition(meas, EKF_GATE_THRESHOLD_GPS_POS);

    EXPECT_TRUE(accepted);
}

TEST(EKFClass, fuse_gps_position_outside_gate_is_rejected)
{
    EKF ekf;
    ekf.init();
    zeroNominalState(ekf.getState());
    setUniformCovariance(ekf, 0.0001f);

    EKFGPSPosMeasurement meas;
    meas.pos = {10000.0f, 10000.0f, 10000.0f};
    meas.var_hor = 0.0001f;
    meas.var_ver = 0.0001f;

    bool accepted = ekf.fuseGPSPosition(meas, EKF_GATE_THRESHOLD_GPS_POS);

    EXPECT_FALSE(accepted);
}
