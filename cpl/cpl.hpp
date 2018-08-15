// 1. Refactor EventExpectant methods.
// 2. Make socketHandle_ and eventHandle_ pointers.
// 3. Write doxygen comments.
// 4. Separate includes.
// 5. WaitForEvents - waitAll parameter.
// 6. WaitForEvent(s) - configuration for sockets.

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
#include <memory>
#include <mutex>
#include <queue>
#include <iostream>
#include <vector>

#include <sys/types.h>
#include <sys/socket.h>
#include <netinet/in.h>
#include <fcntl.h>
#include <unistd.h>
#include <arpa/inet.h>

#include <sys/eventfd.h>
#include <sys/epoll.h>
#include <vector>

// =============================
// ========== Defines ==========
// =============================

// Values for SocketBase, UdpSocket classes.
#define CPL_INVALID_SOCKET_HANDLE        ( 0 )

// Values for UdpSocket class.
#define CPL_UDP_MESSAGE_MAX_SIZE       ( 65507 )

// Errors for UdpSocket class.
#define CPL_UDP_SOCKET_ERROR_INVALID_BUFFER  ( -1 )
#define CPL_UDP_SOCKET_ERROR_INVALID_SOCKET  ( -2 )
#define CPL_UDP_SOCKET_ERROR_RECVFROM_FAILED ( -3 )
#define CPL_UDP_SOCKET_ERROR_SENDTO_FAILED   ( -4 )

// Values for Event class.
#define CPL_INVALID_EVENT_HANDLE  ( 0 )

// Values for EventExpectant class.
#define CPL_EE_WFE_MAX_EVENTS                   ( 64 )
#define CPL_EE_WFE_INFINITE_WAIT              ( 0xFFFF )
#define CPL_EE_WFE_TIME_IS_UP                 ( 0xFFFF )
#define CPL_EE_WFE_ERROR_INVALID_EVENT_HANDLE ( 0xFFFE )
#define CPL_EE_WFE_ERROR_EVENT_MAX_LIMIT      ( 0xFFFD )
#define CPL_EE_WFE_ERROR_FAILED               ( 0xFFFC )

// ==============================
// ========== Typedefs ==========
// ==============================

typedef int32_t CPL_PLATFORM_SOCKET;
typedef int32_t CPL_PLATFORM_EVENT;

// ==========================================
// ========== Classes declarations ==========
// ==========================================

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

class UdpSocket : public SocketBase {
public:
    bool open( const uint16_t& portNumber, const bool& nonBlockingModeFlag ) override;
    bool isOpen() const override;
    void close() override;

    int32_t receiveFrom( uint8_t* bufPtr, uint16_t bufSize, IpAddress& ipAddress );
    int32_t sendTo( uint8_t* bufPtr, uint16_t bufSize, IpAddress& ipAddress );
};

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

class EventExpectant {
public:
    static uint32_t waitForEvent( Event* event, uint32_t milliseconds = 0 );
    static uint32_t waitForEvents( std::vector<Event*>* events, bool waitAll, uint32_t milliseconds = 0 );
};

template<typename T>
class EventQueue {
public:
    EventQueue();
    EventQueue& operator=( const EventQueue& ) = delete;
    ~EventQueue();
    void push( T value );
    bool tryPop( T& value );
    std::shared_ptr<T> tryPop();
    bool isEmpty() const;
    uint64_t size() const;
    Event* getEventHandle();
private:
    mutable std::mutex queueMutex_;
    std::queue<T> queue_;
    Event* newElementEvent_;
};

// =========================================
// ========== Classes definitions ==========
// =========================================

IpAddress::IpAddress() {
    portNumber_ = 0;
}

inline bool IpAddress::setIp( const std::string& ip ) {
    sockaddr_in sa;
    int32_t result =  inet_pton( AF_INET, ip.data(), &( sa.sin_addr ) );
    if ( result != 0 ) {
        ip_ = ip;
        return true;
    }
    else {
        return false;
    }
}

inline bool IpAddress::setPortNumber( const uint16_t& portNumber ) {
    if ( 49152 <= portNumber && portNumber <= 65535 ) {
        portNumber_ = portNumber;
        return true;
    }
    else {
        return false;
    }
}

inline std::string IpAddress::getIp() const {
    return ip_;
}

inline uint16_t IpAddress::getPortNumber() const {
    return portNumber_;
}

SocketBase::SocketBase() {
    socketHandle_ = CPL_INVALID_SOCKET_HANDLE;
}

CPL_PLATFORM_SOCKET SocketBase::getSocketHandle() const {
    return socketHandle_;
}

