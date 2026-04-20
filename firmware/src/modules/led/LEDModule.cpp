#include "LEDModule.h"
#include "modules/common/ModuleLogger.h"
#include <board_config.h>

#define COLOR(r, g, b) HAL_WS2812B_COLOR((uint8_t)((r) * BRIGHTNESS), (uint8_t)((g) * BRIGHTNESS), (uint8_t)((b) * BRIGHTNESS))

void LEDModule::init()
{
    hal_ws2812b_init(CFG_PIN_LED, false);

    for (int i = 0; i < DIODES_COUNT; i++)
    {
        m_DiodesColors[i] = COLOR(0, 0, 0);
    }

    setArmState(false);

    update();
}

void LEDModule::run()
{
    using namespace PubSub::Helpers;

    if (m_ContinuitySubscriber.poll())
    {
        const auto &ignFrame = m_ContinuitySubscriber.get();

        for (uint8_t i = 0; i < PubSub::Helpers::IGN_CHANNELS_COUNT; i++)
        {
            setIgniterValue(i + 1, ignFrame.detectorsFlags[i] & IgnChannelContinuityFlags::FUSE_WORKING, ignFrame.detectorsFlags[i] & IgnChannelContinuityFlags::IGN_PRESENT);
        }
    }

    if (m_BatSubscriber.poll())
    {
        setBatteryPercentage(m_BatSubscriber.get().batPercent);
    }

    if (m_ReadySubscriber.poll())
    {
        setReadyState(m_ReadySubscriber.get().ready);
    }

    if (m_StateSubscriber.poll())
    {
        setArmState(m_StateSubscriber.get().state != DATALINK_SM_STATE_STANDING);
    }

    if (m_Updated)
    {
        m_Updated = false;

        update();
    }
}

void LEDModule::setIgniterValue(uint8_t igniterNumber, bool fuseWorking, bool ignPresent)
{
    hal_ws2812b_color_t &diodeColor = m_DiodesColors[igniterNumber - 1];

    if (fuseWorking)
    {
        diodeColor = ignPresent ? COLOR(0, 255, 0) : COLOR(255, 0, 0);
    }
    else
    {
        diodeColor = ignPresent ? COLOR(255, 165, 0) : COLOR(0, 0, 0);
    }

    m_Updated = true;
}

void LEDModule::setArmState(bool armed)
{
    m_DiodesColors[6] = armed ? COLOR(0, 255, 0) : COLOR(255, 0, 0);
    m_Updated = true;
}

void LEDModule::setBatteryPercentage(uint8_t percent)
{
    hal_ws2812b_color_t &diodeColor = m_DiodesColors[5];

    if (percent == 0)
    {
        diodeColor = COLOR(0, 0, 0);
    }
    else if (percent < 50)
    {
        diodeColor = COLOR(255, 255 * percent / 50, 0);
    }
    else
    {
        diodeColor = COLOR(255 - 255 * (percent - 50) / 50, 255, 0);
    }

    m_Updated = true;
}

void LEDModule::setReadyState(bool ready)
{
    m_DiodesColors[4] = ready ? COLOR(0, 255, 0) : COLOR(255, 0, 0);
    m_Updated = true;
}

void LEDModule::update()
{
    hal_ws2812b_set_colors(m_DiodesColors, DIODES_COUNT);

    OBC_DEBUG("LEDs updated");
}