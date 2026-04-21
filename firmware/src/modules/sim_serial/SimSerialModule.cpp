#include "SimSerialModule.h"
#include "sitl.h"
#include "modules/common/ModuleLogger.h"
#include <lib/debug/sys_assert.h>
#include <board_config.h>

SimSerialModule::~SimSerialModule()
{
    m_SerialSocket.close();
}

void SimSerialModule::init()
{
    sitl_init_godmode();

#ifndef CFG_SIM_SERIAL_SERVER
    m_SerialSocket.createServer(CFG_SIM_SERIAL_PORT);
#else
    m_SerialSocket.createClient(CFG_SIM_SERIAL_SERVER, CFG_SIM_SERIAL_PORT);
#endif

    m_SerialSocket.setBlocking(false);
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
    if (!m_Flushed)
    {
        m_SerialSubscriber.pollLatest();
        m_Flushed = true;
        return;
    }
    
    while (m_SerialSubscriber.poll())
    {
        const auto &data = m_SerialSubscriber.get();

        m_SerialSocket.send(&data);
    }
}