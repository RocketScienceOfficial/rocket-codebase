#pragma once

#include <pubsub/Topics.h>
#include <pubsub/Publisher.h>
#include "mmc5983ma_driver.h"

class mmc5983ma_Module
{
public:
    mmc5983ma_Module(hal_spi_bus_t spi, hal_gpio_pin_t cs) : m_SPI(spi), m_CS(cs) {}

    void init();
    void run();

private:
    PubSub::Publisher<PubSub::Topics::sensors_mag_1_topic> m_Publisher;

    const hal_spi_bus_t m_SPI;
    const hal_gpio_pin_t m_CS;

    mmc5983ma_device_t m_Device;
    PubSub::Messages::SensorsMag m_CurrentFrame{};
};