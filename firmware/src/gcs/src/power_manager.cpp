#include "power_manager.h"
#include "logger.h"
#include "XPowersLib.h"
#include <Arduino.h>
#include <Wire.h>

static PMUData g_current_data;
static XPowersLibInterface *g_pmu;

// REF: https://github.com/Xinyuan-LilyGO/LilyGo-LoRa-Series/blob/master/examples/PMU/boards.h
void power_init()
{
    if (!g_pmu)
    {
        g_pmu = new XPowersAXP2101(Wire);

        if (!g_pmu->init())
        {
            SERIAL_DEBUG_PRINTF("Warning: Failed to find AXP2101 power management\n");

            delete g_pmu;

            g_pmu = NULL;
        }
        else
        {
            SERIAL_DEBUG_PRINTF("AXP2101 PMU init succeeded, using AXP2101 PMU\n");
        }
    }

    if (!g_pmu)
    {
        return;
    }

    g_pmu->setChargingLedMode(XPOWERS_CHG_LED_BLINK_1HZ);

    g_pmu->disablePowerOutput(XPOWERS_DCDC2);
    g_pmu->disablePowerOutput(XPOWERS_DCDC3);
    g_pmu->disablePowerOutput(XPOWERS_DCDC4);
    g_pmu->disablePowerOutput(XPOWERS_DCDC5);
    g_pmu->disablePowerOutput(XPOWERS_ALDO1);
    g_pmu->disablePowerOutput(XPOWERS_ALDO4);
    g_pmu->disablePowerOutput(XPOWERS_BLDO1);
    g_pmu->disablePowerOutput(XPOWERS_BLDO2);
    g_pmu->disablePowerOutput(XPOWERS_DLDO1);
    g_pmu->disablePowerOutput(XPOWERS_DLDO2);

    g_pmu->setPowerChannelVoltage(XPOWERS_VBACKUP, 3300);
    g_pmu->enablePowerOutput(XPOWERS_VBACKUP);

    g_pmu->setProtectedChannel(XPOWERS_DCDC1);

    g_pmu->setPowerChannelVoltage(XPOWERS_ALDO2, 3300);
    g_pmu->enablePowerOutput(XPOWERS_ALDO2);

    g_pmu->setPowerChannelVoltage(XPOWERS_ALDO3, 3300);
    g_pmu->enablePowerOutput(XPOWERS_ALDO3);

    g_pmu->enableSystemVoltageMeasure();
    g_pmu->enableVbusVoltageMeasure();
    g_pmu->enableBattVoltageMeasure();

    g_pmu->setPowerKeyPressOffTime(XPOWERS_POWEROFF_4S);
}

void power_read()
{
    if (!g_pmu)
    {
        return;
    }

    g_current_data.isCharging = g_pmu->isCharging();
    g_current_data.isDischarge = g_pmu->isDischarge();
    g_current_data.isVBusIn = g_pmu->isVbusIn();
    g_current_data.isBatteryConnected = g_pmu->isBatteryConnect();
    g_current_data.batteryVoltage = g_pmu->getBattVoltage() / 1000.0f;
    g_current_data.VBusVoltage = g_pmu->getVbusVoltage() / 1000.0f;
    g_current_data.systemVoltage = g_pmu->getSystemVoltage() / 1000.0f;
    g_current_data.batteryPercentage = g_current_data.isBatteryConnected ? g_pmu->getBatteryPercent() : 0;
}

const PMUData &power_get_current_data()
{
    return g_current_data;
}