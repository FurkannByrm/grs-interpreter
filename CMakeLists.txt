cmake_minimim_required(VERSION 3.10)

project(INTERPRETER)

set(CMAKE_CXX_STANDARD 17)

set(CMAKE_CXX_STANDARD_REQUIRED True)

set(SOURCES
	src/main.cpp
	src/interpreter.cpp
)

add_executable(exe ${SOURCES})
include_directories(${CMAKE_SOURCE_DIR}/include)
