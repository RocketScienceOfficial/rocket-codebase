#pragma once

#include "internal/PubSubMeta.h"
#include "internal/PubSubRegister.h"
#include <lib/maths/quaternion.h>
#include <lib/maths/vector.h>
#include <lib/geo/wgs84.h>
#include <datalink.h>

#define PUBSUB_ID(name) PubSub::Topics::name##_topic
#define PUBSUB_RPC_ID(name) PubSub::Topics::name##_req_topic, PubSub::Topics::name##_res_topic

namespace PubSub
{
    namespace Helpers
    {
        inline constexpr uint8_t IGN_CHANNELS_COUNT = 4;

        enum IMUClippingFlags : uint8_t
        {
            ACC_CLIP_X = 1 << 0,
            ACC_CLIP_Y = 1 << 1,
            ACC_CLIP_Z = 1 << 2,
            GYRO_CLIP_X = 1 << 3,
            GYRO_CLIP_Y = 1 << 4,
            GYRO_CLIP_Z = 1 << 5,
        };

        enum IgnChannelContinuityFlags : uint8_t
        {
            IGN_PRESENT = 1 << 0,
            FUSE_WORKING = 1 << 1,
        };

        enum VoltagePinsFlags : uint8_t
        {
            VOLTAGE_PIN_3V3 = 1 << 0,
            VOLTAGE_PIN_5V = 1 << 1,
            VOLTAGE_PIN_VBAT = 1 << 2,
        };

        enum class CommanderStatus : uint8_t
        {
            PENDING,
            SUCCESS,
            FAILURE,
        };
    }

    namespace Messages
    {
        using DatalinkMessage = datalink_message_t;

        using TelemetryDataOBC = telemetry_data_obc;
        using TelemetryResponse = telemetry_response;

        struct LoRaRXData
        {
            datalink_message_t msg;
            int rssi;
            uint8_t sequence;
        };

        struct LoRaTXData
        {
            datalink_message_t msg;
            uint8_t sequence;
        };

        struct LoRaTXAck
        {
            uint8_t reserved;
        };

        struct SensorsIMU
        {
            float dt;
            vec3_t acc;
            vec3_t gyro;
            uint8_t clippingFlags; // Based on IMUClippingFlags enum
        };

        struct SensorsMag
        {
            vec3_t mag;
        };

        struct SensorsBaro
        {
            int press;
            float temp;
            float baroHeight;
        };

        struct SensorsGPS
        {
            geo_position_t pos;
            vec3_t vel;
            float stddev_horizontal;
            float stddev_vertical;
            float stddev_speed;
            bool gpsFix;
            bool gpsIs3dFix;
            uint8_t gpsSatellitesCount;
        };

        using SensorsSimplifiedGPS = geo_position_t;

        struct SensorsBatteryRaw
        {
            float rawVoltage;
        };

        struct SensorsBattery
        {
            float batVolts;
            uint8_t batPercent;
            uint8_t batNCells;
        };

        struct IgnContinuity
        {
            uint8_t detectorsFlags[PubSub::Helpers::IGN_CHANNELS_COUNT]; // Based on IgnChannelContinuityFlags enum
        };

        struct IgnFired
        {
            bool fired[PubSub::Helpers::IGN_CHANNELS_COUNT];
        };

        struct IgnAdcChannels
        {
            float volts[PubSub::Helpers::IGN_CHANNELS_COUNT];
        };

        struct EKFState
        {
            quat_t orientation;
            vec3_t position;
            vec3_t velocity;
        };

        struct AirbrakeState
        {
            float predictedApogee;
        };

        struct StateMachineState
        {
            state_machine_state state;
        };

        struct StateMachineHeight
        {
            float height;
        };

        struct DatabaseReady
        {
            bool ready;
        };

        struct VoltageState
        {
            uint8_t pingsFlags; // Based on VoltagePinsFlags enum
        };

        struct RadioAck
        {
            uint8_t reserved;
        };

        struct GCSRadioState
        {
            uint32_t rx;
            uint32_t tx;
        };

        struct PMUState
        {
            float batteryVoltage;
            int batteryPercentage;
        };

