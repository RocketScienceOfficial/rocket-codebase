#pragma once

#define PMU_UPDATE_INTERVAL 1000

struct PMUData
{
    bool isCharging;
    bool isDischarge;
    bool isVBusIn;
    bool isBatteryConnected;
    float batteryVoltage;
    float VBusVoltage;
    float systemVoltage;
    int batteryPercentage;
};

void power_init();
void power_read();
const PMUData &power_get_current_data();