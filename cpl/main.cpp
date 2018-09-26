#include <iostream>
#include <string>

#include "source/cpl.hpp"
#include "source/cpl_event_queue.hpp"

int main() {
    if ( !cpl::CplBase::initialize() ) {
        std::cout << "Failed to initialize CPL library." << std::endl;
        getchar();
        return 1;
    }
    else {
        std::cout << "Press any key to shutdown CPL library." << std::endl;
        getchar();
        cpl::CplBase::shutdown();
        return 0;
    }
}