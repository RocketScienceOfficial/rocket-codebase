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
    PubSub::RPCHandler<PUBSUB_RPC_ID(command_set_voltage)> m_RPC;
    PubSub::Publisher<PubSub::Topics::voltage_state_topic> m_VoltageStatePublisher;

    uint8_t m_CurrentPinStates;

    void initPin(uint8_t pin);
    void setPinState(bool enable, uint8_t pin);
};