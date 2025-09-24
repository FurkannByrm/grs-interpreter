cmake_minimum_required(VERSION 3.10)
project(grs_interpreter)

set(CMAKE_CXX_STANDARD 20)
set(CMAKE_CXX_STANDARD_REQUIRED ON)

include_directories(${CMAKE_CURRENT_SOURCE_DIR}/include)
include_directories(${CMAKE_SOURCE_DIR}/lexer)
set(LEXER
    src/lexer/token.cpp
    src/lexer/lexer.cpp
)
set(PARSER
    src/parser/parser.cpp
)

set(AST
    src/ast/ast.cpp
)

set(INTERPRETER
    src/interpreter/instruction_generator.cpp
)

set(EXECUTOR
    src/executor/executor.cpp
    src/executor/state_machine.cpp
)

add_executable(interpreter src/main.cpp ${LEXER} ${PARSER} ${AST} ${INTERPRETER} ${EXECUTOR})

add_subdirectory(/home/cengo/constexpr_map constexpr_map)

target_link_libraries(interpreter PRIVATE constexpr_map)
