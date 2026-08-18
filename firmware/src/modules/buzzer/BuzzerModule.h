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
    PubSub::Subscriber<PubSub::Topics::sensors_gps_1_topic> m_GPSSubscriber;
    PubSub::Subscriber<PubSub::Topics::sm_state_topic> m_SMSubscriber;

    enum class Tone
    {
        START,
        ARM,
        DISARM,
        GPS_FIX,
        LANDED,
    };

    const BuzzerTone *m_CurrentTone;
    size_t m_CurrentToneSize;
    uint32_t m_BuzzerToneStartTime;
    bool m_GPSPlayed;

    void setTone(Tone tone);
    void playCurrentTone();
};