#pragma once

#include "DatabaseMetadataController.h"
#include "DatabaseWriter.h"
#include "DatabaseReader.h"
#include "DatabaseCleaner.h"
#include <pubsub/Topics.h>
#include <pubsub/Publisher.h>
#include <pubsub/Subscriber.h>
#include <cstdint>
#include <cstddef>

class DatabaseModule
{
public:
    void init();
    void run();

private:
    PubSub::Publisher<PubSub::Topics::database_ready_topic> m_ReadyPublisher;
    PubSub::Publisher<PubSub::Topics::database_tx_topic> m_TXPublisher;
    PubSub::Subscriber<PubSub::Topics::database_rx_topic> m_RXSubscriber;
    PubSub::Subscriber<PubSub::Topics::sm_state_topic> m_StateMachineStateSubscriber;
    PubSub::Subscriber<PubSub::Topics::sensors_imu_1_topic> m_IMUSubscriber;
    PubSub::Subscriber<PubSub::Topics::sensors_mag_1_topic> m_MagSubscriber;
    PubSub::Subscriber<PubSub::Topics::sensors_gps_1_topic> m_GPSSubscriber;
    PubSub::Subscriber<PubSub::Topics::sensors_baro_1_topic> m_BarometerSubscriber;
    PubSub::Subscriber<PubSub::Topics::sensors_battery_topic> m_BatterySubscriber;
    PubSub::Subscriber<PubSub::Topics::ign_continuity_topic> m_IgnContinuitySubscriber;
    PubSub::Subscriber<PubSub::Topics::ign_fired_topic> m_IgnFiredSubscriber;
    PubSub::Subscriber<PubSub::Topics::ekf_state_topic> m_EKFSubscriber;

    enum class DatamanState
    {
        READ,
        RECOVER,
        CLEAR,
        STANDING_WRITE,
        FLIGHT_WRITE,
        LANDED_WRITE,
        TERMINATED
    };

    DatamanState m_CurrentState = DatamanState::LANDED_WRITE;
    state_machine_state m_CurrentFlightState = DATALINK_SM_STATE_STANDING;
    uint64_t m_LastSaveTime = 0;
    size_t m_LandingBufferIndex = 0;

    DatabaseMetadataController m_MetadataController{m_ReadyPublisher};
    DatabaseWriter m_Writer{m_MetadataController};
    DatabaseReader m_Reader{m_TXPublisher, m_MetadataController};
    DatabaseCleaner m_Cleaner{m_TXPublisher, m_MetadataController};

    void gatherData();
    void setState(DatamanState newState);
    DatabaseFrame getFrame(uint16_t dt_us);

    void handle_state_read();
    void handle_state_recover();
    void handle_state_clear();
    void handle_state_standing_write();
    void handle_state_flight_write();
    void handle_state_landed_write();
};