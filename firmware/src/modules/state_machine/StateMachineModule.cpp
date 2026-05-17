#include "StateMachineModule.h"
#include "SMConfig.h"
#include "modules/common/ModuleLogger.h"
#include <osal/systime.h>
#include <lib/maths/math_utils.h>
#include <lib/geo/physical_constants.h>
#include <cmath>

void StateMachineModule::init()
{
}

void StateMachineModule::run()
{
    updateData();

    if (m_State == DATALINK_SM_STATE_STANDING)
    {
        handle_state_standing();
    }
    else if (m_State == DATALINK_SM_STATE_ARMED)
    {
        handle_state_armed();
    }
    else if (m_State == DATALINK_SM_STATE_ACCELERATING)
    {
        handle_state_accelerating();
    }
    else if (m_State == DATALINK_SM_STATE_FREE_FLIGHT)
    {
        handle_state_free_flight();
    }
    else if (m_State == DATALINK_SM_STATE_FREE_FALL)
    {
        handle_state_free_fall();
    }
    else if (m_State == DATALINK_SM_STATE_LANDED)
    {
        handle_state_landed();
    }

    postUpdate();
}

void StateMachineModule::updateData()
{
    if (m_RPC_ARM.requestAvailable())
    {
        const auto &req = m_RPC_ARM.getRequestData();
        bool valid = false;

        if (m_State == DATALINK_SM_STATE_STANDING && req.arm)
        {
            LOG_INFO("ARM");

            changeState(DATALINK_SM_STATE_ARMED);
            valid = true;
        }
        else if (m_State == DATALINK_SM_STATE_ARMED && !req.arm)
        {
            LOG_INFO("DISARM");

            changeState(DATALINK_SM_STATE_STANDING);
            valid = true;
        }

        m_RPC_ARM.sendResponse(valid);
    }

    if (m_BaroDataSubscriber.poll())
    {
        const auto &baro_data = m_BaroDataSubscriber.get();

        m_CurrentBaroHeight = exp_smoothing(baro_data.baroHeight, m_CurrentBaroHeight, SM_CFG_BARO_SMOOTHING_ALPHA);
        m_BaroHeightChanged = true;

        if (!m_StartupBaseAltSet)
        {
            m_StartupBaseAlt = m_CurrentBaroHeight;
            m_StartupBaseAltSet = true;

            LOG_INFO("Startup base altitude set to %.2f", m_StartupBaseAlt);
        }
    }

    if (m_IMUDataSubscriber.poll())
    {
        const auto &imu_data = m_IMUDataSubscriber.get();

        m_CurrentIMUAcc = imu_data.acc;
        m_IMUAccChanged = true;
    }
}

void StateMachineModule::postUpdate()
{
    m_FlightStateHeightPublisher.publish({m_CurrentBaroHeight - m_BaseAlt});

    m_BaroHeightChanged = false;
    m_IMUAccChanged = false;
}

void StateMachineModule::changeState(state_machine_state new_state)
{
    m_State = new_state;

    LOG_INFO("State changed to %d (Time = %dms)", m_State, osal_systime_get_ms());

    m_FlightStatePublisher.publish({.state = m_State});
}

void StateMachineModule::handle_state_standing()
{
    (void)(0);
}

void StateMachineModule::handle_state_armed()
{
    if (m_IMUAccChanged)
    {
        if (!m_VerifyingStandingAlt)
        {
            if (vec3_mag_compare(&m_CurrentIMUAcc, SM_CFG_START_ACC_THRESHOLD) >= 0)
            {
                m_VerificationStartTime = osal_systime_get_ms();
                m_VerifyingStandingAlt = true;

                LOG_INFO("Acceleration threshold exceeded. Starting standing altitude verification.");
            }
        }
    }

    if (m_BaroHeightChanged)
    {
        if (m_VerifyingStandingAlt)
        {
            if (m_BaseAlt == 0)
            {
                m_BaseAlt = m_CurrentBaroHeight;
            }
            else
            {
                if (m_CurrentBaroHeight - m_BaseAlt >= SM_CFG_START_ALT_THRESHOLD)
                {
                    changeState(DATALINK_SM_STATE_ACCELERATING);
                }
                else
                {
                    if (osal_systime_get_ms() - m_VerificationStartTime > SM_CFG_START_ALT_VERIFICATION_TIME_MS)
                    {
                        m_BaseAlt = 0;
                        m_VerificationStartTime = 0;
                        m_VerifyingStandingAlt = false;

                        LOG_INFO("Failed to verify standing altitude. Resetting base altitude and verification process.");
                    }
                }
            }
        }
        else if (m_StartupBaseAltSet && m_CurrentBaroHeight - m_StartupBaseAlt >= SM_CFG_START_ALT_FALLBACK_HEIGHT)
        {
            LOG_INFO("Fallback altitude threshold exceeded. Starting flight.");

            changeState(DATALINK_SM_STATE_ACCELERATING);
        }
    }
}

void StateMachineModule::handle_state_accelerating()
{
    if (m_IMUAccChanged)
    {
        // NOTE: This may fail if aerodynamic drag is higher than 1g (it will just detect free flight when thrust = drag + 1g)
        if (vec3_mag_compare(&m_CurrentIMUAcc, EARTH_GRAVITY) < 0)
        {
            changeState(DATALINK_SM_STATE_FREE_FLIGHT);
        }
    }
}

void StateMachineModule::handle_state_free_flight()
{
    if (m_BaroHeightChanged)
    {
        float alt = m_CurrentBaroHeight - m_BaseAlt;

        if (alt <= m_Apogee || alt - m_Apogee <= SM_CFG_APOGEE_MAX_DELTA)
        {
            if (osal_systime_get_ms() - m_VerificationStartTime > SM_CFG_LAST_ALT_APOGEE_TIME_MS)
            {
                m_VerificationStartTime = 0;

                changeState(DATALINK_SM_STATE_FREE_FALL);
            }
        }
        else
        {
            m_Apogee = alt;
            m_VerificationStartTime = osal_systime_get_ms();
        }
    }
}

void StateMachineModule::handle_state_free_fall()
{
    if (m_BaroHeightChanged)
    {
        float alt = m_CurrentBaroHeight - m_BaseAlt;
        float delta = fabsf(m_LandingAlt - alt);

        if (delta > SM_CFG_LAND_MAX_DELTA)
        {
            m_LandingAlt = alt;
            m_VerificationStartTime = osal_systime_get_ms();
        }
        else
        {
            if (osal_systime_get_ms() - m_VerificationStartTime > SM_CFG_LAST_ALT_LAND_VERIFICATION_TIME_MS)
            {
                m_VerificationStartTime = 0;
                
                changeState(DATALINK_SM_STATE_LANDED);
            }
        }
    }
}

void StateMachineModule::handle_state_landed()
{
    (void)(0);
}