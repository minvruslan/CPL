#include <iostream>
#include <string>

#include "source/cpl.hpp"
#include "source/cpl_event_queue.hpp"

int main() {
    cpl::CplBase::initialize();

    getchar();

    cpl::CplBase::close();

    return 0;
}