        struct GCSCommanderTimeout
        {
            uint8_t timeoutSec;
        };

        struct CommanderState
        {
            uint8_t seq;
            PubSub::Helpers::CommanderStatus status;
        };

        struct CommandArm
        {
            bool arm;
        };

        struct CommandSetVoltage
        {
            PubSub::Helpers::VoltagePinsFlags pin;
            bool enabled;
        };

        struct CommandIgnite
        {
            uint8_t channel;
        };
    }

    namespace Topics
    {
        PUBSUB_REGISTER_TOPIC_SIZE(Messages::DatalinkMessage, uart_rx, 4)
        PUBSUB_REGISTER_TOPIC_SIZE(Messages::DatalinkMessage, uart_tx, 4)
        PUBSUB_REGISTER_TOPIC_SIZE(Messages::DatalinkMessage, serial_rx, 4)
        PUBSUB_REGISTER_TOPIC_SIZE(Messages::DatalinkMessage, serial_tx, 4)
        PUBSUB_REGISTER_TOPIC_SIZE(Messages::DatalinkMessage, database_rx, 4)
        PUBSUB_REGISTER_TOPIC_SIZE(Messages::DatalinkMessage, database_tx, 4)

        PUBSUB_REGISTER_TOPIC(Messages::TelemetryDataOBC, telemetry_tx)
        PUBSUB_REGISTER_TOPIC(Messages::TelemetryResponse, telemetry_rx)

        PUBSUB_REGISTER_TOPIC(Messages::LoRaRXData, lora_rx)
        PUBSUB_REGISTER_TOPIC(Messages::LoRaTXData, lora_tx)
        PUBSUB_REGISTER_TOPIC(Messages::LoRaTXAck, lora_tx_ack)

        PUBSUB_REGISTER_TOPIC(Messages::SensorsIMU, sensors_imu_1)
        PUBSUB_REGISTER_TOPIC(Messages::SensorsMag, sensors_mag_1)
        PUBSUB_REGISTER_TOPIC(Messages::SensorsBaro, sensors_baro_1)
        PUBSUB_REGISTER_TOPIC(Messages::SensorsGPS, sensors_gps_1)
        PUBSUB_REGISTER_TOPIC(Messages::SensorsSimplifiedGPS, sensors_simplified_gps_1)
        PUBSUB_REGISTER_TOPIC(Messages::SensorsBatteryRaw, sensors_battery_raw)
        PUBSUB_REGISTER_TOPIC(Messages::SensorsBattery, sensors_battery)

        PUBSUB_REGISTER_TOPIC(Messages::IgnContinuity, ign_continuity)
        PUBSUB_REGISTER_TOPIC(Messages::IgnFired, ign_fired)
        PUBSUB_REGISTER_TOPIC(Messages::IgnAdcChannels, ign_adc_channels)

        PUBSUB_REGISTER_TOPIC(Messages::EKFState, ekf_state)
        PUBSUB_REGISTER_TOPIC(Messages::AirbrakeState, airbrake_state)

        PUBSUB_REGISTER_TOPIC(Messages::StateMachineState, sm_state)
        PUBSUB_REGISTER_TOPIC(Messages::StateMachineHeight, sm_height)

        PUBSUB_REGISTER_TOPIC(Messages::DatabaseReady, database_ready)

        PUBSUB_REGISTER_TOPIC(Messages::VoltageState, voltage_state)

        PUBSUB_REGISTER_TOPIC(Messages::RadioAck, radio_ack)
        PUBSUB_REGISTER_TOPIC(Messages::GCSRadioState, gcs_radio_state)

        PUBSUB_REGISTER_TOPIC(Messages::PMUState, pmu_state)

        PUBSUB_REGISTER_TOPIC(Messages::GCSCommanderTimeout, gcs_commander_timeout)
        PUBSUB_REGISTER_TOPIC(Messages::CommanderState, commander_state)

        PUBSUB_REGISTER_RPC(Messages::CommandArm, command_arm)
        PUBSUB_REGISTER_RPC(Messages::CommandSetVoltage, command_set_voltage)
        PUBSUB_REGISTER_RPC(Messages::CommandIgnite, command_ignite)
    }
}
