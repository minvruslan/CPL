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

#include "cpl.hpp"

using namespace cpl;

// ==============================================
// ========== Class CplBase definition ==========
// ==============================================

bool CplBase::initialize() {
    return true;
}

void CplBase::close() {
    return;;
}

// ================================================
// ========== Class IpAddress definition ==========
// ================================================

IpAddress::IpAddress() :
    portNumber_( 0 )
{}

bool IpAddress::setIp( const std::string& ip ) {
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

bool IpAddress::setPortNumber( const uint16_t& portNumber ) {
    if ( 1024 <= portNumber && portNumber <= 65535 ) {
        portNumber_ = portNumber;
        return true;
    }
    else {
        return false;
    }
}

std::string IpAddress::getIp() const {
    return ip_;
}

uint16_t IpAddress::getPortNumber() const {
    return portNumber_;
}

// =================================================
// ========== Class SocketBase definition ==========
// =================================================

SocketBase::SocketBase( SocketType socketType ) :
    socketType_( socketType ),
    socketHandle_( CPL_INVALID_SOCKET_HANDLE )
{}

bool SocketBase::isOpen() const {
    return ( socketHandle_ != CPL_INVALID_SOCKET_HANDLE );
}

void SocketBase::close() {
    if ( socketHandle_ != CPL_INVALID_SOCKET_HANDLE ) {
        ::close( socketHandle_ );
        socketHandle_ = CPL_INVALID_SOCKET_HANDLE;
    }
}

CPL_PLATFORM_SOCKET SocketBase::getSocketHandle() const {
    return socketHandle_;
}

SocketType SocketBase::getSocketType() const {
    return socketType_;
}

// ================================================
// ========== Class UdpSocket definition ==========
// ================================================

UdpSocket::UdpSocket() : SocketBase( SocketType::UDP_SOCKET )
{}

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

int32_t UdpSocket::receiveFrom( uint8_t* bufPtr, uint16_t bufSize, IpAddress& ipAddress ) {
    if ( socketHandle_ == CPL_INVALID_SOCKET_HANDLE ) {
        return CPL_UDP_SOCKET_ERROR_INVALID_SOCKET;
    }

    if ( bufSize == 0 ) {
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

int32_t UdpSocket::sendTo( uint8_t* bufPtr, uint16_t bufSize, IpAddress& ipAddress ) {
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

// ==============================================================
// ========== Class TcpServerExchangeSocket definition ==========
// ==============================================================

TcpServerExchangeSocket::TcpServerExchangeSocket( CPL_PLATFORM_SOCKET  socketHandle) :
    SocketBase( SocketType::TCP_SERVER_EXCHANGE_SOCKET )
{
    socketHandle_ = socketHandle;
}

int32_t TcpServerExchangeSocket::receive( uint8_t* bufPtr, uint16_t bufSize ) {
    if ( socketHandle_ == CPL_INVALID_SOCKET_HANDLE ) {
        return CPL_TCP_SOCKET_ERROR_INVALID_SOCKET;
    }

    if ( bufSize == 0 ) {
        return CPL_TCP_SOCKET_ERROR_INVALID_BUFFER;
    }

    ssize_t numberOfReceivedBytes = ::recv( socketHandle_, bufPtr, bufSize, 0 );

    if ( numberOfReceivedBytes >= 0 ) {
        return ( int32_t )numberOfReceivedBytes;
    }
    else {
        return CPL_TCP_SOCKET_ERROR_RECV_FAILED;
    }
}

int32_t TcpServerExchangeSocket::send( uint8_t* bufPtr, uint16_t bufSize ) {
    if ( socketHandle_ == CPL_INVALID_SOCKET_HANDLE ) {
        return CPL_TCP_SOCKET_ERROR_INVALID_SOCKET;
    }

    if ( bufSize == 0 ) {
        return CPL_TCP_SOCKET_ERROR_INVALID_BUFFER;
    }

    if ( ::send( socketHandle_ , bufPtr , bufSize , 0 ) >= 0 ) {
        return CPL_TCP_SOCKET_SEND_OK;
    }
    else {
        return CPL_TCP_SOCKET_ERROR_SEND_FAILED;
    }
}

// ============================================================
// ========== Class TcpServerListenSocket definition ==========
// ============================================================

TcpServerListenSocket::TcpServerListenSocket() :
    SocketBase( SocketType::TCP_SERVER_LISTEN_SOCKET )
{}

bool TcpServerListenSocket::open( const uint16_t& portNumber,
                                  const bool& nonBlockingModeFlag,
                                  int32_t maxPendingConnections )
{
    socketHandle_ = socket( AF_INET , SOCK_STREAM , 0 );

    if ( socketHandle_ < 0 ) {
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

    if ( listen( socketHandle_ , maxPendingConnections ) <= 0 ) {
        this->close();
        return false;
    }

    return false;
}

TcpServerExchangeSocket TcpServerListenSocket::accept() {
    sockaddr_in clientAddress;
    socklen_t clientAddressLen = sizeof( sockaddr_in );

    CPL_PLATFORM_SOCKET socketHandle = ::accept( socketHandle_, ( sockaddr* )&clientAddress, &clientAddressLen );

    if ( socketHandle >= 0 ) {
        return TcpServerExchangeSocket( socketHandle );
    }
    else {
        return TcpServerExchangeSocket( CPL_INVALID_SOCKET_HANDLE );
    }
}

// ======================================================
// ========== Class TcpClientSocket definition ==========
// ======================================================

TcpClientSocket::TcpClientSocket() : SocketBase( SocketType::TCP_CLIENT_SOCKET )
{}

bool TcpClientSocket::open( const uint16_t& portNumber, const bool& nonBlockingModeFlag ) {
    socketHandle_ = ::socket( AF_INET, SOCK_STREAM, 0 );

    if ( socketHandle_ < 0 ) {
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

bool TcpClientSocket::connect( IpAddress& ipAddress ) {
    if ( socketHandle_ == CPL_INVALID_SOCKET_HANDLE ) {
        return false;
    }

    sockaddr_in destinationAddress;

    destinationAddress.sin_addr.s_addr = inet_addr( ipAddress.getIp().data() );
    destinationAddress.sin_family = AF_INET;
    destinationAddress.sin_port = htons( ipAddress.getPortNumber() );

    return  ( ::connect( socketHandle_ , ( sockaddr* )&destinationAddress , sizeof( destinationAddress ) ) >= 0 );
}

int32_t TcpClientSocket::receive( uint8_t* bufPtr, uint16_t bufSize ) {
    if ( socketHandle_ == CPL_INVALID_SOCKET_HANDLE ) {
        return CPL_TCP_SOCKET_ERROR_INVALID_SOCKET;
    }

    if ( bufSize == 0 ) {
        return CPL_TCP_SOCKET_ERROR_INVALID_BUFFER;
    }

    ssize_t numberOfReceivedBytes = ::recv( socketHandle_, bufPtr, bufSize, 0 );

    if ( numberOfReceivedBytes < 0 ) {
        return CPL_TCP_SOCKET_ERROR_RECV_FAILED;
    }

    return ( int32_t )numberOfReceivedBytes;
}

int32_t TcpClientSocket::send( uint8_t* bufPtr, uint16_t bufSize) {
    if ( socketHandle_ == CPL_INVALID_SOCKET_HANDLE ) {
        return CPL_TCP_SOCKET_ERROR_INVALID_SOCKET;
    }

    if ( bufSize == 0 ) {
        return CPL_TCP_SOCKET_ERROR_INVALID_BUFFER;
    }

    if ( ::send( socketHandle_ , bufPtr , bufSize , 0 ) >= 0 ) {
        return CPL_TCP_SOCKET_SEND_OK;
    }
    else {
        return CPL_TCP_SOCKET_ERROR_SEND_FAILED;
    }
}

// ============================================
// ========== Class Event definition ==========
// ============================================

Event::Event() :
    signaled_( false ),
    eventHandle_( CPL_INVALID_EVENT_HANDLE ),
    isSocketEvent_( false ),
    socketEventTypes_( 0x00 )
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
    return ( eventHandle_ != CPL_INVALID_EVENT_HANDLE );
}

bool Event::initializeEvent( UdpSocket& udpSocket, CPL_SOCKET_EVENT_TYPES socketEventTypes ) {
    if( !udpSocket.isOpen() ) {
        return false;
    }

    if ( socketEventTypes == CPL_SOCKET_EVENT_TYPE_READ  ||
         socketEventTypes == CPL_SOCKET_EVENT_TYPE_WRITE ||
         socketEventTypes == ( CPL_SOCKET_EVENT_TYPE_READ | CPL_SOCKET_EVENT_TYPE_WRITE ) )
    {
        eventHandle_ = udpSocket.getSocketHandle();
        isSocketEvent_ = true;
        socketEventTypes_ = socketEventTypes;

        return true;
    }
    else {
        return false;
    }
}

bool Event::initializeEvent( TcpServerListenSocket& tcpServerListenSocket,
                             CPL_SOCKET_EVENT_TYPES socketEventTypes)
{
    if( !tcpServerListenSocket.isOpen() ) {
        return false;
    }

    if ( socketEventTypes == CPL_SOCKET_EVENT_TYPE_ACCEPT ) {
        eventHandle_ = tcpServerListenSocket.getSocketHandle();
        isSocketEvent_ = true;
        socketEventTypes_ = socketEventTypes;
        return true;
    }
    else {
        return false;
    }
}

bool Event::initializeEvent( TcpServerExchangeSocket& tcpServerExchangeSocket,
                             CPL_SOCKET_EVENT_TYPES socketEventTypes )
{
    if( !tcpServerExchangeSocket.isOpen() ) {
        return false;
    }

    if ( socketEventTypes == CPL_SOCKET_EVENT_TYPE_READ  ||
         socketEventTypes == CPL_SOCKET_EVENT_TYPE_WRITE ||
         socketEventTypes == ( CPL_SOCKET_EVENT_TYPE_READ | CPL_SOCKET_EVENT_TYPE_WRITE ) )
    {
        eventHandle_ = tcpServerExchangeSocket.getSocketHandle();
        isSocketEvent_ = true;
        socketEventTypes_ = socketEventTypes;

        return true;
    }
    else {
        return false;
    }
}

bool Event::initializeEvent( TcpClientSocket& tcpClientSocket,
                             CPL_SOCKET_EVENT_TYPES socketEventTypes )
{
    if( !tcpClientSocket.isOpen() ) {
        return false;
    }

    if ( socketEventTypes == CPL_SOCKET_EVENT_TYPE_READ  ||
         socketEventTypes == CPL_SOCKET_EVENT_TYPE_WRITE ||
         socketEventTypes == ( CPL_SOCKET_EVENT_TYPE_READ | CPL_SOCKET_EVENT_TYPE_WRITE ) )
    {
        eventHandle_ = tcpClientSocket.getSocketHandle();
        isSocketEvent_ = true;
        socketEventTypes_ = socketEventTypes;

        return true;
    }
    else {
        return false;
    }
}

bool Event::setEvent() {
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

    return ( eventfd_read( eventHandle_, &eventfdt ) != -1 );
}

bool Event::isSocketEvent() const {
    return isSocketEvent_;
}

CPL_SOCKET_EVENT_TYPES Event::getSocketEventTypes() const {
    return socketEventTypes_;
}

// =====================================================
// ========== Class EventExpectant definition ==========
// =====================================================

uint32_t EventExpectant::waitForEvent( Event* event, uint32_t milliseconds ) {
    if ( event->getEventHandle() == CPL_INVALID_EVENT_HANDLE ) {
        return CPL_EE_WFE_ERROR_INVALID_EVENT_HANDLE;
    }

    int32_t epollFD = epoll_create( 1 );

    if ( epollFD < 0 ) {
        return CPL_EE_WFE_ERROR_FAILED;
    }

    epoll_event epollEvent;
    epollEvent.data.fd = event->getEventHandle();
    if ( !event->isSocketEvent() ) {
        epollEvent.events = EPOLLIN | EPOLLERR | EPOLLHUP;
    }
    else {
        if ( event->getSocketEventTypes() == CPL_SOCKET_EVENT_TYPE_READ) {
            epollEvent.events = EPOLLIN | EPOLLERR | EPOLLHUP;
        }
        else if ( event->getSocketEventTypes() == CPL_SOCKET_EVENT_TYPE_WRITE ) {
            epollEvent.events = EPOLLOUT | EPOLLERR | EPOLLHUP;
        }
        else if ( event->getSocketEventTypes() == ( CPL_SOCKET_EVENT_TYPE_READ |
                                                    CPL_SOCKET_EVENT_TYPE_WRITE ) )
        {
            epollEvent.events = EPOLLIN | EPOLLOUT | EPOLLERR | EPOLLHUP;
        }
        else {
            epollEvent.events = EPOLLERR | EPOLLHUP;
        }
    }
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

uint32_t EventExpectant::waitForEvents( std::vector<Event*>* events,
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
        if ( !events->at( i )->isSocketEvent() ) {
            epollEvent.events = EPOLLIN | EPOLLERR | EPOLLHUP;
        }
        else {
            if ( events->at( i )->getSocketEventTypes() == CPL_SOCKET_EVENT_TYPE_READ) {
                epollEvent.events = EPOLLIN | EPOLLERR | EPOLLHUP;
            }
            else if ( events->at( i )->getSocketEventTypes() == CPL_SOCKET_EVENT_TYPE_WRITE ) {
                epollEvent.events = EPOLLOUT | EPOLLERR | EPOLLHUP;
            }
            else if ( events->at( i )->getSocketEventTypes() == ( CPL_SOCKET_EVENT_TYPE_READ |
                                                                  CPL_SOCKET_EVENT_TYPE_WRITE ) )
            {
                epollEvent.events = EPOLLIN | EPOLLOUT | EPOLLERR | EPOLLHUP;
            }
            else {
                epollEvent.events = EPOLLERR | EPOLLHUP;
            }
        }
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