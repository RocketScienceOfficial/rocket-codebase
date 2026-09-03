#pragma once

#include <pubsub/Topics.h>
#include <pubsub/Subscriber.h>
#include <pubsub/Publisher.h>
#include <pubsub/RPCHandler.h>
#include <hal/gpio_driver.h>
#include <cstdint>

class VoltageModule
{
public:
    VoltageModule(hal_gpio_pin_t pin3V3, hal_gpio_pin_t pin5V, hal_gpio_pin_t pinVBAT) : m_Pin3V3(pin3V3), m_Pin5V(pin5V), m_PinVBAT(pinVBAT) {}

    void init();
    void run();

private:
    PubSub::RPCHandler<PUBSUB_RPC_ID(command_set_voltage)> m_RPC;
    PubSub::Publisher<PubSub::Topics::voltage_state_topic> m_VoltageStatePublisher;

    const hal_gpio_pin_t m_Pin3V3;
    const hal_gpio_pin_t m_Pin5V;
    const hal_gpio_pin_t m_PinVBAT;

    uint8_t m_CurrentPinStates = 0;

    void initPin(hal_gpio_pin_t pin);
    void setPinState(bool enable, hal_gpio_pin_t pin);
};