#pragma once

#include <pubsub/Topics.h>
#include <pubsub/Publisher.h>
#include "ms56xx_driver.h"

class ms56xx_Module
{
public:
    ms56xx_Module(hal_spi_bus_t spi, hal_gpio_pin_t cs, bool version_5611) : m_SPI(spi), m_CS(cs), m_Version5611(version_5611) {}

    void init();
    void run();

private:
    PubSub::Publisher<PubSub::Topics::sensors_baro_1_topic> m_Publisher;

    const hal_spi_bus_t m_SPI;
    const hal_gpio_pin_t m_CS;
    const bool m_Version5611;

    ms56xx_device_t m_Device;
    PubSub::Messages::SensorsBaro m_CurrentFrame{};
};