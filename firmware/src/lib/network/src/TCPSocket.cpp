#include "network/TCPSocket.h"
#include <lib/debug/sys_assert.h>
#include <cstring>
#include <cstdio>
#include <cstdlib>

#ifdef _WIN32
#include <winsock2.h>
#include <ws2tcpip.h>

#define NATIVE_SOCKET SOCKET
#define INVALID_SOCKET_HANDLE INVALID_SOCKET
#define CLOSESOCKET closesocket
#else
#include <arpa/inet.h>
#include <unistd.h>

#define NATIVE_SOCKET int
#define INVALID_SOCKET_HANDLE -1
#define CLOSESOCKET ::close
#endif

#define GET_NATIVE_SOCKET(x) ((NATIVE_SOCKET)(x))

#ifdef NETWORK_LOGS_ENABLED
#define NETWORK_DEBUG(msg, ...) printf("[NETWORK] (debug)  " msg "\n", ##__VA_ARGS__)
#define NETWORK_INFO(msg, ...) printf("[NETWORK] (info)   " msg "\n", ##__VA_ARGS__)
#define NETWORK_WARN(msg, ...) printf("[NETWORK] (warn)   " msg "\n", ##__VA_ARGS__)
#define NETWORK_ERROR(msg, ...) printf("[NETWORK] (error)  " msg "\n", ##__VA_ARGS__)
#else
#define NETWORK_DEBUG(msg, ...) (void)0
#define NETWORK_INFO(msg, ...) (void)0
#define NETWORK_WARN(msg, ...) (void)0
#define NETWORK_ERROR(msg, ...) (void)0
#endif

namespace network
{
    static void SetNonBlocking(uint64_t sock, bool non_blocking)
    {
        if (!non_blocking)
        {
            return;
        }

#ifdef _WIN32
        u_long mode = 1;
        ioctlsocket(GET_NATIVE_SOCKET(sock), FIONBIO, &mode);
#else
        int flags = fcntl(GET_NATIVE_SOCKET(sock), F_GETFL, 0);
        fcntl(GET_NATIVE_SOCKET(sock), F_SETFL, flags | O_NONBLOCK);
#endif
    }

    TCPSocket::TCPSocket() : m_Active(false), m_Blocking(true), m_ServerSocketHandle(INVALID_SOCKET_HANDLE), m_SocketHandle(INVALID_SOCKET_HANDLE), m_ReceiveBufferLength(0), m_SendBufferLength(0)
    {
#ifdef _WIN32
        if (GetRefCount()++ == 0)
        {
            WSADATA wsaData;
            int res = WSAStartup(MAKEWORD(2, 2), &wsaData);

            SYS_ASSERT_MSG(res == 0, "WSAStartup failed: %d", res);
        }
#endif
    }

    TCPSocket::~TCPSocket()
    {
        close();

#ifdef _WIN32
        if (--GetRefCount() == 0)
        {
            WSACleanup();
        }
#endif
    }

    void TCPSocket::createServer(uint16_t port)
    {
        SYS_ASSERT(m_SocketHandle == INVALID_SOCKET_HANDLE);

        NATIVE_SOCKET tmpSocket = socket(AF_INET, SOCK_STREAM, 0);
        SYS_ASSERT(tmpSocket != INVALID_SOCKET_HANDLE);
        m_ServerSocketHandle = tmpSocket;

        int opt = 1;
        setsockopt(GET_NATIVE_SOCKET(m_ServerSocketHandle), SOL_SOCKET, SO_REUSEADDR, (const char *)&opt, sizeof(opt));

        sockaddr_in addr;
        memset(&addr, 0, sizeof(addr));

        addr.sin_family = AF_INET;
        addr.sin_addr.s_addr = INADDR_ANY;
        addr.sin_port = htons(port);

        if (bind(GET_NATIVE_SOCKET(m_ServerSocketHandle), (sockaddr *)&addr, sizeof(addr)) < 0)
        {
            close();

            NETWORK_ERROR("Bind failed");
            SYS_ASSERT(false);

            return;
        }

        NETWORK_DEBUG("TCP Server socket bound to port %d", port);

        if (listen(GET_NATIVE_SOCKET(m_ServerSocketHandle), 1) < 0)
        {
            close();

            NETWORK_ERROR("Listen failed");
            SYS_ASSERT(false);

            return;
        }

        NETWORK_DEBUG("TCP server socket waiting for any client to connect...");

        m_SocketHandle = accept(GET_NATIVE_SOCKET(m_ServerSocketHandle), nullptr, nullptr);

        if (m_SocketHandle == INVALID_SOCKET_HANDLE)
        {
            close();

            NETWORK_WARN("No client connected");

            return;
        }

        NETWORK_DEBUG("Client connected!");

        m_Active = true;
    }