bool UdpSocket::open( const uint16_t& portNumber, const bool& nonBlockingModeFlag ) {
    socketHandle_ = ::socket( AF_INET, SOCK_DGRAM, IPPROTO_UDP );

    if ( socketHandle_ <= 0 ) {
        return false;
    }

    sockaddr_in address;
    address.sin_family = AF_INET;
    address.sin_addr.s_addr = INADDR_ANY;
    address.sin_port = htons( portNumber );

    if ( ::bind( socketHandle_, ( const sockaddr* )&address, sizeof( sockaddr_in ) ) < 0 ) {
        this->close();
        return false;
    }

    if ( nonBlockingModeFlag ) {
        if ( fcntl( socketHandle_, F_SETFL, O_NONBLOCK, 1 ) == -1 ) {
            this->close();
            return false;
        }
    }

    return true;
}

bool UdpSocket::isOpen() const {
    return socketHandle_ != CPL_INVALID_SOCKET_HANDLE;
}

void UdpSocket::close() {
    if ( socketHandle_ != CPL_INVALID_SOCKET_HANDLE ) {
        ::close( socketHandle_ );
        socketHandle_ = CPL_INVALID_SOCKET_HANDLE;
    }
}

int32_t UdpSocket::receiveFrom( uint8_t* bufPtr, uint16_t bufSize, IpAddress& ipAddress ) {
    if ( socketHandle_ == CPL_INVALID_SOCKET_HANDLE ) {
        return CPL_UDP_SOCKET_ERROR_INVALID_SOCKET;
    }

    if ( bufSize <= 0 ) {
        return CPL_UDP_SOCKET_ERROR_INVALID_BUFFER;
    }

    sockaddr_in senderAddress;
    socklen_t senderAddressSize = sizeof( senderAddress );

    ssize_t receivedBytes = recvfrom(socketHandle_,
                                     reinterpret_cast<char*>( bufPtr ),
                                     bufSize,
                                     0,
                                     ( sockaddr* )&senderAddress,
                                     &senderAddressSize);

    if ( receivedBytes >= 0 ) {
        return receivedBytes;
    }
    else {
        return CPL_UDP_SOCKET_ERROR_RECVFROM_FAILED;
    }
}

int32_t UdpSocket::sendTo( uint8_t* bufPtr, uint16_t bufSize, IpAddress& ipAddress ) {
    if ( socketHandle_ == CPL_INVALID_SOCKET_HANDLE ) {
        return CPL_UDP_SOCKET_ERROR_INVALID_SOCKET;
    }

    if ( bufSize <= 0 || CPL_UDP_MESSAGE_MAX_SIZE < bufSize ) {
        return CPL_UDP_SOCKET_ERROR_INVALID_BUFFER;
    }

    sockaddr_in destinationAddress;
    destinationAddress.sin_family = AF_INET;
    inet_aton( ipAddress.getIp().data(), &destinationAddress.sin_addr );
    destinationAddress.sin_port = htons( ipAddress.getPortNumber() );

    ssize_t bytesSend = sendto( socketHandle_,
                                reinterpret_cast<const char*>(bufPtr),
                                bufSize,
                                0,
                                ( sockaddr* )&destinationAddress,
                                sizeof( sockaddr_in ) );

    if ( bytesSend >= 0 ) {
        return bytesSend;
    }
    else {
        return CPL_UDP_SOCKET_ERROR_SENDTO_FAILED;
    }
}

Event::Event() :
    signaled_( false ),
    eventHandle_( CPL_INVALID_EVENT_HANDLE )
{}

Event::~Event() {
    if ( eventHandle_ != CPL_INVALID_EVENT_HANDLE ) {
        close( eventHandle_ );
    }
}

CPL_PLATFORM_EVENT Event::getEventHandle() const {
    return eventHandle_;
}

bool Event::initializeEvent() {
    eventHandle_ = eventfd( 0, EFD_NONBLOCK );
    return eventHandle_ != CPL_INVALID_EVENT_HANDLE;
}

bool Event::initializeEvent( UdpSocket& udpSocket ) {
    if( !udpSocket.isOpen() ) {
        return false;
    }

    eventHandle_ = udpSocket.getSocketHandle();

    return true;
}

bool Event::setEvent() {
    if ( eventHandle_ == CPL_INVALID_EVENT_HANDLE ) {
        signaled_ = false;
        return false;
    }

    signaled_ = true;

    int32_t result =  eventfd_write( eventHandle_, 1 );
    if ( result == -1 ) {
        signaled_ = false;
        return false;
    }
    else {
        return true;
    }
}

bool Event::isSignaled() const {
    return signaled_;
}

bool Event::resetEvent() {
    if ( !signaled_ ) {
        return true;
    }

    signaled_ = false;

    if ( eventHandle_ == CPL_INVALID_EVENT_HANDLE ) {
        return false;
    }

    eventfd_t eventfdt;
    return eventfd_read( eventHandle_, &eventfdt ) != -1;
}

