#pragma once

#include "DriverBase.h"

class Driver_adc_ign : public DriverBase<Driver_adc_ign, PubSub::Topics::IgnAdcChannels>
{
public:
    Driver_adc_ign() : DriverBase(PUBSUB_ID(ign_adc_channels), 10) {}

    void initialize();
    void readAndPublish(float dt);

private:
    float readADC(uint8_t pin);
};