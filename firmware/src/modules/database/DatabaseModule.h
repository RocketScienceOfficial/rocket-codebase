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
    PubSub::Publisher<PubSub::Topics::DatabaseReady> m_ReadyPublisher{PUBSUB_ID(database_ready)};
    PubSub::Publisher<PubSub::Topics::DatalinkMessage> m_TXPublisher{PUBSUB_ID(database_tx)};
    PubSub::Subscriber<PubSub::Topics::DatalinkMessage> m_RXSubscriber{PUBSUB_ID(database_rx)};
    PubSub::Subscriber<PubSub::Topics::StateMachineState> m_StateMachineStateSubscriber{PUBSUB_ID(sm_state)};
    PubSub::Subscriber<PubSub::Topics::SensorsIMU> m_IMUSubscriber{PUBSUB_ID(sensors_imu_1)};
    PubSub::Subscriber<PubSub::Topics::SensorsMag> m_MagSubscriber{PUBSUB_ID(sensors_mag_1)};
    PubSub::Subscriber<PubSub::Topics::SensorsGPS> m_GPSSubscriber{PUBSUB_ID(sensors_gps_1)};
    PubSub::Subscriber<PubSub::Topics::SensorsBaro> m_BarometerSubscriber{PUBSUB_ID(sensors_baro_1)};
    PubSub::Subscriber<PubSub::Topics::SensorsBattery> m_BatterySubscriber{PUBSUB_ID(sensors_battery)};
    PubSub::Subscriber<PubSub::Topics::IgnContinuity> m_IgnContinuitySubscriber{PUBSUB_ID(ign_continuity)};
    PubSub::Subscriber<PubSub::Topics::IgnFired> m_IgnFiredSubscriber{PUBSUB_ID(ign_fired)};
    PubSub::Subscriber<PubSub::Topics::EKFState> m_EKFSubscriber{PUBSUB_ID(ekf_state)};

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

    DatamanState m_CurrentState;
    state_machine_state m_CurrentFlightState;
    uint32_t m_LastSaveTime;
    size_t m_LandingBufferIndex;

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