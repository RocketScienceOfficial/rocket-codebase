#include "PMUModule.h"
#include <board_config.h>
#include <hal/i2c_driver.h>
#include <string.h>

static int i2c_read_reg(uint8_t devAddr, uint8_t regAddr, uint8_t *data, uint8_t len)
{
    return hal_i2c_transfer(CFG_I2C, devAddr, &regAddr, 1, data, len) == false;
}

static int i2c_write_reg(uint8_t devAddr, uint8_t regAddr, uint8_t *data, uint8_t len)
{
    static uint8_t buf[256];

    memcpy(buf, &regAddr, 1);
    memcpy(buf + 1, data, len);

    return hal_i2c_transfer(CFG_I2C, devAddr, buf, len + 1, NULL, 0) == false;
}

// REF: https://github.com/Xinyuan-LilyGO/LilyGo-LoRa-Series/blob/master/examples/PMU/LoRaBoards.cpp
void PMUModule::init()
{
    m_Device.begin(AXP2101_SLAVE_ADDRESS, i2c_read_reg, i2c_write_reg);

    m_Device.setChargingLedMode(XPOWERS_CHG_LED_BLINK_1HZ);
    m_Device.setPowerKeyPressOffTime(XPOWERS_POWEROFF_4S);

    // Unuse power channel
    m_Device.disablePowerOutput(XPOWERS_DCDC2);
    m_Device.disablePowerOutput(XPOWERS_DCDC3);
    m_Device.disablePowerOutput(XPOWERS_DCDC4);
    m_Device.disablePowerOutput(XPOWERS_DCDC5);
    m_Device.disablePowerOutput(XPOWERS_ALDO1);
    m_Device.disablePowerOutput(XPOWERS_ALDO4);
    m_Device.disablePowerOutput(XPOWERS_BLDO1);
    m_Device.disablePowerOutput(XPOWERS_BLDO2);
    m_Device.disablePowerOutput(XPOWERS_DLDO1);
    m_Device.disablePowerOutput(XPOWERS_DLDO2);
    m_Device.disablePowerOutput(XPOWERS_CPULDO);

    // GNSS RTC PowerVDD 3300mV
    m_Device.setPowerChannelVoltage(XPOWERS_VBACKUP, 3300);
    m_Device.enablePowerOutput(XPOWERS_VBACKUP);

    // ESP32 VDD 3300mV
    //  ! No need to set, automatically open , Don't close it
    //  m_Device.setPowerChannelVoltage(XPOWERS_DCDC1, 3300);
    //  m_Device.setProtectedChannel(XPOWERS_DCDC1);
    m_Device.setProtectedChannel(XPOWERS_DCDC1);

    // LoRa VDD 3300mV
    m_Device.setPowerChannelVoltage(XPOWERS_ALDO2, 3300);
    m_Device.enablePowerOutput(XPOWERS_ALDO2);

    // GNSS VDD 3300mV
    m_Device.setPowerChannelVoltage(XPOWERS_ALDO3, 3300);
    m_Device.enablePowerOutput(XPOWERS_ALDO3);

    // Set constant current charge current limit
    m_Device.setChargerConstantCurr(XPOWERS_AXP2101_CHG_CUR_500MA);

    // Set charge cut-off voltage
    m_Device.setChargeTargetVoltage(XPOWERS_AXP2101_CHG_VOL_4V2);

    m_Device.enableBattVoltageMeasure();
}

void PMUModule::run()
{
    float batteryVoltage = m_Device.getBattVoltage() / 1000.0f;
    int batteryPercentage = m_Device.getBatteryPercent();

    m_Publisher.publish({batteryVoltage, batteryPercentage});
}