#include "ign_test_harness.h"

TEST(IgnitersModule, GroundTestsThenNominalFlight)
{
    IgnHarness h;

    auto groundTest = [&](uint8_t channel)
    {
        const uint8_t idx = channel - 1;

        ASSERT_TRUE(h.ignRpc.call({.channel = channel}, 0));
        h.step();

        ASSERT_TRUE(h.firedUpdated) << "test fire must publish ign_fired on the tick it fires";
        EXPECT_TRUE(h.fired(idx));
        EXPECT_FALSE(h.ignRpc.finished()) << "test fire must not respond before the pulse ends";

        h.expectPulse(idx, true);

        ASSERT_TRUE(h.ignRpc.finished()) << "test fire must respond on the tick the pulse ends";
        EXPECT_TRUE(h.ignRpc.isSuccess());
        ASSERT_TRUE(h.firedUpdated) << "end of a test fire must publish ign_fired";
        EXPECT_FALSE(h.fired(idx)) << "a test fire must clear its fired flag so the channel stays usable in flight";
    };

    auto expectRejected = [&](uint8_t channel, const char *msg)
    {
        ASSERT_TRUE(h.ignRpc.call({.channel = channel}, 0));
        h.step();

        ASSERT_TRUE(h.ignRpc.finished()) << msg << " (no response)";
        EXPECT_FALSE(h.ignRpc.isSuccess()) << msg;
        EXPECT_FALSE(h.firedUpdated) << msg << " (ign_fired was published)";

        for (uint8_t i = 0; i < IGN_COUNT; i++)
        {
            EXPECT_EQ(h.cont(i), CONT_OK) << msg << " (channel " << i + 1 << " is being driven)";
        }
    };

    {
        SCOPED_TRACE("Phase A: idle on the pad");

        EXPECT_EQ(h.stepCountingFiredUpdates(100), 0) << "nothing may fire while idle in STANDING";
        ASSERT_TRUE(h.contUpdated);

        for (uint8_t i = 0; i < IGN_COUNT; i++)
        {
            EXPECT_EQ(h.cont(i), CONT_OK);
        }
    }

    {
        SCOPED_TRACE("Phase B: continuity classification");

        // Nominal readings for each circuit state, as fractions of battery voltage (factors in IgnitersModule.cpp).
        const float vref = h.bat.batVolts;
        h.adc = {.volts = {0.0f, IGN_FUSE_WORKING_IGN_NOT_PRESENT_FACTOR * vref, IGN_FUSE_NOT_WORKING_IGN_PRESENT_FACTOR * vref, IGN_FUSE_NOT_WORKING_IGN_NOT_PRESENT_FACTOR * vref}};
        h.step();

        ASSERT_TRUE(h.contUpdated);
        EXPECT_EQ(h.cont(0), IGN_PRESENT | FUSE_WORKING);
        EXPECT_EQ(h.cont(1), FUSE_WORKING);
        EXPECT_EQ(h.cont(2), IGN_PRESENT);
        EXPECT_EQ(h.cont(3), 0);

        h.bat.batPercent = 0;
        h.step();

        for (uint8_t i = 0; i < IGN_COUNT; i++)
        {
            EXPECT_EQ(h.cont(i), 0) << "an empty battery must report no continuity on channel " << i + 1;
        }

        h.bat.batPercent = 100;
        h.adc = {};
        h.step();
    }

    {
        SCOPED_TRACE("Phase C: ground test fires");

        groundTest(1);
        groundTest(2);
        groundTest(3);
        groundTest(3); // re-testing a channel must be allowed
    }

    {
        SCOPED_TRACE("Phase D: invalid test fire commands");

        expectRejected(0, "channel 0 must be rejected");
        expectRejected(IGN_COUNT + 1, "channel past IGN_COUNT must be rejected");
    }

    {
        SCOPED_TRACE("Phase E: test fire outside STANDING");

        h.smState = DATALINK_SM_STATE_ARMED;
        h.step();

        expectRejected(4, "test fire while ARMED must be rejected");
    }

    {
        SCOPED_TRACE("Phase F: nothing fires before apogee");

        h.smState = DATALINK_SM_STATE_FREE_FLIGHT;

        h.setFlight(800.0f, 150.0f);
        EXPECT_EQ(h.stepCountingFiredUpdates(50), 0) << "nothing may fire during ascent";

        // Backup triggers are only evaluated in FREE_FALL, even when the EKF alone would satisfy them.
        h.setFlight(150.0f, -30.0f);
        EXPECT_EQ(h.stepCountingFiredUpdates(50), 0) << "backup must not fire before FREE_FALL";
    }

    {
        SCOPED_TRACE("Phase G: apogee fires the pilot igniter once");

        h.smState = DATALINK_SM_STATE_FREE_FALL;
        h.setFlight(1000.0f, -1.0f);
        h.step();

        ASSERT_TRUE(h.firedUpdated) << "pilot must fire on the first FREE_FALL tick";
        h.expectFired(true, false, false, false);

        h.expectPulse(0, false);

        EXPECT_EQ(h.stepCountingFiredUpdates(100), 0) << "pilot must not re-fire, and backup must stay idle above main altitude";
    }

    {
        SCOPED_TRACE("Phase H: backup fires at the main deployment altitude");

        h.setFlight(200.5f, -15.0f);
        EXPECT_EQ(h.stepCountingFiredUpdates(20), 0) << "backup must not fire above MAIN_PARACHUTE_HEIGHT (200 m)";

        h.setFlight(200.0f, -15.0f);
        h.step();

        ASSERT_TRUE(h.firedUpdated) << "backup must fire at exactly MAIN_PARACHUTE_HEIGHT";
        h.expectFired(true, true, false, false);

        h.expectPulse(1, false);

        h.setFlight(100.0f, -15.0f);
        EXPECT_EQ(h.stepCountingFiredUpdates(100), 0) << "backup must not re-fire";
    }

    {
        SCOPED_TRACE("Phase I: continuity with a slow ADC and after ADC loss");

        // The ADC module runs at 10 Hz while ign runs at 500 Hz, so most cycles see no new sample.
        // That alone must never be treated as lost ADC data.
        for (uint32_t ms = 0; ms < 1000; ms++)
        {
            h.adcEnabled = (ms % 100 == 0);
            h.step();

            if (h.contUpdated)
            {
                EXPECT_EQ(h.cont(0), CONT_OK) << "continuity was reset " << ms << " ms into 10 Hz ADC updates";
            }
        }

        h.adcEnabled = true;
        h.step(); // last ADC sample

        h.adcEnabled = false;

        for (uint32_t ms = 1; ms < 500; ms++)
        {
            h.step();

            EXPECT_FALSE(h.contUpdated) << "continuity reset only " << ms << " ms after the last ADC sample";
        }

        h.step();

        ASSERT_TRUE(h.contUpdated) << "continuity must be reset IGN_CONTINUITY_DEAD_TIME (500 ms) after the last ADC sample";

        for (uint8_t i = 0; i < IGN_COUNT; i++)
        {
            EXPECT_EQ(h.cont(i), 0);
        }
    }
}
