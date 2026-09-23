#include "IgnitersModule.h"
#include "modules/common/ModuleLogger.h"
#include <osal/task.h>
#include <hal/gpio_driver.h>

IgnitersModule::IgnitersModule(hal_gpio_pin_t ign1, hal_gpio_pin_t ign2, hal_gpio_pin_t ign3, hal_gpio_pin_t ign4)
{
    for (uint8_t i = 0; i < IGN_COUNT; i++)
    {
        m_Igniters[i] = {};
    }

    m_Igniters[0].pin = ign1;
    m_Igniters[1].pin = ign2;
    m_Igniters[2].pin = ign3;
    m_Igniters[3].pin = ign4;
}

void IgnitersModule::init()
{
    for (uint8_t i = 0; i < IGN_COUNT; i++)
    {
        initIgniterPin(m_Igniters[i]);
    }
}

void IgnitersModule::run()
{
    gatherData();

    if (m_SMSubscriber.get().state == DATALINK_SM_STATE_FREE_FALL)
    {
        if (!m_ApogeeReached)
        {
            LOG_INFO("Apogee reached, firing pilot igniter");

            fireIgniter(m_Igniters[0]);

            m_ApogeeReached = true;
        }
        if (!m_Igniters[1].fired)
        {
            // Convert to ENU convention for easier reasoning (down is negative)
            const float vel_z = -m_EKFSubscriber.get().velocity.z;
            const float pos_z = -m_EKFSubscriber.get().position.z;

            bool fire = false;

            // Those cases are expected to call if they happen during apogee
            if (vel_z <= -MALFUNCTION_SPEED)
            {
                LOG_INFO("Malfunction detected, firing backup igniter");
                fire = true;
            }
            else if (pos_z <= MAIN_PARACHUTE_HEIGHT)
            {
                LOG_INFO("Main parachute deployment detected, firing backup igniter");
                fire = true;
            }

            if (fire)
            {
                LOG_INFO("Height: %.2f m", pos_z);
                LOG_INFO("Velocity: %.2f m/s", vel_z);

                fireIgniter(m_Igniters[1]);
            }
        }
    }

    if (m_RPC_IGN.requestAvailable())
    {
        testIgniter();
    }

    updateIgniter(m_Igniters[0]);
    updateIgniter(m_Igniters[1]);
    updateIgniter(m_Igniters[2]);
    updateIgniter(m_Igniters[3]);

    // After all possible ways to fire
    if (m_StatusUpdate)
    {
        m_StatusUpdate = false;

        updateFiredStatus();
    }

    updateContinuity();
}

void IgnitersModule::gatherData()
{
    m_SMSubscriber.poll();
    m_EKFSubscriber.poll();
    m_BatSubscriber.poll();
}

void IgnitersModule::updateFiredStatus()
{
    for (uint8_t i = 0; i < IGN_COUNT; i++)
    {
        m_CurrentIgnFiredPubData.fired[i] = m_Igniters[i].fired;
    }

    m_IgnFiredPublisher.publish(m_CurrentIgnFiredPubData);
}

void IgnitersModule::updateContinuity()
{
    using namespace PubSub::Helpers;

    if (!m_ADCSubscriber.poll())
    {
        if (osal_task_get_ms() - m_LastContinuityUpdateTime >= IGN_CONTINUITY_DEAD_TIME)
        {
            LOG_WARN("ADC data not available for continuity check, resetting continuity data");

            m_LastContinuityUpdateTime = osal_task_get_ms();
            m_CurrentIgnContinuityPubData = {0};
            m_IgnDetPublisher.publish(m_CurrentIgnContinuityPubData);
        }

        return;
    }

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
        else if (v < vref * (IGN_FUSE_NOT_WORKING_IGN_NOT_PRESENT_FACTOR + IGN_FUSE_CHECK_EPS))
        {
            contFlags = 0;
        }
        else
        {
            contFlags = 0; // This should not never happen, but we explicitly set it to 0 just in case
        }

        m_CurrentIgnContinuityPubData.detectorsFlags[i] = contFlags;
    }

    m_LastContinuityUpdateTime = osal_task_get_ms();
    m_IgnDetPublisher.publish(m_CurrentIgnContinuityPubData);
}

void IgnitersModule::initIgniterPin(IgniterPinData &data)
{
    data.fired = false;
    data.finished = false;

    hal_gpio_init_pin(data.pin, HAL_GPIO_OUTPUT);
    hal_gpio_set_pin_state(data.pin, HAL_GPIO_LOW);

    LOG_INFO("Igniter pin %d initialized", data.pin);
}

void IgnitersModule::testIgniter()
{
    const auto &cmd = m_RPC_IGN.getRequestData();

    LOG_INFO("Received igniter test fire command for channel %d", cmd.channel);

    if (m_SMSubscriber.get().state != DATALINK_SM_STATE_STANDING)
    {
        LOG_WARN("Cannot test fire igniter, state machine is not in STANDING state");

        m_RPC_IGN.sendResponse(false);

        return;
    }

    if (m_CurrentTestingIgniter == nullptr && cmd.channel >= 1 && cmd.channel <= PubSub::Helpers::IGN_CHANNELS_COUNT)
    {
        IgniterPinData &igniter = m_Igniters[cmd.channel - 1];

        if (igniter.fired || igniter.finished)
        {
            LOG_WARN("Cannot test fire igniter, it has already been fired or finished");

            m_RPC_IGN.sendResponse(false);

            return;
        }

        m_CurrentTestingIgniter = &igniter;

        fireIgniter(igniter);
    }
    else
    {
        LOG_WARN("Invalid igniter test fire command received or another test is currently running");

        m_RPC_IGN.sendResponse(false);
    }
}

void IgnitersModule::fireIgniter(IgniterPinData &data)
{
    if (!data.fired && !data.finished)
    {
        hal_gpio_set_pin_state(data.pin, HAL_GPIO_HIGH);

        data.fired = true;
        data.fireTime = osal_task_get_ms();

        LOG_INFO("Fired igniter on pin %d", data.pin);

        m_StatusUpdate = true;
    }
}

void IgnitersModule::updateIgniter(IgniterPinData &data)
{
    if (data.fired && !data.finished)
    {
        if (osal_task_get_ms() - data.fireTime >= IGN_UP_TIME_MS)
        {
            finishFire(data);
        }
    }
}

void IgnitersModule::finishFire(IgniterPinData &data)
{
    hal_gpio_set_pin_state(data.pin, HAL_GPIO_LOW);

    if (m_CurrentTestingIgniter && m_CurrentTestingIgniter == &data)
    {
        m_CurrentTestingIgniter = nullptr;
        m_StatusUpdate = true; // Request a status update

        data.fired = false;

        m_RPC_IGN.sendResponse(true);
    }
    else
    {
        data.finished = true;
    }

    LOG_INFO("Finished igniter on pin %d", data.pin);
}