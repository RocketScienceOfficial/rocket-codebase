#include "DatabaseModule.h"
#include "modules/common/ModuleLogger.h"
#include <hal/time_driver.h>

void DatabaseModule::init()
{
    m_MetadataController.read();

    setState(DatamanState::STANDING_WRITE);
}

void DatabaseModule::run()
{
    gatherData();

    switch (m_CurrentState)
    {
    case DatamanState::READ:
        handle_state_read();
        break;
    case DatamanState::RECOVER:
        handle_state_recover();
        break;
    case DatamanState::CLEAR:
        handle_state_clear();
        break;
    case DatamanState::STANDING_WRITE:
        handle_state_standing_write();
        break;
    case DatamanState::FLIGHT_WRITE:
        handle_state_flight_write();
        break;
    case DatamanState::LANDED_WRITE:
        handle_state_landed_write();
        break;
    case DatamanState::TERMINATED:
        return;
    }
}

void DatabaseModule::gatherData()
{
    if (m_StateMachineStateSubscriber.poll())
    {
        m_CurrentFlightState = m_StateMachineStateSubscriber.get().state;

        if (m_CurrentFlightState == DATALINK_SM_STATE_STANDING || m_CurrentFlightState == DATALINK_SM_STATE_ARMED)
        {
            setState(DatamanState::STANDING_WRITE);
        }
        else if (m_CurrentFlightState == DATALINK_SM_STATE_ACCELERATING || m_CurrentFlightState == DATALINK_SM_STATE_FREE_FLIGHT || m_CurrentFlightState == DATALINK_SM_STATE_FREE_FALL)
        {
            setState(DatamanState::FLIGHT_WRITE);
        }
        else if (m_CurrentFlightState == DATALINK_SM_STATE_LANDED)
        {
            setState(DatamanState::LANDED_WRITE);
        }
    }

    if (m_RXSubscriber.poll())
    {
        if (m_CurrentFlightState == DATALINK_SM_STATE_STANDING)
        {
            switch (m_RXSubscriber.get().msg_id)
            {
            case DATALINK_MESSAGE_ID_READ_REQUEST:
                setState(DatamanState::READ);
                break;
            case DATALINK_MESSAGE_ID_RECOVERY_REQUEST:
                setState(DatamanState::RECOVER);
                break;
            case DATALINK_MESSAGE_ID_CLEAR_REQUEST:
                setState(DatamanState::CLEAR);
                break;
            }
        }
    }

    m_IMUSubscriber.pollLatest();
    m_MagSubscriber.pollLatest();
    m_GPSSubscriber.pollLatest();
    m_BarometerSubscriber.pollLatest();
    m_BatterySubscriber.pollLatest();
    m_IgnContinuitySubscriber.pollLatest();
    m_IgnFiredSubscriber.pollLatest();
    m_EKFSubscriber.pollLatest();
}

void DatabaseModule::setState(DatamanState newState)
{
    if (m_CurrentState != newState)
    {
        m_CurrentState = newState;

        LOG_DEBUG("New state: %d", (int)m_CurrentState);
    }
}

DatabaseFrame DatabaseModule::getFrame(uint16_t dt_us)
{
    using namespace PubSub::Helpers;

    uint8_t ignFlags = 0;

    for (size_t i = 0; i < IGN_CHANNELS_COUNT; i++)
    {
        uint8_t flag_det = m_IgnContinuitySubscriber.get().detectorsFlags[i];
        uint8_t val_det = (flag_det & IgnChannelContinuityFlags::FUSE_WORKING) && (flag_det & IgnChannelContinuityFlags::IGN_PRESENT) ? 1 << i : 0;
        ignFlags |= val_det;

        uint8_t flag_fired = m_IgnFiredSubscriber.get().fired[i];
        uint8_t val_fired = flag_fired ? 1 << (i + IGN_CHANNELS_COUNT) : 0;
        ignFlags |= val_fired;
    }

    DatabaseFrame frame = {
        .dt_us = dt_us,
        .accRaw = m_IMUSubscriber.get().acc,
        .gyroRaw = m_IMUSubscriber.get().gyro,
        .magRaw = m_MagSubscriber.get().mag,
        .gpsPos = m_GPSSubscriber.get().pos,
        .gpsData = (uint8_t)((uint8_t)m_GPSSubscriber.get().gpsIs3dFix | (m_GPSSubscriber.get().gpsSatellitesCount << 1)),
        .pressure = m_BarometerSubscriber.get().press,
        .accNED = m_EKFSubscriber.get().acceleration,
        .velNED = m_EKFSubscriber.get().velocity,
        .posNED = m_EKFSubscriber.get().position,
        .qNED = m_EKFSubscriber.get().orientation,
        .smState = (uint8_t)m_CurrentFlightState,
        .batteryVoltage100 = (uint8_t)(m_BatterySubscriber.get().batVolts * 100),
        .ignFlags = ignFlags,
    };
    return frame;
}

void DatabaseModule::handle_state_read()
{
    m_Reader.setRecoveryMode(false);
    m_Reader.update();

    if (m_Reader.isFinished())
    {
        setState(DatamanState::STANDING_WRITE);
    }
}

void DatabaseModule::handle_state_recover()
{
    m_Reader.setRecoveryMode(true);
    m_Reader.update();

    if (m_Reader.isFinished())
    {
        setState(DatamanState::STANDING_WRITE);
    }
}

void DatabaseModule::handle_state_clear()
{
    m_Cleaner.update();

    if (m_Cleaner.isFinished())
    {
        setState(DatamanState::STANDING_WRITE);
    }
}

void DatabaseModule::handle_state_standing_write()
{
    uint32_t currentTime = hal_time_get_us_since_boot();

    if (currentTime - m_LastSaveTime >= DATA_SAVE_RATE_US)
    {
        uint16_t dt_us = currentTime - m_LastSaveTime;
        m_LastSaveTime = currentTime;

        m_Writer.saveStandingFrame(getFrame(dt_us));
    }
}

void DatabaseModule::handle_state_flight_write()
{
    uint32_t currentTime = hal_time_get_us_since_boot();

    if (currentTime - m_LastSaveTime >= DATA_SAVE_RATE_US)
    {
        uint16_t dt_us = currentTime - m_LastSaveTime;
        m_LastSaveTime = currentTime;

        m_Writer.saveFrame(getFrame(dt_us));
    }
}

void DatabaseModule::handle_state_landed_write()
{
    uint32_t currentTime = hal_time_get_us_since_boot();

    if (currentTime - m_LastSaveTime >= DATA_SAVE_RATE_US)
    {
        uint16_t dt_us = currentTime - m_LastSaveTime;
        m_LastSaveTime = currentTime;
        m_LandingBufferIndex++;

        m_Writer.saveFrame(getFrame(dt_us));

        if (m_LandingBufferIndex == LANDING_BUFFER_LENGTH)
        {
            m_Writer.flush();

            setState(DatamanState::TERMINATED);
        }
    }
}