#include "SimUARTModule.h"
#include "sitl.h"
#include "modules/common/ModuleLogger.h"
#include <lib/debug/sys_assert.h>
#include <board_config.h>

SimUARTModule::~SimUARTModule()
{
    m_UARTSocket.close();
}

void SimUARTModule::init()
{
    sitl_init_godmode();

    m_UARTSocket.createServer(CFG_SIM_UART_PORT, false);
}

void SimUARTModule::run()
{
    if (!m_UARTSocket.isActive())
    {
        return;
    }

    receive();
    sendIfAvailable();
}

void SimUARTModule::receive()
{
    datalink_message_t uartData;
    if (m_UARTSocket.receive(&uartData))
    {
        m_UARTPublisher.publish(uartData);
    }
}

void SimUARTModule::sendIfAvailable()
{
    while (m_UARTSubscriber.poll())
    {
        const auto &data = m_UARTSubscriber.get();

        m_UARTSocket.send(&data);
    }
}