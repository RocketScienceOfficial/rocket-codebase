#include "TelemetryModule.h"
#include "modules/common/ModuleLogger.h"
#include <osal/systime.h>

#define PACKETS_WITHOUT_RESPONSE_COUNT 10
#define RADIO_TX_DONE_RECOVERY_TIME_MS 250
#define RADIO_RESPONSE_RECOVERY_TIME_MS 2000
#define RADIO_PACKET_SEND_DELAY_MS 200

void TelemetryModule::init()
{
    m_PacketTimer = osal_systime_get_ms();
    m_WaitingToSendPacket = true;
}

void TelemetryModule::run()
{
    if (m_RadioAckSubscriber.pollLatest())
    {
        handleAck();
    }
    if (m_RadioResponseSubscriber.pollLatest())
    {
        handleResponse();
    }

    m_CommanderSubscriber.pollLatest();
    m_EKFStateSubscriber.pollLatest();
    m_BatterySubscriber.pollLatest();
    m_GPSSubscriber.pollLatest();
    m_StateMachineStateSubscriber.pollLatest();
    m_VoltageStateSubscriber.pollLatest();
    m_IgnitionContinuitySubscriber.pollLatest();

    if (!m_WaitingForResponse && m_RadioTXDoneRecoveryTimeOffset != 0 && osal_systime_get_ms() - m_RadioTXDoneRecoveryTimeOffset >= RADIO_TX_DONE_RECOVERY_TIME_MS)
    {
        m_RadioTXDoneRecoveryTimeOffset = 0;

        scheduleNextPacket();

        LOG_DEBUG("Didn't receive ack from radio module");
    }

    if (m_WaitingForResponse && m_RadioResponseRecoveryTimeOffset != 0 && osal_systime_get_ms() - m_RadioResponseRecoveryTimeOffset >= RADIO_RESPONSE_RECOVERY_TIME_MS)
    {
        m_RadioResponseRecoveryTimeOffset = 0;
        m_WaitingForResponse = false;

        scheduleNextPacket();

        LOG_DEBUG("Didn't receive response from GCS");
    }

    if (m_WaitingToSendPacket && osal_systime_get_ms() - m_PacketTimer >= RADIO_PACKET_SEND_DELAY_MS)
    {
        telemetry_data_obc payload;
        payload.qw = (int16_t)(m_EKFStateSubscriber.get().orientation.w * 32767);
        payload.qx = (int16_t)(m_EKFStateSubscriber.get().orientation.x * 32767);
        payload.qy = (int16_t)(m_EKFStateSubscriber.get().orientation.y * 32767);
        payload.qz = (int16_t)(m_EKFStateSubscriber.get().orientation.z * 32767);
        payload.velocity_kmh = (uint16_t)(vec3_mag(&m_EKFStateSubscriber.get().velocity) * 3.6f);
        payload.batteryVoltage100 = (uint16_t)(m_BatterySubscriber.get().batVolts * 100);
        payload.batteryPercentage = m_BatterySubscriber.get().batPercent;
        payload.lat = (int)(m_GPSSubscriber.get().pos.lat * 10000000);
        payload.lon = (int)(m_GPSSubscriber.get().pos.lon * 10000000);
        payload.alt = packetGetAltitude();
        payload.gpsData = packetGetGPSData();
        payload.state = (uint8_t)m_StateMachineStateSubscriber.get().state;
        payload.sendResponse = m_PacketCounterForResponse == PACKETS_WITHOUT_RESPONSE_COUNT ? 1 : 0;
        payload.stateFlags = packetGetStateFlags();
        payload.cmd_seq = m_CommanderSubscriber.get().seq;
        payload.cmd_last_status = packetGetCommanderState();

        m_TelemetryTXPublisher.publish(payload);

        LOG_DEBUG("Sent telemetry packet with (awaiting response: %d)", payload.sendResponse);

        m_PacketTimer = 0;
        m_WaitingToSendPacket = false;
        m_RadioTXDoneRecoveryTimeOffset = osal_systime_get_ms();

        if (m_PacketCounterForResponse == PACKETS_WITHOUT_RESPONSE_COUNT)
        {
            m_WaitingForResponse = true;
            m_RadioResponseRecoveryTimeOffset = osal_systime_get_ms();
            m_PacketCounterForResponse = 0;
        }
        else
        {
            m_PacketCounterForResponse++;
        }
    }
}