static uint32_t EventExpectant::waitForEvent( Event* event, uint32_t milliseconds = 0 ) {
    if ( event->getEventHandle() == CPL_INVALID_EVENT_HANDLE ) {
        return CPL_EE_WFE_ERROR_INVALID_EVENT_HANDLE;
    }

    int32_t epollFD = epoll_create( 1 );

    if ( epollFD < 0 ) {
        return CPL_EE_WFE_ERROR_FAILED;
    }

    epoll_event epollEvent;
    epollEvent.data.fd = event->getEventHandle();
    epollEvent.events = EPOLLIN;
    epoll_ctl( epollFD, EPOLL_CTL_ADD, event->getEventHandle(), &epollEvent );

    epoll_event epollEvents[ 1 ];

    int32_t result = epoll_wait( epollFD, epollEvents, 1, -1 );

    close( epollFD );

    if ( result == -1 ) {
        return CPL_EE_WFE_ERROR_FAILED;
    }
    else {
        return 1;
    }
}

static uint32_t EventExpectant::waitForEvents( std::vector<Event*>* events,
                                               bool waitAll,
                                               uint32_t milliseconds = 0 )
{
    if ( events->size() > CPL_EE_WFE_MAX_EVENTS ) {
        return CPL_EE_WFE_ERROR_EVENT_MAX_LIMIT;
    }

    for ( auto event : *events ) {
        if ( event->getEventHandle() == CPL_EE_WFE_ERROR_INVALID_EVENT_HANDLE ) {
            return CPL_EE_WFE_ERROR_INVALID_EVENT_HANDLE;
        }
    }

    int32_t epollFD = epoll_create( ( int32_t )events->size() );

    if ( epollFD < 0 ) {
        return CPL_EE_WFE_ERROR_FAILED;
    }

    for ( auto event : *events ) {
        epoll_event epollEvent;
        epollEvent.data.fd = event->getEventHandle();
        epollEvent.events = EPOLLIN | EPOLLRDHUP;
        epoll_ctl( epollFD, EPOLL_CTL_ADD, event->getEventHandle(), &epollEvent );
    }

    epoll_event* epollEvents = new epoll_event[ events->size() ];

    int32_t result;
    if( milliseconds == CPL_EE_WFE_INFINITE_WAIT ) {
        result = epoll_wait( epollFD, epollEvents, events->size(), -1 );
    }
    else {
        result = epoll_wait( epollFD, epollEvents, events->size(), milliseconds );
    }

    close( epollFD );

    if ( result == -1 ) {
        delete[] epollEvents;
        return CPL_EE_WFE_ERROR_FAILED;
    }
    else if ( milliseconds != CPL_EE_WFE_INFINITE_WAIT && result == 0 ) {
        delete[] epollEvents;
        return CPL_EE_WFE_TIME_IS_UP;
    }
    else {
        uint32_t i = 0;
        uint32_t eventNumber = 0;
        bool breakLoop = false;
        for ( auto event : *events ) {
            for ( i = 0; i < result; i++ ) {
                if ( epollEvents[ i ].data.fd == event->getEventHandle() ) {
                    breakLoop = true;
                    break;
                }
            }

            if ( breakLoop ) {
                break;
            }
            else {
                eventNumber++;
            }
        }

        delete[] epollEvents;
        return eventNumber;
    }
}

template<typename T>
EventQueue::EventQueue() {
    newElementEvent_ = new Event;
    newElementEvent_->initializeEvent();
}

template<typename T>
EventQueue::~EventQueue() {
    delete newElementEvent_; // How to check? Handle may be anywhere.
}

template<typename T>
void EventQueue::push( T value ) {
    std::lock_guard<std::mutex> lock( queueMutex_ );
    queue_.push( value );
    if ( !newElementEvent_->isSignaled() ) {
        newElementEvent_->setEvent();
    }
}

template<typename T>
bool EventQueue::tryPop( T& value ) {
    if ( !newElementEvent_->isSignaled() ) {
        return false;
    }
    else {
        std::lock_guard<std::mutex> lock( queueMutex_ );
        if ( !queue_.empty() ) {
            value = queue_.front();
            queue_.pop();
        }
        if ( queue_.empty() ) {
            newElementEvent_->resetEvent();
        }
        return true;
    }
}

template<typename T>
std::shared_ptr<T> EventQueue::tryPop() {
    std::shared_ptr<T> result = NULL;
    if ( !newElementEvent_->isSignaled() ) {
        return result;
    }
    else {
        std::lock_guard<std::mutex> lock( queueMutex_ );
        if ( !queue_.empty() ) {
            result = std::make_shared<T>( queue_.front() );
            queue_.pop();
            if ( queue_.empty() ) {
                newElementEvent_->resetEvent();
            }
            return result;
        }
    }
}

template<typename T>
bool EventQueue::isEmpty() const {
    return !newElementEvent_->isSignaled();
}

template<typename T>
uint64_t EventQueue::size() const {
    std::lock_guard<std::mutex> lock( queueMutex_ );
    return queue_.size();
}

template<typename T>
Event* EventQueue::getEventHandle() {
    return newElementEvent_;
}

#endif // CPL_HPP