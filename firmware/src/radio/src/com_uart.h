#pragma once

#include <datalink.h>
#include <stddef.h>

class CommunicationUART
{
public:
    void init();
    bool read(datalink_message_t *msg);

    void sendMessage(const datalink_message_t *msg);

private:
    uint8_t m_ReceiveBuffer[512];
    size_t m_ReceiveLen;
};