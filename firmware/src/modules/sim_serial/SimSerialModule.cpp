#include "SimSerialModule.h"
#include "sitl.h"
#include "modules/common/ModuleLogger.h"
#include <lib/debug/sys_assert.h>

#define SERIAL_ENGINE_PORT 12347

SimSerialModule::~SimSerialModule()
{
    m_SerialSocket.close();
}

void SimSerialModule::init()
{
    sitl_init_godmode();

    m_SerialSocket.createServer(SERIAL_ENGINE_PORT, false);
}

void SimSerialModule::run()
{
    if (!m_SerialSocket.isActive())
    {
        return;
    }

    receive();
    sendIfAvailable();
}

void SimSerialModule::receive()
{
    datalink_message_t serialData;
    if (m_SerialSocket.receive(&serialData))
    {
        m_SerialPublisher.publish(serialData);
    }
}

void SimSerialModule::sendIfAvailable()
{
    while (m_SerialSubscriber.poll())
    {
        const auto &data = m_SerialSubscriber.get();

        m_SerialSocket.send(&data);
    }
}