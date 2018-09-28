#include <iostream>
#include <string>

#include "source/cpl.hpp"
#include "source/cpl_event_queue.hpp"

void application() {
    cpl::TcpServerListenSocket tcpServerListenSocket;

    std::cout << "Starting TCP server." << std::endl;

    tcpServerListenSocket.open( 7070, false, 5 );
    if ( tcpServerListenSocket.isOpen() ) {
        std::cout << "TCP server started." << std::endl;
    }
    else {
        std::cout << "Failed to start TCP server." << std::endl;
        tcpServerListenSocket.close();
        return;
    }

    cpl::TcpServerExchangeSocket tcpServerExchangeSocket;

    tcpServerListenSocket.accept( &tcpServerExchangeSocket );

    uint8_t  buf[ 32 ];
    uint16_t bufSize = 32;


    int32_t bytesRead = tcpServerExchangeSocket.receive( buf, bufSize );
    if ( bytesRead > 0 ) {
        std::string str( ( char* )buf, bytesRead );
        std::cout << "Received message: " << str << std::endl;

    }
    else {
        std::cout << "Failed while receiving message." << std::endl;
    }

    std::cout << "Sending response." << std::endl;
    if ( tcpServerExchangeSocket.send( buf, bytesRead ) == CPL_TCP_SOCKET_SEND_OK ) {
        std::cout << "Successfully send response." << std::endl;
    }
    else {
        std::cout << "Failed to send response." << std::endl;
    }

    std::cout << "Closing sockets." << std::endl;

    tcpServerExchangeSocket.close();
    tcpServerListenSocket.close();
}

int main() {
    if ( !cpl::CplBase::initialize() ) {
        std::cout << "Failed to initialize CPL library." << std::endl;
        getchar();
        return 1;
    }
    else {
        std::cout << "Press any key to shutdown CPL library." << std::endl;
        application();
        getchar();
        cpl::CplBase::shutdown();
        return 0;
    }
}