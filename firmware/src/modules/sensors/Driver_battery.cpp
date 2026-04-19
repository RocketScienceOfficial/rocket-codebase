#include "Driver_battery.h"
#include <board_config.h>
#include <lib/maths/math_utils.h>
#include <cmath>

#define ADC_CALIBRATION_OFFSET 0.10f

#define BATTERY_VOLTAGE_DIVIDER 11.0f
#define BATTERY_VOLTAGE_DIFFERENCE_THRESHOLD 0.1f
#define EXP_FILTER_BAT_COEFF 0.2f

// REF: https://blog.ampow.com/lipo-voltage-chart/
static const battery_table_entry_t s_BatteryTable[] = {
    {4.50f, 100},
    {4.20f, 100},
    {4.15f, 95},
    {4.11f, 90},
    {4.08f, 85},
    {4.02f, 80},
    {3.98f, 75},
    {3.95f, 70},
    {3.91f, 65},
    {3.87f, 60},
    {3.85f, 55},
    {3.84f, 50},
    {3.82f, 45},
    {3.80f, 40},
    {3.79f, 35},
    {3.77f, 30},
    {3.75f, 25},
    {3.73f, 20},
    {3.71f, 15},
    {3.69f, 10},
    {3.61f, 5},
    {3.27f, 0},
};

void Driver_battery::initialize()
{
    ads786x_init(&m_Device, OBC_SPI, PIN_CS_ADS, ADS786X_TYPE_6, ADC_VREF);
    battery_init(&m_BatteryConfig, s_BatteryTable, sizeof(s_BatteryTable) / sizeof(battery_table_entry_t));
}

void Driver_battery::readAndPublish(float dt)
{
    (void)dt;
    
    float batteryReading = ads786x_read(&m_Device) * BATTERY_VOLTAGE_DIVIDER + ADC_CALIBRATION_OFFSET;
    accumulateBatteryReading(batteryReading);

    float delta = 0.0f;
    smoothenBatteryVoltage(delta);

    if (fabsf(delta) < BATTERY_VOLTAGE_DIFFERENCE_THRESHOLD)
    {
        battery_data_t data = {};
        battery_convert(&m_BatteryConfig, m_CurrentFrame.batVolts, &data);

        m_CurrentFrame.batPercent = data.percentage;
        m_CurrentFrame.batNCells = data.nCells;
    }
    else
    {
        m_CurrentFrame.batPercent = 0;
        m_CurrentFrame.batNCells = 0;
    }

    m_Publisher.publish(m_CurrentFrame);
}

void Driver_battery::accumulateBatteryReading(float reading)
{
    if (m_BatteryReadingsCount < BATTERY_READINGS_COUNT)
    {
        m_BatteryReadingsSum += reading;
        m_BatteryReadingsCount++;
    }
    else
    {
        m_BatteryReadingsSum = m_BatteryReadingsSum + reading - m_BatteryReadings[m_BatteryReadingsNextIndex];
    }

    m_BatteryReadings[m_BatteryReadingsNextIndex++] = reading;

    if (m_BatteryReadingsNextIndex >= BATTERY_READINGS_COUNT)
    {
        m_BatteryReadingsNextIndex = 0;
    }
}

void Driver_battery::smoothenBatteryVoltage(float &delta)
{
    float avgBat = m_BatteryReadingsSum / m_BatteryReadingsCount;
    float smoothedBatVolts = exp_smoothing(avgBat, m_CurrentFrame.batVolts, EXP_FILTER_BAT_COEFF);
    float deltaBatVolts = smoothedBatVolts - m_CurrentFrame.batVolts;

    delta = deltaBatVolts;
    m_CurrentFrame.batVolts = smoothedBatVolts;

    if (m_CurrentFrame.batVolts < 0.2f)
    {
        m_CurrentFrame.batVolts = 0.0f;
    }
}