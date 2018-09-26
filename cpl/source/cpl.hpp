// Licensed under the MIT License <http://opensource.org/licenses/MIT>.
// Copyright (c) 2017-2018 Minnibaev Ruslan <minvruslan@gmail.com>

// Permission is hereby granted, free of charge, to any person obtaining a copy
// of this software and associated documentation files (the "Software"), to deal
// in the Software without restriction, including without limitation the rights
// to use, copy, modify, merge, publish, distribute, sublicense, and/or sell
// copies of the Software, and to permit persons to whom the Software is
// furnished to do so, subject to the following conditions:

// The above copyright notice and this permission notice shall be included in all
// copies or substantial portions of the Software.

// THE SOFTWARE IS PROVIDED "AS IS", WITHOUT WARRANTY OF ANY KIND, EXPRESS OR
// IMPLIED, INCLUDING BUT NOT LIMITED TO THE WARRANTIES OF MERCHANTABILITY,
// FITNESS FOR A PARTICULAR PURPOSE AND NONINFRINGEMENT. IN NO EVENT SHALL THE
// AUTHORS OR COPYRIGHT HOLDERS BE LIABLE FOR ANY CLAIM, DAMAGES OR OTHER
// LIABILITY, WHETHER IN AN ACTION OF CONTRACT, TORT OR OTHERWISE, ARISING FROM,
// OUT OF OR IN CONNECTION WITH THE SOFTWARE OR THE USE OR OTHER DEALINGS IN THE
// SOFTWARE.

#ifndef CPL_HPP
#define CPL_HPP

// ==============================
// ========== Includes ==========
// ==============================

#include <cstdint>
#include <string>
#include <vector>
#include <queue>
#include <memory>
#include <mutex>

#include <fcntl.h>
#include <unistd.h>
#include <netinet/in.h>
#include <arpa/inet.h>
#include <sys/types.h>
#include <sys/socket.h>
#include <sys/eventfd.h>
#include <sys/epoll.h>

// =============================
// ========== Defines ==========
// =============================

#define CPL_INVALID_SOCKET_HANDLE ( 0 )

#define CPL_UDP_MESSAGE_MAX_SIZE ( 65507 )

#define CPL_UDP_SOCKET_ERROR_INVALID_BUFFER  ( -1 )
#define CPL_UDP_SOCKET_ERROR_INVALID_SOCKET  ( -2 )
#define CPL_UDP_SOCKET_ERROR_RECVFROM_FAILED ( -3 )
#define CPL_UDP_SOCKET_ERROR_SENDTO_FAILED   ( -4 )

#define CPL_TCP_SOCKET_SEND_OK               (  0 )
#define CPL_TCP_SOCKET_ERROR_INVALID_BUFFER  ( -1 )
#define CPL_TCP_SOCKET_ERROR_INVALID_SOCKET  ( -2 )
#define CPL_TCP_SOCKET_ERROR_RECV_FAILED     ( -3 )
#define CPL_TCP_SOCKET_ERROR_SEND_FAILED     ( -4 )

#define CPL_INVALID_EVENT_HANDLE ( 0 )

#define CPL_EE_WFE_MAX_EVENTS                 (   1000 )
#define CPL_EE_WFE_INFINITE_WAIT              ( 0xFFFF )
#define CPL_EE_WFE_TIME_IS_UP                 ( 0xFFFF )
#define CPL_EE_WFE_ERROR_INVALID_EVENT_HANDLE ( 0xFFFE )
#define CPL_EE_WFE_ERROR_EVENT_MAX_LIMIT      ( 0xFFFD )
#define CPL_EE_WFE_ERROR_FAILED               ( 0xFFFC )
#define CPL_EE_WFE_ALL_EVENTS_SIGNALED        ( 0xFFFB )

#define CPL_SOCKET_EVENT_TYPE_INVALID ( 0x00 ) // 0x00 = 0b00000000
#define CPL_SOCKET_EVENT_TYPE_READ    ( 0x01 ) // 0x01 = 0b00000001
#define CPL_SOCKET_EVENT_TYPE_WRITE   ( 0x02 ) // 0x02 = 0b00000010

#define NS_CPL_START namespace cpl {
#define NS_CPL_END   }

NS_CPL_START

// ==============================
// ========== Typedefs ==========
// ==============================

typedef int32_t CPL_PLATFORM_SOCKET;
typedef int32_t CPL_PLATFORM_EVENT;
typedef uint8_t CPL_SOCKET_EVENT_TYPES;

// ===========================
// ========== Enums ==========
// ===========================

