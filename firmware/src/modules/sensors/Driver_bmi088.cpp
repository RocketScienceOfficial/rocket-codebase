#include "Driver_bmi088.h"
#include <board_config.h>

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

    m_Publisher.publish(m_CurrentFrame);
}