    void TCPSocket::createClient(const char *ip, uint16_t port)
    {
        SYS_ASSERT(m_SocketHandle == INVALID_SOCKET_HANDLE);
        SYS_ASSERT(ip != nullptr);

        NATIVE_SOCKET tmpSocket = socket(AF_INET, SOCK_STREAM, 0);
        SYS_ASSERT(tmpSocket != INVALID_SOCKET_HANDLE);
        m_SocketHandle = tmpSocket;

        sockaddr_in addr;
        memset(&addr, 0, sizeof(addr));

        addr.sin_family = AF_INET;
        addr.sin_port = htons(port);

        if (inet_pton(AF_INET, ip, &addr.sin_addr) <= 0)
        {
            close();

            NETWORK_ERROR("Invalid IP");
            SYS_ASSERT(false);

            return;
        }

        NETWORK_DEBUG("Connecting client to %s:%d...", ip, port);

        while (::connect(GET_NATIVE_SOCKET(m_SocketHandle), (sockaddr *)&addr, sizeof(addr)) < 0)
        {
            NETWORK_WARN("Failed to connect TCP Client socket, retrying...");

            continue;
        }

        m_Active = true;

        NETWORK_DEBUG("TCP Client socket connected!");
    }

    void TCPSocket::setBlocking(bool blocking)
    {
        m_Blocking = blocking;

        SetNonBlocking(m_ServerSocketHandle, !m_Blocking);
        SetNonBlocking(m_SocketHandle, !m_Blocking);
    }

    void TCPSocket::close()
    {
        closeSocket();
        closeServerSocket();

        m_Active = false;
        m_ReceiveBufferLength = 0;
        m_SendBufferLength = 0;
    }

    bool TCPSocket::receive(datalink_message_t *msg)
    {
        SYS_ASSERT(msg != nullptr);
        SYS_ASSERT(m_SocketHandle != INVALID_SOCKET_HANDLE);

        while (true)
        {
            int space_left = sizeof(m_ReceiveBuffer) - m_ReceiveBufferLength;
            SYS_ASSERT(space_left > 0);

            int received = recv(GET_NATIVE_SOCKET(m_SocketHandle), (char *)(m_ReceiveBuffer + m_ReceiveBufferLength), space_left, 0);

            if (received <= 0)
            {
                if (received == 0)
                {
                    NETWORK_INFO("Client disconnected.");

                    close();
                }

                return false;
            }

            m_ReceiveBufferLength += received;

            for (int i = 0; i < m_ReceiveBufferLength; ++i)
            {
                if (m_ReceiveBuffer[i] == 0x00)
                {
                    int len = i + 1;
                    bool status = false;

                    if (datalink_deserialize_message_serial(msg, m_ReceiveBuffer, len) == DATALINK_OK)
                    {
                        status = true;
                    }
                    else
                    {
                        SYS_ASSERT_MSG(false, "Failed to deserialize message from TCP, dropping message");
                    }

                    int remaining_bytes = m_ReceiveBufferLength - len;

                    if (remaining_bytes > 0)
                    {
                        memmove(m_ReceiveBuffer, m_ReceiveBuffer + len, remaining_bytes);
                    }

                    m_ReceiveBufferLength = remaining_bytes;

                    return status;
                }
            }

            if (!m_Blocking)
            {
                return false;
            }
        }

        return false;
    }

    void TCPSocket::send(const datalink_message_t *msg)
    {
        SYS_ASSERT(msg != nullptr);
        SYS_ASSERT(m_SocketHandle != INVALID_SOCKET_HANDLE);

        int len = sizeof(m_SendBuffer);
        int res = datalink_serialize_message_serial(msg, m_SendBuffer, &len);

        SYS_ASSERT(res == DATALINK_OK);

        int total_sent = 0;

        while (total_sent < len)
        {
            int sent = ::send(GET_NATIVE_SOCKET(m_SocketHandle), (const char *)m_SendBuffer + total_sent, len - total_sent, 0);

            if (sent <= 0)
            {
                NETWORK_ERROR("Failed to send data over TCP socket");
                SYS_ASSERT(false);

                return;
            }

            total_sent += sent;
        }
    }

    void TCPSocket::closeServerSocket()
    {
        if (m_ServerSocketHandle != INVALID_SOCKET_HANDLE)
        {
            CLOSESOCKET(GET_NATIVE_SOCKET(m_ServerSocketHandle));
            m_ServerSocketHandle = INVALID_SOCKET_HANDLE;

            NETWORK_DEBUG("TCP server socket closed.");
        }
    }

    void TCPSocket::closeSocket()
    {
        if (m_SocketHandle != INVALID_SOCKET_HANDLE)
        {
            CLOSESOCKET(GET_NATIVE_SOCKET(m_SocketHandle));
            m_SocketHandle = INVALID_SOCKET_HANDLE;

            NETWORK_DEBUG("TCP socket closed.");
        }
    }

    int &TCPSocket::GetRefCount()
    {
        static int ref_count = 0;
        return ref_count;
    }
}