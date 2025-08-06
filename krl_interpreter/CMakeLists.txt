cmake_minimum_required(VERSION 3.10)
project(krl_interpreter)

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
    src/interpreter/instruction_set.cpp
)

add_executable(test_interpreter tests/test_interpreter.cpp ${LEXER} ${PARSER} ${AST} ${INTERPRETER})

add_subdirectory(/home/cengo/constexpr_map constexpr_map)

target_link_libraries(test_interpreter PRIVATE constexpr_map)

# add_executable(test_ast tests/test_ast.cpp ${LEXER} ${PARSER} ${AST})