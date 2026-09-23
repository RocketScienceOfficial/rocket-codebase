#pragma once

#include <gtest/gtest.h>

#include "modules/ign/IgnitersModule.h"
#include "modules/ign/IgnConfig.h"
#include <pubsub/Topics.h>
#include <pubsub/Publisher.h>
#include <pubsub/Subscriber.h>
#include <pubsub/RPCRequest.h>
#include <sitl.h>

using namespace PubSub::Topics;
using namespace PubSub::Helpers;

inline constexpr int CONT_OK = IGN_PRESENT | FUSE_WORKING;

// IgnitersModule owns ign_fired/ign_continuity/command_ignite-response (Publisher's constructor enforces
// single ownership per topic), so each test binary may construct exactly one harness.
class IgnHarness
{
public:
    IgnitersModule module{1, 2, 3, 4};

    state_machine_state smState = DATALINK_SM_STATE_STANDING;
    PubSub::Messages::EKFState ekf{};
    PubSub::Messages::SensorsBattery bat{.batVolts = 12.0f, .batPercent = 100, .batNCells = 3};
    PubSub::Messages::IgnAdcChannels adc{}; // 0 V on every channel reads as CONT_OK

    bool adcEnabled = true;

    bool firedUpdated = false;
    bool contUpdated = false;

    PubSub::Subscriber<ign_fired_topic> firedSub;
    PubSub::Subscriber<ign_continuity_topic> contSub;
    PubSub::RPCRequest<PUBSUB_RPC_ID(command_ignite)> ignRpc;

    IgnHarness()
    {
        module.init();
    }

    // One module cycle at the current tick, then advances the SITL lockstep clock by 1 ms without a
    // real wall-clock delay (requires SITL_FREERUN=OFF). Every topic has depth 2, so outputs are
    // drained on each step to stay clear of the too-slow assert.
    void step()
    {
        m_SMPub.publish({.state = smState});
        m_EKFPub.publish(ekf);
        m_BatPub.publish(bat);

        if (adcEnabled)
        {
            m_ADCPub.publish(adc);
        }

        module.run();

        firedUpdated = firedSub.poll();
        contUpdated = contSub.poll();

        sitl_time_tick();
    }

    // Takes ENU values and stores them in the EKF's NED convention.
    void setFlight(float heightUp, float velUp)
    {
        ekf.position.z = -heightUp;
        ekf.velocity.z = -velUp;
    }

    bool fired(uint8_t idx) const
    {
        return firedSub.get().fired[idx];
    }

    int cont(uint8_t idx) const
    {
        return contSub.get().detectorsFlags[idx];
    }

    void expectFired(bool ign1, bool ign2, bool ign3, bool ign4) const
    {
        EXPECT_EQ(fired(0), ign1) << "ign_fired[0]";
        EXPECT_EQ(fired(1), ign2) << "ign_fired[1]";
        EXPECT_EQ(fired(2), ign3) << "ign_fired[2]";
        EXPECT_EQ(fired(3), ign4) << "ign_fired[3]";
    }

    int stepCountingFiredUpdates(int steps)
    {
        int updates = 0;

        for (int i = 0; i < steps; i++)
        {
            step();

            if (firedUpdated)
            {
                updates++;
            }
        }

        return updates;
    }

    // Call right after the step() on which igniter idx fired. Host GPIO is a no-op, so the pulse is
    // observed through continuity, which the module forces to 0 for exactly the window the pin is HIGH.
    void expectPulse(uint8_t idx, bool pendingTestFire)
    {
        ASSERT_TRUE(adcEnabled) << "pulse tracking needs ADC samples every step";

        EXPECT_EQ(cont(idx), 0) << "continuity must drop to 0 on the tick the igniter fires";

        for (uint32_t ms = 1; ms < IGN_UP_TIME_MS; ms++)
        {
            step();

            EXPECT_EQ(cont(idx), 0) << "igniter released early, " << ms << " ms after firing";
            EXPECT_FALSE(firedUpdated) << "ign_fired must not change mid-pulse (" << ms << " ms after firing)";

            if (pendingTestFire)
            {
                EXPECT_FALSE(ignRpc.finished()) << "test fire responded mid-pulse (" << ms << " ms after firing)";
            }
        }

        step();

        EXPECT_EQ(cont(idx), CONT_OK) << "igniter still held " << IGN_UP_TIME_MS << " ms after firing";
    }

private:
    PubSub::Publisher<sm_state_topic> m_SMPub;
    PubSub::Publisher<ekf_state_topic> m_EKFPub;
    PubSub::Publisher<sensors_battery_topic> m_BatPub;
    PubSub::Publisher<ign_adc_channels_topic> m_ADCPub;
};
