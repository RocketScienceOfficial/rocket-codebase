#pragma once

#include <pubsub/Topics.h>
#include <pubsub/Publisher.h>
#include <hal/adc_driver.h>
#include <cstdint>

class InternalIgniterADCModule
{
public:
    InternalIgniterADCModule(hal_adc_channel_t ign1, hal_adc_channel_t ign2, hal_adc_channel_t ign3, hal_adc_channel_t ign4, float calibScale, float calibOffset)
        : m_ign1(ign1), m_ign2(ign2), m_ign3(ign3), m_ign4(ign4), m_CalibScale(calibScale), m_CalibOffset(calibOffset) {}

    void init();
    void run();

private:
    PubSub::Publisher<PubSub::Topics::ign_adc_channels_topic> m_Publisher;

    const hal_adc_channel_t m_ign1;
    const hal_adc_channel_t m_ign2;
    const hal_adc_channel_t m_ign3;
    const hal_adc_channel_t m_ign4;
    const float m_CalibScale;
    const float m_CalibOffset;

    PubSub::Messages::IgnAdcChannels m_CurrentFrame{};

    float readADC(hal_adc_channel_t channel);
};