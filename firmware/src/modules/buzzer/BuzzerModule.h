#pragma once

#include "BuzzerMelodies.h"
#include <pubsub/Topics.h>
#include <pubsub/Subscriber.h>
#include <hal/pwm_driver.h>
#include <cstddef>

class BuzzerModule
{
public:
    BuzzerModule(hal_pwm_timer_t buzzerTimer, hal_pwm_channel_t buzzerChannel, hal_gpio_pin_t buzzerPin) : m_BuzzerTimer(buzzerTimer), m_BuzzerChannel(buzzerChannel), m_BuzzerPin(buzzerPin) {}

    void init();
    void run();

private:
    // API
    PubSub::Subscriber<PubSub::Topics::sensors_gps_1_topic> m_GPSSubscriber;
    PubSub::Subscriber<PubSub::Topics::sm_state_topic> m_SMSubscriber;

    // Buzzer configuration
    const hal_pwm_timer_t m_BuzzerTimer;
    const hal_pwm_channel_t m_BuzzerChannel;
    const hal_gpio_pin_t m_BuzzerPin;

    // Current tone being played
    const BuzzerTone *m_CurrentTone = NULL;
    size_t m_CurrentToneSize = 0;
    uint32_t m_BuzzerToneStartTime = 0;
    bool m_GPSPlayed = false;

    void setTone(ToneType tone);
    void playCurrentTone();
};