cmake_minimum_required(VERSION 3.10)
project(krl_interpreter)

set(CMAKE_CXX_STANDARD 17)
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
    src/ast/visitor.cpp
)

set(INTERPRETER
    src/interpreter/virtual_memory.cpp 
    src/interpreter/instruction_set.cpp
    src/interpreter/instruction_executor.cpp
    src/interpreter/motion_controller.cpp
    src/interpreter/state_machine.cpp
)

add_executable(test_lexer tests/test_lexer.cpp ${LEXER})

# add_executable(test_parser tests/test_parser.cpp ${LEXER} ${PARSER} ${AST})