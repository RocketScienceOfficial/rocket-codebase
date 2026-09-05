#pragma once

#include <pubsub/Topics.h>
#include <pubsub/Publisher.h>
#include <lib/gps/ubx.h>
#include <hal/spi_driver.h>
#include <hal/gpio_driver.h>

class GPSModule
{
public:
    GPSModule(hal_spi_bus_t spi, hal_gpio_pin_t cs) : m_SPI(spi), m_CS(cs) {}

    void init();
    void run();

private:
    PubSub::Publisher<PubSub::Topics::sensors_gps_1_topic> m_Publisher;

    const hal_spi_bus_t m_SPI;
    const hal_gpio_pin_t m_CS;

    ubx_parser_t m_Parser{};
    PubSub::Messages::SensorsGPS m_CurrentFrame{};

    void configureSPI();
};