cmake_minimum_required(VERSION 3.10)
project(multiprocess_test)

set(CMAKE_CXX_STANDARD 20)
set(CMAKE_CXX_STANDARD_REQUIRED ON)


include_directories(${CMAKE_CURRENT_SOURCE_DIR}/include)

add_executable(testind src/fork_with_process.cpp)

add_executable(gui_process src/gui_process.cpp)
add_executable(rt_process src/rt_process.cpp)
