#include <gtest/gtest.h>
#include <cstddef>
#include <cstdint>

// Mirrors the scheduling pattern runner_gen.py's gen_loop() emits for a
// rated loop: each next-due is seeded one period out (never "now", which
// would make the first computed delay zero), and each tick advances to the
// soonest due module before running (and rescheduling) whoever is due.

namespace
{
    struct FakeModule
    {
        int callCount = 0;

        void run() { callCount++; }
    };

    void runFor(FakeModule *modules, uint32_t *nextDue, const uint32_t *periods, size_t count, uint32_t untilMs)
    {
        for (size_t i = 0; i < count; i++)
        {
            nextDue[i] = periods[i];
        }

        uint32_t now = 0;

        while (now < untilMs)
        {
            uint32_t soonest = nextDue[0];
            for (size_t i = 1; i < count; i++)
            {
                if (nextDue[i] < soonest) soonest = nextDue[i];
            }

            now = soonest;

            for (size_t i = 0; i < count; i++)
            {
                if (nextDue[i] <= now)
                {
                    modules[i].run();
                    nextDue[i] += periods[i];
                }
            }
        }
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

    runFor(modules, nextDue, periods, 2, 1000);

    EXPECT_EQ(modules[0].callCount, 500);
    EXPECT_EQ(modules[1].callCount, 100);
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
