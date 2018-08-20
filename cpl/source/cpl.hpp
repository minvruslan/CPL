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

// Socket event types.
// Event + socket.
// TcpSocket.

#ifndef _CPL_HPP_
#define _CPL_HPP_

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

#define CPL_INVALID_SOCKET_HANDLE        ( 0 )

#define CPL_UDP_MESSAGE_MAX_SIZE       ( 65507 )

#define CPL_UDP_SOCKET_ERROR_INVALID_BUFFER  ( -1 )
#define CPL_UDP_SOCKET_ERROR_INVALID_SOCKET  ( -2 )
#define CPL_UDP_SOCKET_ERROR_RECVFROM_FAILED ( -3 )
#define CPL_UDP_SOCKET_ERROR_SENDTO_FAILED   ( -4 )

#define CPL_INVALID_EVENT_HANDLE  ( 0 )

#define CPL_EE_WFE_MAX_EVENTS                   ( 64 )
#define CPL_EE_WFE_INFINITE_WAIT              ( 0xFFFF )
#define CPL_EE_WFE_TIME_IS_UP                 ( 0xFFFF )
#define CPL_EE_WFE_ERROR_INVALID_EVENT_HANDLE ( 0xFFFE )
#define CPL_EE_WFE_ERROR_EVENT_MAX_LIMIT      ( 0xFFFD )
#define CPL_EE_WFE_ERROR_FAILED               ( 0xFFFC )
#define CPL_EE_WFE_ALL_EVENTS_SIGNALED        ( 0xFFFB )

// ==============================
// ========== Typedefs ==========
// ==============================

typedef int32_t CPL_PLATFORM_SOCKET;
typedef int32_t CPL_PLATFORM_EVENT;

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
public:
    SocketBase();

    virtual bool open( const uint16_t& portNumber, const bool& nonBlockingModeFlag ) = 0;
    virtual bool isOpen() const = 0;
    virtual void close() = 0;

    CPL_PLATFORM_SOCKET getSocketHandle() const;
protected:
    CPL_PLATFORM_SOCKET socketHandle_;
};

// =================================================
// ========== Class UdpSocket declaration ==========
// =================================================

class UdpSocket : public SocketBase {
public:
    bool open( const uint16_t& portNumber, const bool& nonBlockingModeFlag );
    bool isOpen() const;
    void close();

    int32_t receiveFrom( uint8_t* bufPtr, uint16_t bufSize, IpAddress& ipAddress );
    int32_t sendTo( uint8_t* bufPtr, uint16_t bufSize, IpAddress& ipAddress );
};

// =====================================================
// ========== Class TcpSocketBase declaration ==========
// =====================================================

//class TcpSocketBase : public SocketBase {
//public:
//    bool open( const uint16_t& portNumber, const bool& nonBlockingMode );
//    bool isOpen() const;
//    void close();
//};

// ===========================================
// ========== Class TcpServerSocket ==========
// ===========================================

//class TcpServerSocket : public TcpSocketBase {
//
//};

// ===========================================
// ========== Class TcpClientSocket ==========
// ===========================================

//class TcpClientSocket : public TcpSocketBase {
//
//};

// =============================================
// ========== Class Event declaration ==========
// =============================================

class Event {
public:
    Event();
    ~Event();

    CPL_PLATFORM_EVENT getEventHandle() const;

    bool initializeEvent();
    bool initializeEvent( UdpSocket& udpSocket );
    bool setEvent();
    bool isSignaled() const;
    bool resetEvent();
private:
    CPL_PLATFORM_EVENT eventHandle_;
    bool signaled_;
};

// ======================================================
// ========== Class EventExpectant declaration ==========
// ======================================================

class EventExpectant {
public:
    static uint32_t waitForEvent( Event* event, uint32_t milliseconds = 0 );
    static uint32_t waitForEvents( std::vector<Event*>* events, bool waitAll, uint32_t milliseconds = 0 );
};

#endif // _CPL_HPP_