void TelemetryModule::handleAck()
{
    m_RadioTXDoneRecoveryTimeOffset = 0;

    if (!m_WaitingForResponse)
    {
        scheduleNextPacket();
    }
}

void TelemetryModule::handleResponse()
{
    m_RadioResponseRecoveryTimeOffset = 0;
    m_WaitingForResponse = false;

    scheduleNextPacket();
}

void TelemetryModule::scheduleNextPacket()
{
    m_PacketTimer = osal_systime_get_ms();
    m_WaitingToSendPacket = true;
}

uint8_t TelemetryModule::packetGetCommanderState()
{
    using CommanderStatus = PubSub::Helpers::CommanderStatus;

    switch (m_CommanderSubscriber.get().status)
    {
    case CommanderStatus::PENDING:
        return DATALINK_TELEMETRY_CMD_STATUS_PENDING;
    case CommanderStatus::SUCCESS:
        return DATALINK_TELEMETRY_CMD_STATUS_SUCCESS;
    case CommanderStatus::FAILURE:
        return DATALINK_TELEMETRY_CMD_STATUS_FAILURE;
    default:
        return DATALINK_TELEMETRY_CMD_STATUS_PENDING;
    }
}

uint16_t TelemetryModule::packetGetAltitude()
{
    const float &tmp = -m_EKFStateSubscriber.get().position.z; // NED to ENU conversion and altitude sign change

    return tmp > 0 ? (uint16_t)tmp : 0;
}

uint8_t TelemetryModule::packetGetGPSData()
{
    return (uint8_t)m_GPSSubscriber.get().gpsIs3dFix | (m_GPSSubscriber.get().gpsSatellitesCount << 1);
}

static void packet_add_state_flag(uint8_t &flag, bool condition, uint8_t flagValue)
{
    if (condition)
    {
        flag |= flagValue;
    }
}

uint8_t TelemetryModule::packetGetStateFlags()
{
    using namespace PubSub::Helpers;

    uint8_t flags = 0;

    packet_add_state_flag(flags, m_VoltageStateSubscriber.get().pingsFlags & VoltagePinsFlags::VOLTAGE_PIN_3V3, DATALINK_FLAGS_TELEMETRY_DATA_CONTROL_3V3_ENABLED);
    packet_add_state_flag(flags, m_VoltageStateSubscriber.get().pingsFlags & VoltagePinsFlags::VOLTAGE_PIN_5V, DATALINK_FLAGS_TELEMETRY_DATA_CONTROL_5V_ENABLED);
    packet_add_state_flag(flags, m_VoltageStateSubscriber.get().pingsFlags & VoltagePinsFlags::VOLTAGE_PIN_VBAT, DATALINK_FLAGS_TELEMETRY_DATA_CONTROL_VBAT_ENABLED);
    packet_add_state_flag(flags, packetCheckIgnCont(1), DATALINK_FLAGS_TELEMETRY_DATA_CONTROL_IGN_1_CONT);
    packet_add_state_flag(flags, packetCheckIgnCont(2), DATALINK_FLAGS_TELEMETRY_DATA_CONTROL_IGN_2_CONT);
    packet_add_state_flag(flags, packetCheckIgnCont(3), DATALINK_FLAGS_TELEMETRY_DATA_CONTROL_IGN_3_CONT);
    packet_add_state_flag(flags, packetCheckIgnCont(4), DATALINK_FLAGS_TELEMETRY_DATA_CONTROL_IGN_4_CONT);

    return flags;
}

bool TelemetryModule::packetCheckIgnCont(uint8_t ign)
{
    using namespace PubSub::Helpers;

    uint8_t cont = m_IgnitionContinuitySubscriber.get().detectorsFlags[ign - 1];

    return (cont & IgnChannelContinuityFlags::IGN_PRESENT) && (cont & IgnChannelContinuityFlags::FUSE_WORKING);
}