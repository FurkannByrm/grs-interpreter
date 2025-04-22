cmake_minimum_required(VERSION 3.10)
project(krl_interpreter)

set(CMAKE_CXX_STANDARD 17)
set(CMAKE_CXX_STANDARD_REQUIRED ON)

include_directories(${CMAKE_CURRENT_SOURCE_DIR}/include)
include_directories(${CMAKE_SOURCE_DIR}/lexer)
set(SOURCES
    src/lexer/token.cpp
    src/lexer/lexer.cpp
)

add_executable(krl_interpreter src/main.cpp ${SOURCES})
