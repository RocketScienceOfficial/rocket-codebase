#include "OLEDModule.h"
#include "modules/common/ModuleLogger.h"
#include <lib/maths/math_utils.h>
#include <osal/systime.h>
#include <string.h>
#include <stdlib.h>
#include <stdio.h>
#include <math.h>

#define REFRESH_RATE_MS 200

void OLEDModule::init()
{
    m_RocketData.name = "ROCKET";
    m_GCSData.name = "GCS";

    initDisplay();
    setNewState(OLEDState::ROCKET);

    u8g2_InitDisplay(&m_Display);
    u8g2_SetPowerSave(&m_Display, 0);

    u8g2_SetFont(&m_Display, u8g2_font_lucasfont_alternate_tf);
}

void OLEDModule::run()
{
    if (m_PMUSubscriber.poll())
    {
        const auto &pmuState = m_PMUSubscriber.get();

        m_GCSData.batteryPercentage = pmuState.batteryPercentage;
        m_GCSData.batteryVoltage = pmuState.batteryVoltage;
    }

    if (m_SimplifiedGPSSubscriber.poll())
    {
        const auto &simplifiedGPS = m_SimplifiedGPSSubscriber.get();

        m_GCSData.lat = simplifiedGPS.lat;
        m_GCSData.lon = simplifiedGPS.lon;
    }

    handleStateChange();

    if (osal_systime_get_ms() - m_LastUpdateTime >= REFRESH_RATE_MS)
    {
        u8g2_ClearBuffer(&m_Display);
        drawPanel();
        u8g2_SendBuffer(&m_Display);

        m_LastUpdateTime = osal_systime_get_ms();
    }
}

void OLEDModule::handleStateChange()
{
    if (shouldChangeState())
    {
        if (!m_StateInitialized)
        {
            return;
        }

        if (!m_ButtonPressed)
        {
            m_ButtonPressed = true;

            setNewState((OLEDState)(((int)m_CurrentState + 1) % (int)OLEDState::_LAST));
        }
    }
    else
    {
        m_ButtonPressed = false;
        m_StateInitialized = true;
    }
}

void OLEDModule::setNewState(OLEDState newState)
{
    m_CurrentState = newState;

    switch (m_CurrentState)
    {
    case OLEDState::ROCKET:
        m_CurrentData = &m_RocketData;
        break;
    case OLEDState::GCS:
        m_CurrentData = &m_GCSData;
        break;
    default:
        break;
    }

    LOG_INFO("Switched to state: '%s'", m_CurrentData->name);
}

static void _draw_left_str_f(u8g2_t *u8g2, uint8_t x, uint8_t y, const char *str, ...)
{
    static char buf[128];

    va_list args;
    va_start(args, str);
    vsnprintf(buf, sizeof(buf), str, args);
    va_end(args);

    u8g2_DrawStr(u8g2, x, y, buf);
}

static void _draw_centered_str_f(u8g2_t *u8g2, uint8_t x, uint8_t y, const char *str, ...)
{
    static char buf[128];

    va_list args;
    va_start(args, str);
    vsnprintf(buf, sizeof(buf), str, args);
    va_end(args);

    uint16_t strWidth = u8g2_GetStrWidth(u8g2, buf);
    u8g2_DrawStr(u8g2, x - strWidth / 2, y, buf);
}

static void _draw_right_str_f(u8g2_t *u8g2, uint8_t x, uint8_t y, const char *str, ...)
{
    static char buf[128];

    va_list args;
    va_start(args, str);
    vsnprintf(buf, sizeof(buf), str, args);
    va_end(args);

    uint16_t strWidth = u8g2_GetStrWidth(u8g2, buf);
    u8g2_DrawStr(u8g2, x - strWidth, y, buf);
}

static float _get_rssi_percentage(float rssi)
{
    const float y_a = 0.0f, y_b = 100.0f, x_a = -130.0f, x_b = -10.0f;

    return clamp_value((y_b - y_a) / (x_b - x_a) * (rssi - x_a) + y_a, y_a, y_b);
}

static void _draw_progress_bar(u8g2_t *u8g2, uint16_t x, uint16_t y, uint16_t width, uint16_t height, uint8_t progress)
{
    u8g2_DrawBox(u8g2, x, y, width * (float)progress / 100.0f, height);
}

static void _draw_battery_indicator(u8g2_t *u8g2, uint16_t x, uint16_t y, uint16_t width, uint16_t height, uint8_t percentage)
{
    const uint8_t MAX_STEPS_COUNT = 4;
    const uint16_t TOTAL_STEP_WIDTH = width / MAX_STEPS_COUNT;
    const uint16_t STEP_WIDTH = TOTAL_STEP_WIDTH * 0.75;
    const uint16_t STEP_HEIGHT = height * 0.75;

    u8g2_DrawFrame(u8g2, x - width / 2, y - height / 2, width, height);
    u8g2_DrawBox(u8g2, x - width / 2 - width / 10, y - height / 4, width / 10, height / 2);

    uint8_t stepsCount = (uint8_t)ceilf(MAX_STEPS_COUNT * (float)percentage / 100);

    for (uint8_t i = 0; i < stepsCount; i++)
    {
        u8g2_DrawBox(u8g2, x + TOTAL_STEP_WIDTH * (1 - i) + (TOTAL_STEP_WIDTH - STEP_WIDTH) / 2, y - STEP_HEIGHT / 2, STEP_WIDTH, STEP_HEIGHT);
    }
}

void OLEDModule::drawPanel()
{
    _draw_progress_bar(&m_Display, 0, 2, 45, 6, (uint8_t)_get_rssi_percentage(m_CurrentData->rssi));

    _draw_left_str_f(&m_Display, 52, 10, "%d", m_CurrentData->rssi);
    _draw_right_str_f(&m_Display, 125, 10, "%s", m_CurrentData->name);

    _draw_left_str_f(&m_Display, 0, 23, "RX: %d", m_CurrentData->rx);
    _draw_left_str_f(&m_Display, 50, 23, "TX: %d", m_CurrentData->tx);
    _draw_left_str_f(&m_Display, 0, 36, "EXEC: %ds", m_CurrentData->execTimeoutLeft);
    _draw_left_str_f(&m_Display, 0, 50, "%.7f", m_CurrentData->lat);
    _draw_left_str_f(&m_Display, 0, 62, "%.7f", m_CurrentData->lon);

    _draw_right_str_f(&m_Display, 125, 28, "%d %%", m_CurrentData->batteryPercentage);
    _draw_battery_indicator(&m_Display, 110, 41, 30, 15, m_CurrentData->batteryPercentage);
    _draw_right_str_f(&m_Display, 125, 63, "%.2f V", m_CurrentData->batteryVoltage);
}