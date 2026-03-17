cmake_minimum_required(VERSION 3.10)
project(ecrt_control)

set(CMAKE_CXX_STANDARD 17)

include_directories(${CMAKE_CURRENT_SOURCE_DIR}/common)

add_executable(holly_client src/client.cpp)
