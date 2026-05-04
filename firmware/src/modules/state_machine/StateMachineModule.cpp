#include "StateMachineModule.h"
#include "modules/common/ModuleLogger.h"
#include <osal/systime.h>
#include <lib/maths/math_utils.h>
#include <lib/geo/physical_constants.h>
#include <cmath>

#define START_ACC_THRESHOLD (3.5f * EARTH_GRAVITY)
#define START_ALT_THRESHOLD 3
#define START_ALT_VERIFICATION_COUNT 300
#define APOGEE_MAX_DELTA 2
#define LAND_MAX_DELTA 2
#define LAST_ALT_APOGEE_VERIFICATION_COUNT 200
#define LAST_ALT_LAND_VERIFICATION_COUNT 300

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

        m_CurrentBaroHeight = exp_smoothing(baro_data.baroHeight, m_CurrentBaroHeight, 0.2f);
        m_BaroHeightChanged = true;
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
}

void StateMachineModule::handle_state_armed()
{
    if (m_IMUAccChanged)
    {
        if (!m_Verifing_StandingAlt)
        {
            if (vec3_mag_compare(&m_CurrentIMUAcc, START_ACC_THRESHOLD) >= 0)
            {
                m_Verifing_StandingAlt = true;
            }
        }
    }

    if (m_BaroHeightChanged && m_Verifing_StandingAlt)
    {
        if (m_BaseAlt == 0)
        {
            m_BaseAlt = m_CurrentBaroHeight;
        }
        else
        {
            if (m_CurrentBaroHeight - m_BaseAlt >= START_ALT_THRESHOLD)
            {
                changeState(DATALINK_SM_STATE_ACCELERATING);
            }
            else
            {
                m_VerificationIndex_StandingAlt++;

                if (m_VerificationIndex_StandingAlt == START_ALT_VERIFICATION_COUNT)
                {
                    m_BaseAlt = 0;
                    m_Verifing_StandingAlt = false;
                    m_VerificationIndex_StandingAlt = 0;
                }
            }
        }
    }
}

void StateMachineModule::handle_state_accelerating()
{
    if (m_IMUAccChanged)
    {
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

        if (alt <= m_Apogee || alt - m_Apogee <= APOGEE_MAX_DELTA)
        {
            m_VerificationIndex_Apogee++;

            if (m_VerificationIndex_Apogee == LAST_ALT_APOGEE_VERIFICATION_COUNT)
            {
                changeState(DATALINK_SM_STATE_FREE_FALL);
            }
        }
        else
        {
            m_Apogee = alt;
            m_VerificationIndex_Apogee = 0;
        }
    }
}

void StateMachineModule::handle_state_free_fall()
{
    if (m_BaroHeightChanged)
    {
        float alt = m_CurrentBaroHeight - m_BaseAlt;
        float delta = fabsf(m_LandingAlt - alt);

        if (delta > LAND_MAX_DELTA)
        {
            m_LandingAlt = alt;
            m_VerificationIndex_Landing = 0;
        }
        else
        {
            m_VerificationIndex_Landing++;

            if (m_VerificationIndex_Landing == LAST_ALT_LAND_VERIFICATION_COUNT)
            {
                changeState(DATALINK_SM_STATE_LANDED);
            }
        }
    }
}

void StateMachineModule::handle_state_landed()
{
    (void)(0);
}