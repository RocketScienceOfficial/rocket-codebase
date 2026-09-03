#pragma once

#include <pubsub/Topics.h>
#include <pubsub/Publisher.h>
#include <XPowersAXP2101.h>
#include <hal/i2c_driver.h>

class PMUModule
{
public:
    PMUModule(hal_i2c_bus_t i2cBus) : m_I2CBus(i2cBus) {}

    void init();
    void run();

private:
    PubSub::Publisher<PubSub::Topics::pmu_state_topic> m_Publisher;

    const hal_i2c_bus_t m_I2CBus;
    XPowersAXP2101 m_Device;
};