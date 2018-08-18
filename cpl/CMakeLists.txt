cmake_minimum_required( VERSION 3.10 )
project( cpl )

set( CMAKE_CXX_STANDARD 11 )

add_executable( cpl
                cpl.hpp
                cpl_tests.cpp
                cpl_examples.cpp )