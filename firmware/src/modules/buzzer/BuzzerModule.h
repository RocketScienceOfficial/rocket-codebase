#pragma once

#include "BuzzerMelodies.h"
#include <hal/pwm_driver.h>
#include <pubsub/Topics.h>
#include <pubsub/Subscriber.h>
#include <cstddef>

class BuzzerModule
{
public:
    void init();
    void run();

private:
    PubSub::Subscriber<PubSub::Topics::SensorsGPS> m_GPSSubscriber{PUBSUB_ID(sensors_gps_1)};
    PubSub::Subscriber<PubSub::Topics::StateMachineState> m_SMSubscriber{PUBSUB_ID(sm_state)};

    enum class Tone
    {
        START,
        ARM,
        DISARM,
        GPS_FIX,
        LANDED,
    };

    hal_pwm_config_t m_Device;
    const BuzzerTone *m_CurrentTone;
    size_t m_CurrentToneSize;
    uint32_t m_BuzzerToneStartTime;
    bool m_GPSPlayed;

    void setTone(Tone tone);
    void playCurrentTone();
};