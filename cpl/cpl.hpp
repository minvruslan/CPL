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

// ==================================================
// ========== Class EventQueue declaration ==========
// ==================================================

template<typename T>
class EventQueue {
public:
    EventQueue();
    ~EventQueue();

    EventQueue& operator=( const EventQueue& ) = delete;

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

// ================================================
// ========== Class IpAddress definition ==========
// ================================================

inline IpAddress::IpAddress() :
    portNumber_( 0 )
{}

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

// =================================================
// ========== Class SocketBase definition ==========
// =================================================

inline SocketBase::SocketBase() :
    socketHandle_( CPL_INVALID_SOCKET_HANDLE )
{}

inline CPL_PLATFORM_SOCKET SocketBase::getSocketHandle() const {
    return socketHandle_;
}

// ================================================
// ========== Class UdpSocket definition ==========
// ================================================

inline bool UdpSocket::open( const uint16_t& portNumber, const bool& nonBlockingModeFlag ) {
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

inline bool UdpSocket::isOpen() const {
    return ( socketHandle_ != CPL_INVALID_SOCKET_HANDLE );
}

inline void UdpSocket::close() {
    if ( socketHandle_ != CPL_INVALID_SOCKET_HANDLE ) {
        ::close( socketHandle_ );
        socketHandle_ = CPL_INVALID_SOCKET_HANDLE;
    }
}

inline int32_t UdpSocket::receiveFrom( uint8_t* bufPtr, uint16_t bufSize, IpAddress& ipAddress ) {
    if ( socketHandle_ == CPL_INVALID_SOCKET_HANDLE ) {
        return CPL_UDP_SOCKET_ERROR_INVALID_SOCKET;
    }

    if ( bufSize <= 0 ) {
        return CPL_UDP_SOCKET_ERROR_INVALID_BUFFER;
    }

    sockaddr_in senderIpAddress;
    socklen_t senderIpAddressSize = sizeof( senderIpAddress );

    ssize_t receivedBytes = recvfrom( socketHandle_,
                                      reinterpret_cast<char*>( bufPtr ),
                                      bufSize,
                                      0,
                                      ( sockaddr* )&senderIpAddress,
                                      &senderIpAddressSize);

    if ( receivedBytes >= 0 ) {
        return ( int32_t )receivedBytes;
    }
    else {
        return CPL_UDP_SOCKET_ERROR_RECVFROM_FAILED;
    }
}

inline int32_t UdpSocket::sendTo( uint8_t* bufPtr, uint16_t bufSize, IpAddress& ipAddress ) {
    if ( socketHandle_ == CPL_INVALID_SOCKET_HANDLE ) {
        return CPL_UDP_SOCKET_ERROR_INVALID_SOCKET;
    }

    if ( bufSize <= 0 || CPL_UDP_MESSAGE_MAX_SIZE < bufSize ) {
        return CPL_UDP_SOCKET_ERROR_INVALID_BUFFER;
    }

    sockaddr_in destinationIpAddress;
    destinationIpAddress.sin_family = AF_INET;
    inet_aton( ipAddress.getIp().data(), &destinationIpAddress.sin_addr );
    destinationIpAddress.sin_port = htons( ipAddress.getPortNumber() );

    ssize_t bytesSend = sendto( socketHandle_,
                                reinterpret_cast<const char*>( bufPtr ),
                                bufSize,
                                0,
                                ( sockaddr* )&destinationIpAddress,
                                sizeof( sockaddr_in ) );

    if ( bytesSend >= 0 ) {
        return ( int32_t )bytesSend;
    }
    else {
        return CPL_UDP_SOCKET_ERROR_SENDTO_FAILED;
    }
}

// ============================================
// ========== Class Event definition ==========
// ============================================

inline Event::Event() :
    signaled_( false ),
    eventHandle_( CPL_INVALID_EVENT_HANDLE )
{}

inline Event::~Event() {
    if ( eventHandle_ != CPL_INVALID_EVENT_HANDLE ) {
        close( eventHandle_ );
    }
}

inline CPL_PLATFORM_EVENT Event::getEventHandle() const {
    return eventHandle_;
}

inline bool Event::initializeEvent() {
    eventHandle_ = eventfd( 0, EFD_NONBLOCK );
    return ( eventHandle_ != CPL_INVALID_EVENT_HANDLE );
}

inline bool Event::initializeEvent( UdpSocket& udpSocket ) {
    if( !udpSocket.isOpen() ) {
        return false;
    }

    eventHandle_ = udpSocket.getSocketHandle();

    return true;
}

inline bool Event::setEvent() {
    if ( eventHandle_ == CPL_INVALID_EVENT_HANDLE ) {
        signaled_ = false;
        return false;
    }

    signaled_ = true;

    int32_t result =  eventfd_write( eventHandle_, 1 );

    if ( result != -1 ) {
        return true;
    }
    else {
        signaled_ = false;
        return false;
    }
}

inline bool Event::isSignaled() const {
    return signaled_;
}

inline bool Event::resetEvent() {
    if ( !signaled_ ) {
        return true;
    }

    signaled_ = false;

    if ( eventHandle_ == CPL_INVALID_EVENT_HANDLE ) {
        return false;
    }

    eventfd_t eventfdt;

    return ( eventfd_read( eventHandle_, &eventfdt ) != -1 );
}

// =====================================================
// ========== Class EventExpectant definition ==========
// =====================================================

inline uint32_t EventExpectant::waitForEvent( Event* event, uint32_t milliseconds ) {
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

    int32_t waitResult = CPL_EE_WFE_ERROR_FAILED;

    if ( milliseconds == CPL_EE_WFE_INFINITE_WAIT ) {
        waitResult = epoll_wait( epollFD, epollEvents, 1, -1 );
    }
    else {
        waitResult = epoll_wait( epollFD, epollEvents, 1, milliseconds );
    }

    close( epollFD );

    if ( milliseconds != CPL_EE_WFE_INFINITE_WAIT && waitResult == 0 ) {
        return CPL_EE_WFE_TIME_IS_UP;
    }
    else if ( waitResult != -1 ) {
        return 0;
    }
    else {
        return CPL_EE_WFE_ERROR_FAILED;
    }
}

// =====================================================
// ========== Class EventExpectant definition ==========
// =====================================================

inline uint32_t EventExpectant::waitForEvents( std::vector<Event*>* events,
                                               bool waitAll,
                                               uint32_t milliseconds )
{
    if ( events->size() > CPL_EE_WFE_MAX_EVENTS ) {
        return CPL_EE_WFE_ERROR_EVENT_MAX_LIMIT;
    }

    for ( uint16_t i = 0; i < events->size(); i++ ) {
        if ( ( events->at( i ) )->getEventHandle() == CPL_EE_WFE_ERROR_INVALID_EVENT_HANDLE ) {
            return CPL_EE_WFE_ERROR_INVALID_EVENT_HANDLE;
        }
    }

    int32_t epollFD = epoll_create( ( int32_t )events->size() );

    if ( epollFD < 0 ) {
        return CPL_EE_WFE_ERROR_FAILED;
    }

    for ( uint16_t i = 0; i < events->size(); i++ ) {
        epoll_event epollEvent;
        epollEvent.data.fd = events->at( i )->getEventHandle();
        epollEvent.events = EPOLLIN | EPOLLRDHUP;
        epoll_ctl( epollFD, EPOLL_CTL_ADD, events->at( i )->getEventHandle(), &epollEvent );
    }

    epoll_event* epollEvents = new epoll_event[ events->size() ];

    int32_t waitResult = CPL_EE_WFE_ERROR_FAILED;

    if ( milliseconds == CPL_EE_WFE_INFINITE_WAIT ) {
        waitResult = epoll_wait( epollFD, epollEvents, ( int32_t )events->size(), -1 );
    }
    else {
        waitResult = epoll_wait( epollFD, epollEvents, ( int32_t )events->size(), milliseconds );
    }

    close( epollFD );

    if ( waitResult == -1 ) {
        delete[] epollEvents;
        return CPL_EE_WFE_ERROR_FAILED;
    }

    if ( milliseconds != CPL_EE_WFE_INFINITE_WAIT && waitResult == 0 ) {
        delete[] epollEvents;
        return CPL_EE_WFE_TIME_IS_UP;
    }

    if ( waitResult == 0 ) {
        delete[] epollEvents;
        return CPL_EE_WFE_ERROR_FAILED;
    }

    uint32_t signaledEventNumber = 0;
    std::vector<Event*> remainingEvents;

    if ( waitAll ) {
        if ( waitResult == events->size() ) {
            delete[] epollEvents;
            return CPL_EE_WFE_ALL_EVENTS_SIGNALED;
        }

        for ( uint16_t i = 0; i < events->size(); i++ ) {
            for ( uint16_t j = 0; j < waitResult; j++ ) {
                if ( epollEvents[ j ].data.fd == events->at( i )->getEventHandle() ) {
                    remainingEvents.push_back( events->at( i ) );
                }
            }
        }

        delete[] epollEvents;

        return  ( waitForEvents( &remainingEvents, waitAll, milliseconds ) );
    }
    else {
        for ( uint16_t i = 0; i < events->size(); i++ ) {
            for ( uint16_t j = 0; j < waitResult; j++ ) {
                if ( epollEvents[ j ].data.fd == events->at( i )->getEventHandle() ) {
                    delete[] epollEvents;
                    return signaledEventNumber;
                }
                else {
                    signaledEventNumber++;
                }
            }
        }
    }

    delete[] epollEvents;

    return CPL_EE_WFE_ERROR_FAILED;
}

template<typename T>
inline EventQueue<T>::EventQueue() {
    newElementEvent_ = new Event;
    newElementEvent_->initializeEvent();
}

template<typename T>
inline EventQueue<T>::~EventQueue() {
    delete newElementEvent_; // How to check? Handle may be anywhere.
}

template<typename T>
inline void EventQueue<T>::push( T value ) {
    std::lock_guard<std::mutex> lock( queueMutex_ );
    queue_.push( value );
    if ( !newElementEvent_->isSignaled() ) {
        newElementEvent_->setEvent();
    }
}

template<typename T>
inline bool EventQueue<T>::tryPop( T& value ) {
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
inline std::shared_ptr<T> EventQueue<T>::tryPop() {
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
inline bool EventQueue<T>::isEmpty() const {
    return ( !newElementEvent_->isSignaled() );
}

template<typename T>
inline uint64_t EventQueue<T>::size() const {
    std::lock_guard<std::mutex> lock( queueMutex_ );
    return ( queue_.size() );
}

template<typename T>
inline Event* EventQueue<T>::getEventHandle() {
    return newElementEvent_;
}

#endif // CPL_HPP