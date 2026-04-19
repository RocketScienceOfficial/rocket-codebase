#pragma once

#include <pubsub/Topics.h>
#include <pubsub/Subscriber.h>
#include <pubsub/Publisher.h>
#include <pubsub/RPCHandler.h>
#include <cstdint>

class VoltageModule
{
public:
    void init();
    void run();

private:
    PubSub::RPCHandler<PubSub::Topics::CommandSetVoltage> m_RPC{PUBSUB_RPC_ID(command_set_voltage)};
    PubSub::Publisher<PubSub::Topics::VoltageState> m_VoltageStatePublisher{PUBSUB_ID(voltage_state)};

    uint8_t m_CurrentPinStates;

    void initPin(uint8_t pin);
    void setPinState(bool enable, uint8_t pin);
};