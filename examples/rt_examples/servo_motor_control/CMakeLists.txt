cmake_minimum_required(VERSION 3.10)

project(ServoMotorControl)

set(CMAKE_CXX_STANDARD 17)
set(CMAKE_CXX_STANDARD_REQUIRED ON)

//set(ETHERCAT_ROOT "/home/rnd/ethercat")

include_directories(${ETHERCAT_ROOT}/include)

add_executable(beckoff_servo_control src/main.cpp)

target_link_directories(beckoff_servo_control PRIVATE ${ETHERCAT_ROOT}/lib/.libs)

target_link_libraries(beckoff_servo_control 
    PRIVATE 
    ethercat 
    rt 
    pthread
)

add_executable(test_minimal src/test_minimal.cpp)

target_link_directories(test_minimal PRIVATE ${ETHERCAT_ROOT}/lib/.libs)

target_link_libraries(test_minimal 
    PRIVATE 
    ethercat 
    rt 
    pthread
)
