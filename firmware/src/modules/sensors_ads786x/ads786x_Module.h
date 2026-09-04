#pragma once

#include <pubsub/Topics.h>
#include <pubsub/Publisher.h>
#include "ads786x_driver.h"

class ads786x_Module
{
public:
    ads786x_Module(hal_spi_bus_t spi, hal_gpio_pin_t cs, float vRef, float calibScale, float calibOffset) : m_SPI(spi), m_CS(cs), m_VRef(vRef), m_CalibScale(calibScale), m_CalibOffset(calibOffset) {}

    void init();
    void run();

private:
    PubSub::Publisher<PubSub::Topics::sensors_battery_raw_topic> m_Publisher;

    const hal_spi_bus_t m_SPI;
    const hal_gpio_pin_t m_CS;
    const float m_VRef;
    const float m_CalibScale;
    const float m_CalibOffset;

    ads786x_device_t m_Device{};
    PubSub::Messages::SensorsBatteryRaw m_CurrentFrame{};
};