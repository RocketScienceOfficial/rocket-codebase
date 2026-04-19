#pragma once

#include <datalink.h>
#include <pubsub/Topics.h>
#include <pubsub/Publisher.h>
#include <pubsub/Subscriber.h>
#include <pubsub/RPCRequest.h>
#include <cstdint>

using CommanderStatus = PubSub::Helpers::CommanderStatus;

class CommanderModule
{
public:
    void init();
    void run();

private:
    PubSub::Subscriber<PubSub::Topics::DatalinkMessage> m_SerialSubscriber{PUBSUB_ID(serial_rx)};
    PubSub::Publisher<PubSub::Topics::DatalinkMessage> m_SerialPublisher{PUBSUB_ID(serial_tx)};
    PubSub::Subscriber<PubSub::Topics::DatalinkMessage> m_RadioSubscriber{PUBSUB_ID(radio_rx)};
    PubSub::Publisher<PubSub::Topics::DatalinkMessage> m_RadioPublisher{PUBSUB_ID(radio_tx)};

    PubSub::Subscriber<PubSub::Topics::DatalinkMessage> m_DatabaseSubscriber{PUBSUB_ID(database_tx)};
    PubSub::Publisher<PubSub::Topics::DatalinkMessage> m_DatabasePublisher{PUBSUB_ID(database_rx)};

    PubSub::Subscriber<PubSub::Topics::TelemetryDataOBC> m_TelemetrySubscriber{PUBSUB_ID(telemetry_tx)};
    PubSub::Publisher<PubSub::Topics::TelemetryResponse> m_TelemetryResponsePublisher{PUBSUB_ID(telemetry_rx)};
    PubSub::Publisher<PubSub::Topics::RadioAck> m_RadioACKPublisher{PUBSUB_ID(radio_ack)};

    PubSub::RPCRequest<PubSub::Topics::CommandArm> m_RPC_ARM{PUBSUB_RPC_ID(command_arm)};
    PubSub::RPCRequest<PubSub::Topics::CommandSetVoltage> m_RPC_Voltage{PUBSUB_RPC_ID(command_set_voltage)};
    PubSub::RPCRequest<PubSub::Topics::CommandIgnite> m_RPC_IGN{PUBSUB_RPC_ID(command_ignite)};
    PubSub::Publisher<PubSub::Topics::CommanderState> m_CommanderRadioRPCStatePublisher{PUBSUB_ID(commander_state)};

    uint8_t m_RadioCommandSeq = 0;
    CommanderStatus m_RadioCommandStatus = CommanderStatus::SUCCESS;
    uint32_t m_LastRadioCommandTime = 0;

    void handleRPCs();
    void processSerialMessage(const datalink_message_t &msg);
    void processRadioMessage(const datalink_message_t &msg);
    void executeRadioCommand(uint8_t cmd);
    void setRadioRPCStatus(bool success);
    void updateState();
};