#include "SimUARTModule.h"
#include "sitl.h"
#include "modules/common/ModuleLogger.h"
#include <lib/debug/sys_assert.h>

SimUARTModule::~SimUARTModule()
{
    m_UARTSocket.close();
}

void SimUARTModule::init()
{
    sitl_init_godmode();

    if (m_Host == NULL)
    {
        m_UARTSocket.createServer(m_Port);
    }
    else
    {
        m_UARTSocket.createClient(m_Host, m_Port);
    }

    m_UARTSocket.setBlocking(false);
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
    if (!m_Flushed)
    {
        m_UARTSubscriber.pollLatest();
        m_Flushed = true;
        return;
    }

    while (m_UARTSubscriber.poll())
    {
        const auto &data = m_UARTSubscriber.get();

        m_UARTSocket.send(&data);
    }
}