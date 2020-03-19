cmake_minimum_required(VERSION 3.15)
project(measurement)

set(CMAKE_CXX_STANDARD 14)
set(CMAKE_VERBOSE_MAKEFILE 1)

# Use cumtom target to build SGX program
add_custom_target(strawman
        COMMAND make -j8 SGX_MODE=SIM SGX_DEBUG=1
        WORKING_DIRECTORY ${CMAKE_CURRENT_SOURCE_DIR}/Strawman)

add_custom_target(strawman_HW
        COMMAND make -j8 SGX_MODE=HW SGX_DEBUG=1
        WORKING_DIRECTORY ${CMAKE_CURRENT_SOURCE_DIR}/Strawman)

add_custom_target(oblivious
        COMMAND make -j8 SGX_MODE=SIM SGX_DEBUG=1
        WORKING_DIRECTORY ${CMAKE_CURRENT_SOURCE_DIR}/Oblivious)

add_custom_target(oblivious_HW
        COMMAND make -j8 SGX_MODE=HW SGX_DEBUG=1
        WORKING_DIRECTORY ${CMAKE_CURRENT_SOURCE_DIR}/Oblivious)

add_custom_target(cleanup
        COMMAND make -j8 clean
        WORKING_DIRECTORY ${CMAKE_CURRENT_SOURCE_DIR})

FIND_PACKAGE(OpenSSL REQUIRED)

# include root dir
INCLUDE_DIRECTORIES(Oblivious/Enclave/SpookyHash Oblivious/Enclave/Sketch Common)

ADD_EXECUTABLE(TestSpookyV2 Strawman/Test/TestSpookyV2.cpp Strawman/Enclave/SpookyHash/SpookyV2.cpp)

ADD_EXECUTABLE(TestCMSketch Strawman/Test/TestCMSketch.cpp Strawman/Enclave/SpookyHash/SpookyV2.cpp)

ADD_EXECUTABLE(TestObliviousSketch Oblivious/Test/TestObliviousSketch.cpp Oblivious/Enclave/SpookyHash/SpookyV2.cpp)

ADD_EXECUTABLE(TestObliviousBucket Oblivious/Test/TestObliviousBucket.cpp Oblivious/Enclave/SpookyHash/SpookyV2.cpp Common/CommonUtil.c)

TARGET_LINK_LIBRARIES(TestObliviousSketch OpenSSL::SSL)

TARGET_LINK_LIBRARIES(TestObliviousBucket OpenSSL::SSL)