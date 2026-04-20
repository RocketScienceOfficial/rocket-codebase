#include "SimRadioModule.h"
#include "sitl.h"
#include "modules/common/ModuleLogger.h"
#include <lib/debug/sys_assert.h>
#include <board_config.h>

SimRadioModule::~SimRadioModule()
{
    m_RadioSocket.close();
}

void SimRadioModule::init()
{
    sitl_init_godmode();

    m_RadioSocket.createServer(CFG_SIM_RADIO_PORT, false);
}

void SimRadioModule::run()
{
    if (!m_RadioSocket.isActive())
    {
        return;
    }

    receive();
    sendIfAvailable();
}

void SimRadioModule::receive()
{
    datalink_message_t radioData;
    if (m_RadioSocket.receive(&radioData))
    {
        m_RadioPublisher.publish(radioData);
    }
}

void SimRadioModule::sendIfAvailable()
{
    while (m_RadioSubscriber.poll())
    {
        const auto &data = m_RadioSubscriber.get();

        m_RadioSocket.send(&data);
    }
}