enum class SocketType {
    UDP_SOCKET,
    TCP_SERVER_LISTEN_SOCKET,
    TCP_SERVER_EXCHANGE_SOCKET,
    TCP_CLIENT_SOCKET
};

// =========================================
// ========== Forward declaration ==========
// =========================================

class TcpServerListenSocket;
class Event;

// ===============================================
// ========== Class CplBase declaration ==========
// ===============================================

class CplBase {
public:
    static bool initialize();
    static void shutdown();
};

// =================================================
// ========== Class IpAddress declaration ==========
// =================================================

class IpAddress {
public:
    IpAddress();

    bool setIp( const std::string& ip );
    bool setPortNumber( const uint16_t& portNumber );

    std::string getIp() const;
    uint16_t getPortNumber() const;
private:
    std::string ip_;
    uint16_t portNumber_;
};

// ==================================================
// ========== Class SocketBase declaration ==========
// ==================================================

class SocketBase {
protected:
    explicit SocketBase( SocketType socketType );
    CPL_PLATFORM_SOCKET getSocketHandle() const;
public:
    bool isOpen() const;
    void close();
    SocketType getSocketType() const;
protected:
    CPL_PLATFORM_SOCKET socketHandle_;
    SocketType socketType_;

    friend class Event;
};

// =================================================
// ========== Class UdpSocket declaration ==========
// =================================================

class UdpSocket : public SocketBase {
public:
    UdpSocket();

    bool open( const uint16_t& portNumber, const bool& nonBlockingModeFlag );

    int32_t receiveFrom( uint8_t* bufPtr, uint16_t bufSize, IpAddress& ipAddress );
    int32_t sendTo( uint8_t* bufPtr, uint16_t bufSize, IpAddress& ipAddress );
};

// ===============================================================
// ========== Class TcpServerExchangeSocket declaration ==========
// ===============================================================

class TcpServerExchangeSocket : public SocketBase {
public:
    TcpServerExchangeSocket();

    int32_t receive( uint8_t* bufPtr, uint16_t bufSize );
    int32_t send( uint8_t* bufPtr, uint16_t bufSize);
protected:
    bool setSocketHandle( CPL_PLATFORM_SOCKET socketHandle );
    friend class TcpServerListenSocket;
};

// =============================================================
// ========== Class TcpServerListenSocket declaration ==========
// =============================================================

class TcpServerListenSocket : public SocketBase {
public:
    TcpServerListenSocket();

    bool open( const uint16_t& portNumber, const bool& nonBlockingModeFlag, int32_t maxPendingConnections );
    bool accept( TcpServerExchangeSocket* tcpServerExchangeSocket );
};

// =======================================================
// ========== Class TcpClientSocket declaration ==========
// =======================================================

class TcpClientSocket : public SocketBase {
public:
    TcpClientSocket();

    bool open( const uint16_t& portNumber, const bool& nonBlockingModeFlag );
    bool connect( IpAddress& ipAddress );
    int32_t receive( uint8_t* bufPtr, uint16_t bufSize );
    int32_t send( uint8_t* bufPtr, uint16_t bufSize);
};

// ======================================================
// ========== Class EventExpectant declaration ==========
// ======================================================

class EventExpectant {
public:
    static uint32_t waitForEvent( Event* event, uint32_t milliseconds = 0 );
    static uint32_t waitForEvents( std::vector<Event*>* events, bool waitAll, uint32_t milliseconds = 0 );
};

// =============================================
// ========== Class Event declaration ==========
// =============================================

class Event {
public:
    Event();
    ~Event();

    bool initializeEvent();
    bool initializeEvent( UdpSocket& udpSocket,
                          CPL_SOCKET_EVENT_TYPES socketEventTypes );
    bool initializeEvent( TcpServerListenSocket& tcpServerListenSocket,
                          CPL_SOCKET_EVENT_TYPES socketEventTypes);
    bool initializeEvent( TcpServerExchangeSocket& tcpServerExchangeSocket,
                          CPL_SOCKET_EVENT_TYPES socketEventTypes );
    bool initializeEvent( TcpClientSocket& tcpClientSocket,
                          CPL_SOCKET_EVENT_TYPES socketEventTypes );
    bool setEvent();
    bool isSignaled() const;
    bool resetEvent();

    void close();

    bool isSocketEvent() const;
    CPL_SOCKET_EVENT_TYPES getSocketEventTypes() const;
protected:
    CPL_PLATFORM_EVENT getEventHandle() const;
private:
    CPL_PLATFORM_EVENT eventHandle_;
    bool signaled_;

    bool isSocketEvent_;
    CPL_SOCKET_EVENT_TYPES socketEventTypes_;

    friend class EventExpectant;
};

NS_CPL_END

#endif // CPL_HPP