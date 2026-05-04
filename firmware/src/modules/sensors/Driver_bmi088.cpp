#include "Driver_bmi088.h"
#include <lib/geo/physical_constants.h>
#include <board_config.h>
#include <cmath>

static constexpr float ACC_CLIPPING_THRESHOLD = 6.0f * EARTH_GRAVITY * 0.95f;
static constexpr float GYRO_CLIPPING_THRESHOLD = 500.0f * 0.95f;

void Driver_bmi088::initialize()
{
    bmi088_acc_init_spi(&m_AccDevice, CFG_SPI, CFG_PIN_CS_BMI_ACC);
    bmi088_acc_set_conf(&m_AccDevice, BMI088_ACC_ODR_800HZ, BMI088_ACC_OSR_NORMAL);
    bmi088_acc_set_range(&m_AccDevice, BMI088_ACC_RANGE_6G);

    bmi088_gyro_init_spi(&m_GyroDevice, CFG_SPI, CFG_PIN_CS_BMI_GYRO);
    bmi088_gyro_set_bandwidth(&m_GyroDevice, BMI088_GYRO_ODR_2000_BW_523HZ);
    bmi088_gyro_set_range(&m_GyroDevice, BMI088_GYRO_RANGE_500DPS);
}

void Driver_bmi088::readAndPublish(float dt)
{
    m_CurrentFrame.dt = dt;

    bmi088_acc_read(&m_AccDevice, &m_CurrentFrame.acc);
    bmi088_gyro_read(&m_GyroDevice, &m_CurrentFrame.gyro);

    // FLU -> FRD conversion (negate y & z)
    m_CurrentFrame.acc.y *= -1;
    m_CurrentFrame.acc.z *= -1;
    m_CurrentFrame.gyro.y *= -1;
    m_CurrentFrame.gyro.z *= -1;

    // Check for clipping
    m_CurrentFrame.clippingFlags = 0;

    if (fabsf(m_CurrentFrame.acc.x) >= ACC_CLIPPING_THRESHOLD)
    {
        m_CurrentFrame.clippingFlags |= PubSub::Helpers::ACC_CLIP_X;
    }
    if (fabsf(m_CurrentFrame.acc.y) >= ACC_CLIPPING_THRESHOLD)
    {
        m_CurrentFrame.clippingFlags |= PubSub::Helpers::ACC_CLIP_Y;
    }
    if (fabsf(m_CurrentFrame.acc.z) >= ACC_CLIPPING_THRESHOLD)
    {
        m_CurrentFrame.clippingFlags |= PubSub::Helpers::ACC_CLIP_Z;
    }
    if (fabsf(m_CurrentFrame.gyro.x) >= GYRO_CLIPPING_THRESHOLD)
    {
        m_CurrentFrame.clippingFlags |= PubSub::Helpers::GYRO_CLIP_X;
    }
    if (fabsf(m_CurrentFrame.gyro.y) >= GYRO_CLIPPING_THRESHOLD)
    {
        m_CurrentFrame.clippingFlags |= PubSub::Helpers::GYRO_CLIP_Y;
    }
    if (fabsf(m_CurrentFrame.gyro.z) >= GYRO_CLIPPING_THRESHOLD)
    {
        m_CurrentFrame.clippingFlags |= PubSub::Helpers::GYRO_CLIP_Z;
    }

    m_Publisher.publish(m_CurrentFrame);
}