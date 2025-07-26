// #include "interpreter/instruction_executor.hpp"
// #include "interpreter/grass_motion_adapter.hpp"
// #include <iostream>
// #include <chrono>
// #include <thread>

// namespace krl_interpreter{

//     InstructionExecutor::InstructionExecutor(){}
//     InstructionExecutor::~InstructionExecutor(){}

//     void InstructionExecutor::execute(const std::vector<Instruction>& instructions, VirtualMemory& memory){
//         // Try to use GRASS Motion Controller, fallback to simulation if not available
//         std::unique_ptr<MotionController> motionController;
        
//         try {
//             auto grassAdapter = std::make_unique<GrassMotionAdapter>();
//             if (grassAdapter->initialize("../GRASS_Motion_Controller/configs/axis_1_config.json", 6)) {
//                 motionController = std::move(grassAdapter);
//                 std::cout << "Using GRASS Motion Controller" << std::endl;
//             } else {
//                 motionController = std::make_unique<MotionController>();
//                 std::cout << "Using simulation motion controller" << std::endl;
//             }
//         } catch (const std::exception& e) {
//             std::cerr << "Failed to initialize GRASS Motion Controller: " << e.what() << std::endl;
//             motionController = std::make_unique<MotionController>();
//             std::cout << "Using simulation motion controller" << std::endl;
//         }

//         for(const auto& instruction : instructions){
//             std::cout<<"Executing: "<<instruction.command<< std::endl;
             
//             if(instruction.command == "MOVE" || instruction.command == "PTP" || instruction.command == "LIN" )
//             {
//                 executeMove(instruction, memory, *motionController);
//             }
//             else if(instruction.command == "WAIT") { 
//                 executeWait(instruction, *motionController);
//             }
//             else{
//                 std::cerr<<"Unknown instruction: "<<instruction.command<<std::endl;
//             }
//         }
//     }

// void InstructionExecutor::executeMove(const Instruction& instruction, VirtualMemory& memory, MotionController& motionController){
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
//         if(arg.first == "X") target.x = arg.second;
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
    
//     motionController.moveToPosition(target, motionType, params);
    
//     //Wait until motion complete
//     while (motionController.isMoving())
//     {
//         std::this_thread::sleep_for(std::chrono::milliseconds(10));
//     }
// }

// void InstructionExecutor::executeWait(const Instruction& instruction, MotionController& motionController){
//     double waitTime = 0.0;

//     for(const auto& arg : instruction.args){
//         if(arg.first == "SEC") waitTime = arg.second;
//     }

//     if(waitTime > 0.0) {
//         motionController.wait(waitTime);
//     }
// }

// } // namespace krl_interpreter