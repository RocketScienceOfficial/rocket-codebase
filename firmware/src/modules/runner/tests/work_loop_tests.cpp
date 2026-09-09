#include <gtest/gtest.h>
#include <cstddef>
#include <cstdint>

// Mirrors the scheduling pattern runner_gen.py's gen_loop() emits for a rated
// loop: each next-due is seeded one period out (never "now", which would make the
// first computed delay zero), the loop blocks on osal_task_delay_until() until the
// soonest deadline, then runs every module that has come due.
//
// The clock is virtual, so a module can be given a run() cost and the loop can be
// started near the uint32 millisecond wrap. Both are needed to cover the ways the
// real loop can fail: a module that overruns its period, a deadline comparison that
// straddles the rollover, and the non-blocking return from osal_task_delay_until()
// when a deadline has already passed.

namespace
{
    // Copies of the helpers gen_loop() emits above the loop functions.
    bool time_reached(uint32_t now, uint32_t deadline)
    {
        return (int32_t)(now - deadline) >= 0;
    }

    bool time_before_or_equal(uint32_t a, uint32_t b)
    {
        return (int32_t)(a - b) <= 0;
    }

    uint32_t schedule_next(uint32_t due, uint32_t period, uint32_t now)
    {
        due += period;

        if (time_reached(now, due))
        {
            due = now + period;
        }

        return due;
    }

    struct FakeModule
    {
        uint32_t cost = 0; // virtual milliseconds consumed by one run()
        int callCount = 0;
    };

    struct LoopResult
    {
        uint32_t elapsed;
        unsigned long iterations;
        unsigned long nonBlockingWaits; // passes where the deadline had already gone
    };

    // Bounded so a loop that fails to make progress fails the test instead of hanging it.
    constexpr unsigned long ITERATION_LIMIT = 1000000u;

    LoopResult runFor(FakeModule *modules, uint32_t *nextDue, const uint32_t *periods, size_t count, uint32_t horizonMs, uint32_t startMs = 0)
    {
        uint32_t clock = startMs;
        unsigned long iterations = 0;
        unsigned long nonBlockingWaits = 0;

        for (size_t i = 0; i < count; i++)
        {
            nextDue[i] = startMs + periods[i];
        }

        while ((uint32_t)(clock - startMs) < horizonMs)
        {
            if (++iterations > ITERATION_LIMIT)
            {
                break;
            }

            uint32_t soonest = nextDue[0];

            for (size_t i = 1; i < count; i++)
            {
                if (time_before_or_equal(nextDue[i], soonest)) soonest = nextDue[i];
            }

            // Models osal_task_delay_until(): an absolute deadline that returns immediately
            // without blocking when it has already passed.
            if (time_reached(clock, soonest))
            {
                nonBlockingWaits++;
            }
            else
            {
                clock = soonest;
            }

            for (size_t i = 0; i < count; i++)
            {
                const uint32_t now = clock;

                if (time_reached(now, nextDue[i]))
                {
                    modules[i].callCount++;
                    clock += modules[i].cost;
                    nextDue[i] = schedule_next(nextDue[i], periods[i], clock);
                }
            }
        }

        return {(uint32_t)(clock - startMs), iterations, nonBlockingWaits};
    }
}

TEST(WorkQueueLoop, first_deadline_is_never_immediate)
{
    const uint32_t periods[2] = {2, 10};
    uint32_t nextDue[2] = {periods[0], periods[1]};

    uint32_t soonest = nextDue[0] < nextDue[1] ? nextDue[0] : nextDue[1];

    EXPECT_GT(soonest, 0u);
    EXPECT_EQ(soonest, 2u);
}

TEST(WorkQueueLoop, faster_module_runs_proportionally_more_often)
{
    FakeModule modules[2];
    const uint32_t periods[2] = {2, 10};
    uint32_t nextDue[2];

    const LoopResult result = runFor(modules, nextDue, periods, 2, 1000);

    EXPECT_EQ(modules[0].callCount, 500);
    EXPECT_EQ(modules[1].callCount, 100);

    // A healthy pool always has its next deadline in the future.
    EXPECT_EQ(result.nonBlockingWaits, 0ul);
}

TEST(WorkQueueLoop, no_drift_over_many_periods)
{
    FakeModule modules[1];
    const uint32_t periods[1] = {3};
    uint32_t nextDue[1];

    runFor(modules, nextDue, periods, 1, 3000);

    EXPECT_EQ(modules[0].callCount, 1000);
}

TEST(WorkQueueLoop, simultaneous_deadlines_both_fire_on_same_wake)
{
    FakeModule modules[2];
    const uint32_t periods[2] = {5, 5};
    uint32_t nextDue[2];

    runFor(modules, nextDue, periods, 2, 100);

    EXPECT_EQ(modules[0].callCount, modules[1].callCount);
    EXPECT_EQ(modules[0].callCount, 20);
}

TEST(WorkQueueLoop, overrunning_module_drops_missed_ticks_instead_of_catching_up)
{
    FakeModule modules[2];
    const uint32_t periods[2] = {2, 10};
    uint32_t nextDue[2];

    modules[0].cost = 5; // five milliseconds of work on a two millisecond period

    runFor(modules, nextDue, periods, 2, 1000);

    // It runs as often as it actually fits, not the 500 its rate claims, and never accumulates a
    // backlog it tries to replay.
    EXPECT_EQ(modules[0].callCount, 143);

    // The well-behaved module in the same pool keeps its full rate.
    EXPECT_EQ(modules[1].callCount, 100);
}

TEST(WorkQueueLoop, a_slow_module_pushes_its_neighbour_past_its_deadline)
{
    FakeModule modules[2];
    const uint32_t periods[2] = {2, 10};
    uint32_t nextDue[2];

    modules[1].cost = 50; // blows well past the 2 ms module's period

    const LoopResult result = runFor(modules, nextDue, periods, 2, 1000);

    // This is the case that reaches osal_task_delay_until() with a deadline already gone, so the
    // call returns without blocking and the overdue module runs straight away.
    EXPECT_GT(result.nonBlockingWaits, 0ul);

    // Not blocking must not mean spinning: every such pass runs at least one module and pushes it
    // forward, so the loop still makes progress and the iteration count stays bounded.
    EXPECT_LE(result.iterations, 1000ul);
    EXPECT_GE(result.elapsed, 1000u);
}

TEST(WorkQueueLoop, deadlines_survive_millisecond_rollover)
{
    FakeModule modules[2];
    const uint32_t periods[2] = {2, 10};
    uint32_t nextDue[2];

    // Starts 500 ms before the uint32 millisecond counter wraps, so every deadline in the run
    // straddles it.
    const LoopResult result = runFor(modules, nextDue, periods, 2, 1000, UINT32_MAX - 500);

    EXPECT_EQ(modules[0].callCount, 500);
    EXPECT_EQ(modules[1].callCount, 100);
    EXPECT_LE(result.iterations, 1000ul);
}
