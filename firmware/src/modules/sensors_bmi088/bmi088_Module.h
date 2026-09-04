#pragma once

#include <pubsub/Topics.h>
#include <pubsub/Publisher.h>
#include "bmi088_acc_driver.h"
#include "bmi088_gyro_driver.h"
#include <cstdint>

class bmi088_Module
{
public:
    bmi088_Module(hal_spi_bus_t spi, hal_gpio_pin_t cs_acc, hal_gpio_pin_t cs_gyro) : m_SPI(spi), m_CS_acc(cs_acc), m_CS_gyro(cs_gyro) {}

    void init();
    void run();

private:
    PubSub::Publisher<PubSub::Topics::sensors_imu_1_topic> m_Publisher;

    const hal_spi_bus_t m_SPI;
    const hal_gpio_pin_t m_CS_acc;
    const hal_gpio_pin_t m_CS_gyro;

    bmi088_acc_device_t m_AccDevice{};
    bmi088_gyro_device_t m_GyroDevice{};
    uint64_t m_LastReadTimeUs = 0;

    PubSub::Messages::SensorsIMU m_CurrentFrame{};
};