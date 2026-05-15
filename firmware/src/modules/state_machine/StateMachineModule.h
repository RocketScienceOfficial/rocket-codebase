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
    PubSub::Subscriber<PubSub::Topics::SensorsIMU> m_IMUDataSubscriber{PUBSUB_ID(sensors_imu_1)};
    PubSub::Subscriber<PubSub::Topics::SensorsBaro> m_BaroDataSubscriber{PUBSUB_ID(sensors_baro_1)};
    PubSub::Publisher<PubSub::Topics::StateMachineState> m_FlightStatePublisher{PUBSUB_ID(sm_state)};
    PubSub::Publisher<PubSub::Topics::StateMachineHeight> m_FlightStateHeightPublisher{PUBSUB_ID(sm_height)};
    PubSub::RPCHandler<PubSub::Topics::CommandArm> m_RPC_ARM{PUBSUB_RPC_ID(command_arm)};

    // State
    state_machine_state m_State;
    float m_StartupBaseAlt;
    bool m_StartupBaseAltSet;

    // Baro data
    float m_CurrentBaroHeight;
    bool m_BaroHeightChanged;

    // IMU data
    vec3_t m_CurrentIMUAcc;
    bool m_IMUAccChanged;

    // Start verification
    float m_BaseAlt;
    bool m_Verifing_StandingAlt;
    size_t m_VerificationIndex_StandingAlt;

    // Apogee verification
    float m_Apogee;
    size_t m_VerificationIndex_Apogee;

    // Landing verification
    float m_LandingAlt;
    size_t m_VerificationIndex_Landing;

    // Utility functions
    void updateData();
    void postUpdate();
    void changeState(state_machine_state new_state);

    // State handlers
    void handle_state_standing();
    void handle_state_armed();
    void handle_state_accelerating();
    void handle_state_free_flight();
    void handle_state_free_fall();
    void handle_state_landed();
};