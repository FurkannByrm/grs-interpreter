cmake_minimum_required(VERSION 3.10)
project(krl_interpreter)

set(CMAKE_CXX_STANDARD 17)
set(CMAKE_CXX_STANDARD_REQUIRED ON)

# Include dizinleri
include_directories(${CMAKE_CURRENT_SOURCE_DIR}/include)

# Kaynak dosyaları
set(SOURCES
    src/lexer/token.cpp
    src/lexer/lexer.cpp
)

# Ana uygulama
add_executable(krl_interpreter src/main.cpp ${SOURCES})

# Test uygulaması
add_executable(test_lexer tests/test_lexer.cpp ${SOURCES})