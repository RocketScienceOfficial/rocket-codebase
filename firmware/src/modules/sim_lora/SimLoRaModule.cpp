#include "SimLoRaModule.h"
#include "sitl.h"
#include "modules/common/ModuleLogger.h"
#include <lib/debug/sys_assert.h>
#include <board_config.h>

SimLoRaModule::~SimLoRaModule()
{
    m_LoRaSocket.close();
}

void SimLoRaModule::init()
{
    sitl_init_godmode();

#ifndef CFG_SIM_LORA_SERVER
    m_LoRaSocket.createServer(CFG_SIM_LORA_PORT);
#else
    m_LoRaSocket.createClient(CFG_SIM_LORA_SERVER, CFG_SIM_LORA_PORT);
#endif

    m_LoRaSocket.setBlocking(false);
}

void SimLoRaModule::run()
{
    if (!m_LoRaSocket.isActive())
    {
        return;
    }

    receive();
    sendIfAvailable();
}

void SimLoRaModule::receive()
{
    datalink_message_t loraData;
    if (m_LoRaSocket.receive(&loraData))
    {
        m_LoRaPublisher.publish(loraData);
    }
}

void SimLoRaModule::sendIfAvailable()
{
    if (!m_Flushed)
    {
        m_LoRaSubscriber.pollLatest();
        m_Flushed = true;
        return;
    }

    while (m_LoRaSubscriber.poll())
    {
        const auto &data = m_LoRaSubscriber.get();

        m_LoRaSocket.send(&data);

        datalink_message_t msg;
        datalink_pack_radio_module_tx_done(&msg);
        m_LoRaPublisher.publish(msg);
    }
}