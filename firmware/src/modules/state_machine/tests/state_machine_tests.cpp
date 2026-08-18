#include <gtest/gtest.h>

#include "modules/state_machine/StateMachineModule.h"
#include "modules/state_machine/SMConfig.h"
#include <pubsub/Topics.h>
#include <pubsub/Publisher.h>
#include <pubsub/Subscriber.h>
#include <pubsub/RPCRequest.h>
#include <lib/geo/physical_constants.h>
#include <sitl.h>

using namespace PubSub::Topics;

// StateMachineModule owns sm_state/sm_height/command_arm-response (single owner per topic, see
// MessageBus::AdvertiseTopic), so only one instance may exist in this test binary -- hence one long
// scenario instead of separate TEST cases.

TEST(StateMachineModule, FullLifecycleAndEdgeCases)
{
    // {} value-initializes m_BaseAlt/m_Apogee/etc, which have no in-class initializer; production
    // code gets this for free via static storage duration.
    StateMachineModule module{};

    PubSub::Publisher<SensorsIMU> imuPub{PUBSUB_ID(sensors_imu_1)};
    PubSub::Publisher<SensorsBaro> baroPub{PUBSUB_ID(sensors_baro_1)};
    PubSub::Subscriber<StateMachineState> stateSub{PUBSUB_ID(sm_state)};
    PubSub::Subscriber<StateMachineHeight> heightSub{PUBSUB_ID(sm_height)};
    PubSub::RPCRequest<CommandArm> armRpc{PUBSUB_RPC_ID(command_arm)};

    const vec3_t LIFTOFF_SPIKE_ACC = {0.0f, 0.0f, -(3.5f * (float)EARTH_GRAVITY)}; // > SM_CFG_START_ACC_THRESHOLD (2.5g)

    auto pushImu = [&](const vec3_t &acc)
    {
        imuPub.publish({.dt = 0.002f, .acc = acc, .gyro = {0.0f, 0.0f, 0.0f}, .clippingFlags = 0});
        module.run();
    };

    auto pushBaro = [&](float height)
    {
        baroPub.publish({.press = 0, .temp = 20.0f, .baroHeight = height});
        module.run();
    };

    // Repeats the same reading so exp_smoothing settles on it without hand-computing the series.
    auto settleBaro = [&](float height)
    {
        for (int i = 0; i < 60; i++)
        {
            pushBaro(height);
        }
    };

    // Advances the SITL lockstep clock without a real wall-clock delay (requires SITL_FREERUN=OFF).
    auto advanceMs = [&](uint32_t ms)
    {
        for (uint32_t i = 0; i < ms; i++)
        {
            sitl_time_tick();
        }
    };

    auto drainHeight = [&]()
    {
        while (heightSub.poll())
        {
        }
    };

    auto arm = [&](bool value) -> bool
    {
        armRpc.call({.arm = value}, 0);
        module.run();

        bool finished = armRpc.finished();
        EXPECT_TRUE(finished) << "ARM RPC did not receive a response within one run() cycle";

        return finished && armRpc.isSuccess();
    };

    auto expectNoStateChange = [&](const char *msg)
    {
        EXPECT_FALSE(stateSub.poll()) << msg;
    };

    auto expectStateChange = [&](state_machine_state expected, const char *msg)
    {
        ASSERT_TRUE(stateSub.poll()) << msg << " (no sm_state message was published at all)";
        EXPECT_EQ(stateSub.get().state, expected) << msg;
    };

    {
        SCOPED_TRACE("Phase A: disarm while STANDING");

        EXPECT_FALSE(arm(false)) << "disarm request while STANDING must be rejected";
        expectNoStateChange("a rejected disarm request must not publish a state change");
    }

    {
        SCOPED_TRACE("Phase B: arm from STANDING");

        EXPECT_TRUE(arm(true)) << "arm request while STANDING must be accepted";
        expectStateChange(DATALINK_SM_STATE_ARMED, "successful arm must publish ARMED");

        drainHeight();
        pushBaro(5.0f); // first-ever baro sample, just to exercise the sm_height publish check below
        EXPECT_TRUE(heightSub.poll()) << "sm_height should be published every run() cycle";
    }

    {
        SCOPED_TRACE("Phase C: arm again while ARMED");

        EXPECT_FALSE(arm(true)) << "a second arm request while already ARMED must be rejected";
        expectNoStateChange("a rejected re-arm request must not publish a state change");
    }

    // Phase D: a realistic abort -- verification starts and captures a real (non-zero) baseline,
    // then the pilot disarms before the altitude threshold is crossed or the verification timeout
    // elapses. Neither of those self-cleaning paths runs, so m_BaseAltSet/m_BaseAlt/
    // m_VerifyingStandingAlt/m_VerificationStartTime are all left stale for Phase E to catch.
    {
        SCOPED_TRACE("Phase D: abort mid-verification, then disarm, leaving stale state behind");

        pushImu(LIFTOFF_SPIKE_ACC);
        pushBaro(50.0f);
        expectNoStateChange("the first verification sample only captures the baseline");

        ASSERT_TRUE(arm(false)) << "disarm from ARMED must be accepted, even mid-verification";
        expectStateChange(DATALINK_SM_STATE_STANDING, "disarm must publish STANDING");
    }

    // Phase E: the primary regression check. m_BaseAltSet/m_BaseAlt/m_VerifyingStandingAlt/
    // m_VerificationStartTime/m_Apogee/m_LandingAlt are now reset in resetFlightTrackingState(),
    // called from updateData() on the STANDING->ARMED transition, so this re-arm should start from
    // a fresh baseline instead of comparing against the stale one left by Phase D.
    {
        SCOPED_TRACE("Phase E: disarm -> rearm baseline-leak regression check");

        ASSERT_TRUE(arm(true)) << "re-arm from STANDING must be accepted";
        expectStateChange(DATALINK_SM_STATE_ARMED, "re-arm must publish ARMED");

        pushImu(LIFTOFF_SPIKE_ACC);
        pushBaro(0.0f);

        // Fatal: if this already fails, the rest of the walk's assumptions no longer hold.
        ASSERT_FALSE(stateSub.poll())
            << "REGRESSION: rearm did not start a fresh standing-altitude baseline -- the first "
               "post-rearm baro sample was compared against a stale baseline left over from a prior "
               "(disarmed) arm attempt instead of being captured as a new one. See "
               "StateMachineModule.cpp handle_state_armed()/updateData().";
    }

    {
        SCOPED_TRACE("Phase F: a zero baseline still detects a real crossing -> ACCELERATING");

        settleBaro(40.0f);
        expectStateChange(DATALINK_SM_STATE_ACCELERATING, "crossing the fresh baseline threshold must transition to ACCELERATING");
    }

    {
        SCOPED_TRACE("Phase G: acceleration-vs-gravity boundary");

        pushImu({0.0f, 0.0f, -(float)EARTH_GRAVITY});
        expectNoStateChange("acceleration magnitude exactly equal to EARTH_GRAVITY must NOT transition");

        pushImu({0.0f, 0.0f, -((float)EARTH_GRAVITY - 1.0f)});
        expectStateChange(DATALINK_SM_STATE_FREE_FLIGHT, "acceleration magnitude below EARTH_GRAVITY must transition to FREE_FLIGHT");
    }

    float apogeeAlt = 90.0f;
    {
        SCOPED_TRACE("Phase H: apogee plateau timing boundary");

        settleBaro(apogeeAlt);
        expectNoStateChange("must not transition to FREE_FALL while altitude is still climbing");

        advanceMs(100);
        pushBaro(apogeeAlt);
        expectNoStateChange("must not transition before SM_CFG_LAST_ALT_APOGEE_TIME_MS has elapsed");

        advanceMs(200); // cumulative 300ms > SM_CFG_LAST_ALT_APOGEE_TIME_MS (250ms)
        pushBaro(apogeeAlt);
        expectStateChange(DATALINK_SM_STATE_FREE_FALL, "must transition to FREE_FALL once the plateau has held past SM_CFG_LAST_ALT_APOGEE_TIME_MS");
    }

    {
        SCOPED_TRACE("Phase I: landing plateau timing boundary");

        pushBaro(apogeeAlt); // first FREE_FALL sample only establishes the landing-candidate altitude
        expectNoStateChange("the first FREE_FALL baro sample only establishes a landing-candidate altitude");

        float landedAlt = -30.0f;
        settleBaro(landedAlt);
        expectNoStateChange("must not transition to LANDED while altitude is still changing");

        advanceMs(2000);
        pushBaro(landedAlt);
        expectNoStateChange("must not transition before SM_CFG_LAST_ALT_LAND_VERIFICATION_TIME_MS has elapsed");

        advanceMs(2500); // cumulative 4500ms > SM_CFG_LAST_ALT_LAND_VERIFICATION_TIME_MS (4000ms)
        pushBaro(landedAlt);
        expectStateChange(DATALINK_SM_STATE_LANDED, "must transition to LANDED once the plateau has held past SM_CFG_LAST_ALT_LAND_VERIFICATION_TIME_MS");
    }
}
