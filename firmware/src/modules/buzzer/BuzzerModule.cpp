#include "BuzzerModule.h"
#include "modules/common/ModuleLogger.h"
#include <board_config.h>
#include <osal/systime.h>

void BuzzerModule::init()
{
    hal_pwm_init_pin(&m_Device, CFG_PIN_BUZZER);

    setTone(Tone::START);
}

void BuzzerModule::run()
{
    if (m_GPSSubscriber.poll())
    {
        if (m_GPSSubscriber.get().gpsFix)
        {
            if (!m_GPSPlayed)
            {
                m_GPSPlayed = true;

                setTone(Tone::GPS_FIX);
            }
        }
        else
        {
            m_GPSPlayed = false;
        }
    }

    if (m_SMSubscriber.poll())
    {
        if (m_SMSubscriber.get().state == DATALINK_SM_STATE_STANDING)
        {
            setTone(Tone::DISARM);
        }
        if (m_SMSubscriber.get().state == DATALINK_SM_STATE_ARMED)
        {
            setTone(Tone::ARM);
        }
        if (m_SMSubscriber.get().state == DATALINK_SM_STATE_LANDED)
        {
            setTone(Tone::LANDED);
        }
    }

    if (m_CurrentTone == NULL)
    {
        if (m_SMSubscriber.get().state == DATALINK_SM_STATE_LANDED)
        {
            setTone(Tone::LANDED);
        }
    }

    if (m_CurrentTone != NULL)
    {
        if (osal_systime_get_ms() - m_BuzzerToneStartTime >= m_CurrentTone->duration_ms)
        {
            if (m_CurrentToneSize > 1)
            {
                m_CurrentTone++;
                m_CurrentToneSize--;
            }
            else
            {
                m_CurrentTone = NULL;
            }

            playCurrentTone();
        }
    }
}

void BuzzerModule::setTone(Tone tone)
{
    if (m_CurrentTone != NULL)
    {
        return;
    }

    switch (tone)
    {
    case Tone::START:
        m_CurrentTone = s_StartupMusic;
        m_CurrentToneSize = sizeof(s_StartupMusic) / sizeof(BuzzerTone);
        break;
    case Tone::ARM:
        m_CurrentTone = s_ArmMusic;
        m_CurrentToneSize = sizeof(s_ArmMusic) / sizeof(BuzzerTone);
        break;
    case Tone::DISARM:
        m_CurrentTone = s_DisarmMusic;
        m_CurrentToneSize = sizeof(s_DisarmMusic) / sizeof(BuzzerTone);
        break;
    case Tone::GPS_FIX:
        m_CurrentTone = s_GPSFixMusic;
        m_CurrentToneSize = sizeof(s_GPSFixMusic) / sizeof(BuzzerTone);
        break;
    case Tone::LANDED:
        m_CurrentTone = s_LandedMusic;
        m_CurrentToneSize = sizeof(s_LandedMusic) / sizeof(BuzzerTone);
        break;
    default:
        return;
    }

    LOG_INFO("Playing buzzer tone: %d", (int)tone);

    playCurrentTone();
}

void BuzzerModule::playCurrentTone()
{
    if (m_CurrentTone == NULL || m_CurrentTone->frequency == 0)
    {
        hal_pwm_set_duty(&m_Device, 0);

        LOG_DEBUG("Silence for %d ms", m_CurrentTone != NULL ? m_CurrentTone->duration_ms : -1);
    }
    else
    {
        hal_pwm_set_frequency(&m_Device, m_CurrentTone->frequency);
        hal_pwm_set_duty(&m_Device, 1e6f * BUZZER_DUTY_CYCLE_RATIO / m_CurrentTone->frequency);

        LOG_DEBUG("Playing tone with frequency %d Hz for %d ms", m_CurrentTone->frequency, m_CurrentTone->duration_ms);
    }

    m_BuzzerToneStartTime = osal_systime_get_ms();
}