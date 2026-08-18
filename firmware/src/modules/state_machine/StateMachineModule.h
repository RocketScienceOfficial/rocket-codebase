#pragma once

#include <pubsub/Topics.h>
#include <pubsub/Publisher.h>
#include <pubsub/Subscriber.h>
#include <pubsub/RPCHandler.h>
#include <lib/maths/vector.h>

class StateMachineModule
{
public:
    void init();
    void run();

private:
    PubSub::Subscriber<PubSub::Topics::sensors_imu_1_topic> m_IMUDataSubscriber;
    PubSub::Subscriber<PubSub::Topics::sensors_baro_1_topic> m_BaroDataSubscriber;
    PubSub::Publisher<PubSub::Topics::sm_state_topic> m_FlightStatePublisher;
    PubSub::Publisher<PubSub::Topics::sm_height_topic> m_FlightStateHeightPublisher;
    PubSub::RPCHandler<PUBSUB_RPC_ID(command_arm)> m_RPC_ARM;

    // State
    state_machine_state m_State;
    float m_StartupBaseAlt;
    bool m_StartupBaseAltSet;
    uint32_t m_VerificationStartTime;

    // Baro data
    float m_CurrentBaroHeight;
    bool m_BaroHeightChanged;

    // IMU data
    vec3_t m_CurrentIMUAcc;
    bool m_IMUAccChanged;

    // Handlers utils
    bool m_BaseAltSet;
    float m_BaseAlt;
    bool m_VerifyingStandingAlt;
    float m_Apogee;
    float m_LandingAlt;

    // Utility functions
    void updateData();
    void postUpdate();
    void changeState(state_machine_state new_state);
    void resetFlightTrackingState();

    // State handlers
    void handle_state_standing();
    void handle_state_armed();
    void handle_state_accelerating();
    void handle_state_free_flight();
    void handle_state_free_fall();
    void handle_state_landed();
};