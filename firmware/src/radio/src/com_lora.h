#pragma once

#include <datalink.h>
#include <stddef.h>

class CommunicationLoRa
{
public:
    void init();
    void update();

    bool getPacket(datalink_message_t *msg);
    void sendMessage(const datalink_message_t *msg);

    void setRX();
    void setTX();

private:
    datalink_message_t m_CurrentPacket;
    bool m_HasPacket;
};