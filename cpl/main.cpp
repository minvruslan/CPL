#include <iostream>

#include "source/cpl.hpp"
#include "source/cpl_event_queue.hpp"

int main() {
    UdpSocket udpSocket;

    if ( !udpSocket.open( 50000, true ) ) {
        std::cout << "Failed to open socket on port 50000." << std::endl;
    }
    else {
        std::cout << "Socket successfully opened on port 50000." << std::endl;
    }

    udpSocket.close();

    getchar();

    return 0;
}