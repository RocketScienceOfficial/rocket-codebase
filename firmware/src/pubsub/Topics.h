#pragma once

#include "internal/PubSubMeta.h"
#include <lib/maths/quaternion.h>
#include <lib/maths/vector.h>
#include <lib/geo/wgs84.h>
#include <datalink.h>

#define PUBSUB_TOPIC_META(name) __##name##_metadata
#define PUBSUB_ID(name) &PubSub::Topics::PUBSUB_TOPIC_META(name)
#define PUBSUB_REGISTER_TOPIC(T, name) inline constexpr PubSub::TopicMetadata<T> PUBSUB_TOPIC_META(name){sizeof(T), 0, #name};
#define PUBSUB_REGISTER_TOPIC_SIZE(T, name, depth) inline constexpr PubSub::TopicMetadata<T> PUBSUB_TOPIC_META(name){sizeof(T), depth, #name};

#define PUBSUB_RPC_TOPIC_META(name, type) __##name##_rpc_##type##_metadata
#define PUBSUB_RPC_ID(name) &PubSub::Topics::PUBSUB_RPC_TOPIC_META(name, req), &PubSub::Topics::PUBSUB_RPC_TOPIC_META(name, res)
#define PUBSUB_REGISTER_RPC(T, name)                                                                                                                        \
    inline constexpr PubSub::TopicMetadata<PubSub::RPCRequestData<T>> PUBSUB_RPC_TOPIC_META(name, req){sizeof(PubSub::RPCRequestData<T>), 0, "req_" #name}; \
    inline constexpr PubSub::TopicMetadata<PubSub::RPCResponseData> PUBSUB_RPC_TOPIC_META(name, res){sizeof(PubSub::RPCResponseData), 0, "res_" #name};

namespace PubSub
{
    namespace Helpers
    {
        static constexpr uint8_t IGN_CHANNELS_COUNT = 4;

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

    namespace Topics
    {
        using DatalinkMessage = datalink_message_t;
        PUBSUB_REGISTER_TOPIC_SIZE(DatalinkMessage, uart_rx, 4)
        PUBSUB_REGISTER_TOPIC_SIZE(DatalinkMessage, uart_tx, 4)
        PUBSUB_REGISTER_TOPIC_SIZE(DatalinkMessage, serial_rx, 4)
        PUBSUB_REGISTER_TOPIC_SIZE(DatalinkMessage, serial_tx, 4)
        PUBSUB_REGISTER_TOPIC_SIZE(DatalinkMessage, database_rx, 4)
        PUBSUB_REGISTER_TOPIC_SIZE(DatalinkMessage, database_tx, 4)

        using TelemetryDataOBC = telemetry_data_obc;
        PUBSUB_REGISTER_TOPIC(TelemetryDataOBC, telemetry_tx)

        using TelemetryResponse = telemetry_response;
        PUBSUB_REGISTER_TOPIC(TelemetryResponse, telemetry_rx)

        struct LoRaRXData
        {
            datalink_message_t msg;
            int rssi;
            uint8_t sequence;
        };
        PUBSUB_REGISTER_TOPIC(LoRaRXData, lora_rx)

        struct LoRaTXData
        {
            datalink_message_t msg;
            uint8_t sequence;
        };
        PUBSUB_REGISTER_TOPIC(LoRaTXData, lora_tx)

        struct LoRaTXAck
        {
            uint8_t reserved;
        };
        PUBSUB_REGISTER_TOPIC(LoRaTXAck, lora_tx_ack)

        struct SensorsIMU
        {
            float dt;
            vec3_t acc;
            vec3_t gyro;
            uint8_t clippingFlags; // Based on IMUClippingFlags enum
        };
        PUBSUB_REGISTER_TOPIC(SensorsIMU, sensors_imu_1)

        struct SensorsMag
        {
            vec3_t mag;
        };
        PUBSUB_REGISTER_TOPIC(SensorsMag, sensors_mag_1)

        struct SensorsBaro
        {
            int press;
            float temp;
            float baroHeight;
        };
        PUBSUB_REGISTER_TOPIC(SensorsBaro, sensors_baro_1)

        struct SensorsGPS
        {
            geo_position_t pos;
            vec3_t vel;
            float std_horizontal;
            float std_vertical;
            float std_speed;
            bool gpsFix;
            bool gpsIs3dFix;
            uint8_t gpsSatellitesCount;
        };
        PUBSUB_REGISTER_TOPIC(SensorsGPS, sensors_gps_1)

        using SensorsSimplifiedGPS = geo_position_t;
        PUBSUB_REGISTER_TOPIC(SensorsSimplifiedGPS, sensors_simplified_gps_1)

        struct SensorsBattery
        {
            float batVolts;
            uint8_t batPercent;
            uint8_t batNCells;
        };
        PUBSUB_REGISTER_TOPIC(SensorsBattery, sensors_battery)

        struct IgnContinuity
        {
            uint8_t detectorsFlags[PubSub::Helpers::IGN_CHANNELS_COUNT]; // Based on IgnChannelContinuityFlags enum
        };
        PUBSUB_REGISTER_TOPIC(IgnContinuity, ign_continuity)

        struct IgnFired
        {
            bool fired[PubSub::Helpers::IGN_CHANNELS_COUNT];
        };
        PUBSUB_REGISTER_TOPIC(IgnFired, ign_fired)

        struct IgnAdcChannels
        {
            float volts[PubSub::Helpers::IGN_CHANNELS_COUNT];
        };
        PUBSUB_REGISTER_TOPIC(IgnAdcChannels, ign_adc_channels)

        struct EKFState
        {
            quat_t orientation;
            vec3_t position;
            vec3_t velocity;
        };
        PUBSUB_REGISTER_TOPIC(EKFState, ekf_state)

        struct AirbrakeState
        {
            float predictedApogee;
        };
        PUBSUB_REGISTER_TOPIC(AirbrakeState, airbrake_state)

        struct StateMachineState
        {
            state_machine_state state;
        };
        PUBSUB_REGISTER_TOPIC(StateMachineState, sm_state)

        struct StateMachineHeight
        {
            float height;
        };
        PUBSUB_REGISTER_TOPIC(StateMachineHeight, sm_height)

        struct DatabaseReady
        {
            bool ready;
        };
        PUBSUB_REGISTER_TOPIC(DatabaseReady, database_ready)

        struct VoltageState
        {
            uint8_t pingsFlags; // Based on VoltagePinsFlags enum
        };
        PUBSUB_REGISTER_TOPIC(VoltageState, voltage_state)

        struct RadioAck
        {
            uint8_t reserved;
        };
        PUBSUB_REGISTER_TOPIC(RadioAck, radio_ack)

        struct GCSRadioState
        {
            uint32_t rx;
            uint32_t tx;
        };
        PUBSUB_REGISTER_TOPIC(GCSRadioState, gcs_radio_state)

        struct PMUState
        {
            float batteryVoltage;
            int batteryPercentage;
        };
        PUBSUB_REGISTER_TOPIC(PMUState, pmu_state)

        struct GCSCommanderTimeout
        {
            uint8_t timeoutSec;
        };
        PUBSUB_REGISTER_TOPIC(GCSCommanderTimeout, gcs_commander_timeout)

        struct CommanderState
        {
            uint8_t seq;
            PubSub::Helpers::CommanderStatus status;
        };
        PUBSUB_REGISTER_TOPIC(CommanderState, commander_state)

        struct CommandArm
        {
            bool arm;
        };
        PUBSUB_REGISTER_RPC(CommandArm, command_arm)

        struct CommandSetVoltage
        {
            PubSub::Helpers::VoltagePinsFlags pin;
            bool enabled;
        };
        PUBSUB_REGISTER_RPC(CommandSetVoltage, command_set_voltage)

        struct CommandIgnite
        {
            uint8_t channel;
        };
        PUBSUB_REGISTER_RPC(CommandIgnite, command_ignite)
    }
}