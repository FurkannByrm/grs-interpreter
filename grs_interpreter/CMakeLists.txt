cmake_minimum_required(VERSION 3.10)
project(grs_interpreter)

set(CMAKE_CXX_STANDARD 17)
set(CMAKE_CXX_STANDARD_REQUIRED ON)

include_directories(${CMAKE_CURRENT_SOURCE_DIR}/include)
#include_directories(${CMAKE_SOURCE_DIR}/lexer)

add_library(constexpr_map_lib INTERFACE)

target_include_directories(constexpr_map_lib INTERFACE $ENV{HOME}/constexpr_map/include)

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

set(IO 
    src/io/io_provider.cpp
    src/io/tcp_io_provider.cpp)

set(STEP_EXECUTOR
    src/executor/step_executor.cpp)

set(INTERPRETER
    src/interpreter/instruction_generator.cpp
)

# set(EXECUTOR
#     src/executor/executor.cpp
#     src/executor/state_machine.cpp
# )

add_executable(interpreter src/main.cpp 
    ${LEXER} 
    ${PARSER} 
    ${AST} 
    ${INTERPRETER} 
    # ${EXECUTOR}
)

target_link_libraries(interpreter PRIVATE constexpr_map_lib)


add_executable(grs_step src/step_main.cpp
    ${LEXER}
    ${PARSER}
    ${AST}
    ${IO}
    ${STEP_EXECUTOR}
)

target_link_libraries(grs_step PRIVATE constexpr_map_lib pthread)
