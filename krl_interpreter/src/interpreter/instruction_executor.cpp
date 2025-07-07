// #include "interpreter/instruction_executor.hpp"

// #include <iostream>
// #include <chrono>
// #include <thread>

// namespace krl_interpreter{


//     InstructionExecutor::InstructionExecutor(){}
//     InstructionExecutor::~InstructionExecutor(){}

//     void InstructionExecutor::execute(const std::vector<Instruction>& instructions, VirtualMemory& memory){
//         MotionController motioncontroller;

//         for(const auto& instruction : instructions){

//             std::cout<<"Executing: "<<instruction.command<< std::endl;
             
//             if(instruction.command == "MOVE" || instruction.command == "PTP" || instruction.command == "LIN" )
//             {
//                 executeMove(instruction, memory);
//             }
//             else if(instruction.command == "WAIT") { 
//                 executeWait(instruction);
//             }
//             else{
//                 std::cerr<<"Unknow instruction: "<<instruction.command<<std::endl;
//             }
//         }

//     }

// void InstructionExecutor::executeMove(const Instruction& instruction, VirtualMemory& memory){
//     MotionController motioncontroller;
//     Position target;
//     MotionParameters params;
//     MotionType motionType = MotionType::PTP;

//     if(instruction.command == "LIN")
//     {
//         motionType = MotionType::LIN;
//     }
//     else if(instruction.command == "CIRC")
//     {
//         motionType = MotionType::CIRC;
//     }
//     else if(instruction.command == "SPL")
//     {
//         motionType = MotionType::SPL;
//     }


//     for(const auto& arg : instruction.args){
//         if(arg.first == "x") target.x = arg.second;
//         else if(arg.first == "Y") target.y = arg.second;
//         else if(arg.first == "Z") target.z = arg.second;
//         else if(arg.first == "A") target.a = arg.second;
//         else if(arg.first == "B") target.b = arg.second;
//         else if(arg.first == "C") target.c = arg.second;
//         else if(arg.first == "VEL") params.velocity = arg.second;
//         else if(arg.first == "ACC") params.acceleration = arg.second;
//         else if(arg.first == "DEC") params.deceleration = arg.second;
//     } 

//     std::cout<<"Motion to X="<<target.x<<" Y="<<target.y<<" Z="<<target.z
//                                <<" A="<<target.a<< " B="<<target.b<<" C="<<target.c<<std::endl;
    
//     motioncontroller.moveToPosition(target,motionType, params);
//     //Wait until motion complete
//     while (motioncontroller.isMoving())
//     {
//         std::this_thread::sleep_for(std::chrono::milliseconds(10));
//     }
        
// }

// void InstructionExecutor::executeWait(const Instruction& Instruction){
//     double waitTime  = 0.0;

//     for(const auto& arg : Instruction.args){
//         if(arg.first == "Time" || arg.first == "SEC"){
//             waitTime = arg.second;
//             break;
//         }
//     }
//     std::cout<<"Waiting for "<<waitTime<<"seconds"<<std::endl;
//     std::this_thread::sleep_for(std::chrono::milliseconds(static_cast<int>(waitTime * 1000)));


// }


// }// namespace krl_interpreter