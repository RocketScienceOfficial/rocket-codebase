#pragma once

#include <datalink.h>
#include <pubsub/Topics.h>
#include <pubsub/Publisher.h>
#include <pubsub/Subscriber.h>
#include <pubsub/RPCRequest.h>
#include <cstdint>

using CommanderStatus = PubSub::Helpers::CommanderStatus;

class CommanderOBCModule
{
public:
    void init();
    void run();

private:
    PubSub::Subscriber<PubSub::Topics::serial_rx_topic> m_SerialSubscriber;
    PubSub::Publisher<PubSub::Topics::serial_tx_topic> m_SerialPublisher;
    PubSub::Subscriber<PubSub::Topics::uart_rx_topic> m_UARTSubscriber;
    PubSub::Publisher<PubSub::Topics::uart_tx_topic> m_UARTPublisher;

    PubSub::Subscriber<PubSub::Topics::database_tx_topic> m_DatabaseSubscriber;
    PubSub::Publisher<PubSub::Topics::database_rx_topic> m_DatabasePublisher;

    PubSub::Subscriber<PubSub::Topics::telemetry_tx_topic> m_TelemetrySubscriber;
    PubSub::Publisher<PubSub::Topics::telemetry_rx_topic> m_TelemetryResponsePublisher;
    PubSub::Publisher<PubSub::Topics::radio_ack_topic> m_RadioACKPublisher;

    PubSub::RPCRequest<PUBSUB_RPC_ID(command_arm)> m_RPC_ARM;
    PubSub::RPCRequest<PUBSUB_RPC_ID(command_set_voltage)> m_RPC_Voltage;
    PubSub::RPCRequest<PUBSUB_RPC_ID(command_ignite)> m_RPC_IGN;
    PubSub::Publisher<PubSub::Topics::commander_state_topic> m_CommanderRadioRPCStatePublisher;

    uint8_t m_RadioCommandSeq = 0;
    CommanderStatus m_RadioCommandStatus = CommanderStatus::SUCCESS;
    uint32_t m_LastRadioCommandTime = 0;

    void handleRPCs();
    void processSerialMessage(const datalink_message_t &msg);
    void processUARTMessage(const datalink_message_t &msg);
    void executeRadioCommand(uint8_t cmd);
    void setRadioRPCStatus(bool success);
    void updateState();
};