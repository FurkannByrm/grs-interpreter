# filepath: /home/cengo/interpreter_demos/regex/CMakeLists.txt

cmake_minimum_required(VERSION 3.10)
project(regex_catch)

set(CMAKE_CXX_STANDARD 17)
set(CMAKE_CXX_STANDARD_REQUIRED ON)

# Include directories
include_directories(${CMAKE_CURRENT_SOURCE_DIR}/include)

add_executable(multi_catch src/multi_catch.cpp)
add_executable(catch src/catch.cpp)