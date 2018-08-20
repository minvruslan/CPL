cmake_minimum_required( VERSION 3.10 )

project( cpl )

set( CMAKE_CXX_STANDARD 11 )

set( SOURCE_FILES source/cpl.hpp
                  source/cpl.cpp
                  source/cpl_event_queue.hpp )

add_executable( cpl main.cpp ${SOURCE_FILES} )