#include "IgnitersModule.h"
#include "modules/common/ModuleLogger.h"
#include <board_config.h>
#include <osal/systime.h>
#include <hal/gpio_driver.h>

#define IGN_UP_TIME_MS 10
#define MAIN_PARACHUTE_HEIGHT 100
#define MALFUNCTION_SPEED 20

#define IGN_FUSE_WORKING_IGN_PRESENT_FACTOR 0
#define IGN_FUSE_WORKING_IGN_NOT_PRESENT_FACTOR 0.0189607f
#define IGN_FUSE_NOT_WORKING_IGN_PRESENT_FACTOR 0.0297897f
#define IGN_FUSE_NOT_WORKING_IGN_NOT_PRESENT_FACTOR 0.0383104f
#define IGN_FUSE_CHECK_EPS 0.005f

void IgnitersModule::init()
{
    initIgniterPin(m_Igniters[0], CFG_PIN_IGN_EN_1);
    initIgniterPin(m_Igniters[1], CFG_PIN_IGN_EN_2);
    initIgniterPin(m_Igniters[2], CFG_PIN_IGN_EN_3);
    initIgniterPin(m_Igniters[3], CFG_PIN_IGN_EN_4);
}

void IgnitersModule::run()
{
    gatherData();

    if (m_ADCUpdate)
    {
        m_ADCUpdate = false;

        ignUpdateContinuity();
    }

    if (m_SMSubscriber.get().state == DATALINK_SM_STATE_FREE_FALL)
    {
        if (!m_ApogeeReached)
        {
            ignFire(m_Igniters[0]);

            m_ApogeeReached = true;
        }
        if (!m_Igniters[1].fired)
        {
            if (vec3_mag_compare(&m_EKFSubscriber.get().velocity, MALFUNCTION_SPEED) >= 0)
            {
                LOG_INFO("Malfunction detected, firing backup igniter");
                LOG_INFO("Height: %.2f m", m_SMHeightSubscriber.get().height);
                LOG_INFO("Velocity: %.2f m/s", vec3_mag(&m_EKFSubscriber.get().velocity));

                ignFire(m_Igniters[1]);
            }
            else if (m_SMHeightSubscriber.get().height <= MAIN_PARACHUTE_HEIGHT)
            {
                LOG_INFO("Main parachute deployment detected, firing backup igniter");
                
                ignFire(m_Igniters[1]);
            }
        }
    }

    if (m_RPC_IGN.requestAvailable())
    {
        ignTestFire();
    }

    ignUpdate(m_Igniters[0]);
    ignUpdate(m_Igniters[1]);
    ignUpdate(m_Igniters[2]);
    ignUpdate(m_Igniters[3]);
}

void IgnitersModule::gatherData()
{
    if (m_ADCSubscriber.poll())
    {
        m_ADCUpdate = true;
    }

    if (m_BatSubscriber.poll())
    {
        m_ADCUpdate = true;
    }

    m_SMSubscriber.poll();
    m_SMHeightSubscriber.poll();
    m_EKFSubscriber.poll();
}

void IgnitersModule::initIgniterPin(IgniterPinData &data, uint8_t pin)
{
    data.pin = pin;
    data.fired = false;
    data.finished = false;

    hal_gpio_init_pin(pin, GPIO_OUTPUT);
    hal_gpio_set_pin_state(pin, GPIO_LOW);

    LOG_INFO("Igniter pin %d initialized", pin);
}

void IgnitersModule::ignTestFire()
{
    const auto &cmd = m_RPC_IGN.getRequestData();

    LOG_INFO("Received igniter test fire command for channel %d", cmd.channel);

    if (m_CurrentTestingIgniter == nullptr && cmd.channel >= 1 && cmd.channel <= PubSub::Helpers::IGN_CHANNELS_COUNT)
    {
        m_CurrentTestingIgniter = &m_Igniters[cmd.channel - 1];

        ignFire(*m_CurrentTestingIgniter);
    }
    else
    {
        LOG_WARN("Invalid igniter test fire command received or another test is currently running");
        
        m_RPC_IGN.sendResponse(false);
    }
}

void IgnitersModule::ignFire(IgniterPinData &data)
{
    if (!data.fired && !data.finished)
    {
        hal_gpio_set_pin_state(data.pin, GPIO_HIGH);

        data.fired = true;
        data.fireTime = osal_systime_get_ms();

        for (uint8_t i = 0; i < IGN_COUNT; i++)
        {
            m_CurrentIgnFiredPubData.fired[i] = m_Igniters[i].fired;
        }

        LOG_INFO("Fired igniter on pin %d", data.pin);

        m_IgnFiredPublisher.publish(m_CurrentIgnFiredPubData);
    }
}

void IgnitersModule::ignUpdate(IgniterPinData &data)
{
    if (data.fired && !data.finished)
    {
        if (osal_systime_get_ms() - data.fireTime >= IGN_UP_TIME_MS)
        {
            ignFinish(data);
        }
    }
}

void IgnitersModule::ignFinish(IgniterPinData &data)
{    
    hal_gpio_set_pin_state(data.pin, GPIO_LOW);

    if (m_CurrentTestingIgniter && m_CurrentTestingIgniter->pin == data.pin)
    {
        m_CurrentTestingIgniter = nullptr;

        data.fired = false;

        m_RPC_IGN.sendResponse(true);
    }
    else
    {
        data.finished = true;
    }

    LOG_INFO("Finished igniter on pin %d", data.pin);
}

void IgnitersModule::ignUpdateContinuity()
{
    using namespace PubSub::Helpers;

    for (uint8_t i = 0; i < IGN_COUNT; i++)
    {
        if (m_BatSubscriber.get().batPercent == 0 || (m_Igniters[i].fired && !m_Igniters[i].finished))
        {
            m_CurrentIgnContinuityPubData.detectorsFlags[i] = 0;

            continue;
        }

        float v = m_ADCSubscriber.get().volts[i];
        float vref = m_BatSubscriber.get().batVolts;
        uint8_t contFlags = 0;

        if (v < vref * (IGN_FUSE_WORKING_IGN_PRESENT_FACTOR + IGN_FUSE_CHECK_EPS))
        {
            contFlags = IgnChannelContinuityFlags::IGN_PRESENT | IgnChannelContinuityFlags::FUSE_WORKING;
        }
        else if (v < vref * (IGN_FUSE_WORKING_IGN_NOT_PRESENT_FACTOR + IGN_FUSE_CHECK_EPS))
        {
            contFlags = IgnChannelContinuityFlags::FUSE_WORKING;
        }
        else if (v < vref * (IGN_FUSE_NOT_WORKING_IGN_PRESENT_FACTOR + IGN_FUSE_CHECK_EPS))
        {
            contFlags = IgnChannelContinuityFlags::IGN_PRESENT;
        }
        else if (v < (vref * IGN_FUSE_NOT_WORKING_IGN_NOT_PRESENT_FACTOR + IGN_FUSE_CHECK_EPS))
        {
            contFlags = 0;
        }
        else
        {
            contFlags = 0;
        }

        m_CurrentIgnContinuityPubData.detectorsFlags[i] = contFlags;
    }

    m_IgnDetPublisher.publish(m_CurrentIgnContinuityPubData);
}