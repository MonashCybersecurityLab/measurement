cmake_minimum_required(VERSION 3.15)
project(measurement)

set(CMAKE_CXX_STANDARD 14)
set(CMAKE_VERBOSE_MAKEFILE 1)
add_custom_target(measurement
        COMMAND make -j8 SGX_MODE=SIM SGX_DEBUG=1
        WORKING_DIRECTORY ${CMAKE_CURRENT_SOURCE_DIR})

add_custom_target(cleanup
        COMMAND make -j8 clean
        WORKING_DIRECTORY ${CMAKE_CURRENT_SOURCE_DIR})