#include "ign_test_harness.h"

TEST(IgnitersModule, MalfunctionSpeedFiresBackup)
{
    IgnHarness h;

    h.smState = DATALINK_SM_STATE_FREE_FLIGHT;
    h.setFlight(1000.0f, 50.0f);
    EXPECT_EQ(h.stepCountingFiredUpdates(20), 0);

    {
        SCOPED_TRACE("Phase A: apogee fires the pilot igniter");

        h.smState = DATALINK_SM_STATE_FREE_FALL;
        h.setFlight(1000.0f, -1.0f);
        h.step();

        ASSERT_TRUE(h.firedUpdated);
        h.expectFired(true, false, false, false);

        h.expectPulse(0, false);
    }

    {
        SCOPED_TRACE("Phase B: backup fires at the malfunction speed, well above main altitude");

        h.setFlight(950.0f, -19.9f);
        EXPECT_EQ(h.stepCountingFiredUpdates(20), 0) << "backup must not fire below MALFUNCTION_SPEED (20 m/s)";

        h.setFlight(940.0f, -20.0f);
        h.step();

        ASSERT_TRUE(h.firedUpdated) << "backup must fire at exactly MALFUNCTION_SPEED";
        h.expectFired(true, true, false, false);

        h.expectPulse(1, false);

        EXPECT_EQ(h.stepCountingFiredUpdates(100), 0) << "backup must not re-fire";
    }
}
