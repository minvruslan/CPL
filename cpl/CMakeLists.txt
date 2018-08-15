cmake_minimum_required(VERSION 3.10)
project(cpl)

set(CMAKE_CXX_STANDARD 11)

add_executable(cpl cpl_tests.cpp cpl.hpp cpl_examples.cpp)