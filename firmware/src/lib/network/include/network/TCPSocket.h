#pragma once

#include <datalink.h>
#include <cstdint>

#define TCP_RECEIVE_BUFFER_SIZE 512
#define TCP_SEND_BUFFER_SIZE 512

namespace network
{
    class TCPSocket
    {
    public:
        TCPSocket();
        ~TCPSocket();

        void createServer(uint16_t port, bool blocking);
        void createClient(const char *ip, uint16_t port);
        void close();
        bool receive(datalink_message_t *msg);
        void send(const datalink_message_t *msg);
        bool isActive() const { return m_Active; }

    private:
        bool m_Active;
        bool m_Blocking;

        uint64_t m_ServerSocketHandle;
        uint64_t m_SocketHandle;

        uint8_t m_ReceiveBuffer[TCP_RECEIVE_BUFFER_SIZE];
        int m_ReceiveBufferLength;
        uint8_t m_SendBuffer[TCP_SEND_BUFFER_SIZE];
        int m_SendBufferLength;

        void closeServerSocket();
        void closeSocket();

        static int &GetRefCount();
    };
}