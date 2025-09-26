#include "executor/executor.hpp"
#include <thread>
#include <chrono>

namespace grs_interpreter{

    Executor::Executor() {
        setupStateMachine();
    }

void Executor::setupStateMachine(){
    stateMachine_.addState("IDLE", [](){
    std::cout<<"State: IDLE \n";   
    });

    stateMachine_.addState("RUNNING", [](){
    std::cout<<"State: RUNNING\n";
    });

    stateMachine_.addState("WAITING", [](){
    std::cout<<"State: WAITING\n";
    });
    
    stateMachine_.setInitialState("IDLE");
    }


void Executor::executeLinMotion(prStringAndValueType args){

    auto pos = std::get<common::Position>(args.second);
    mockLinearMotion(pos.x, pos.y, pos.z);
    std::this_thread::sleep_for(std::chrono::milliseconds(2000));
}    

void Executor::executePtpMotion(prStringAndValueType args){

    auto pos = std::get<common::Position>(args.second);
    mockPtpMotion(pos.x, pos.y, pos.z);
    std::this_thread::sleep_for(std::chrono::milliseconds(2000));

}    


void Executor::executeCirclMotion(prStringAndValueType args){

    auto pos = std::get<common::Position>(args.second);
    mockCircMotion(pos.x, pos.y, pos.z);
    std::this_thread::sleep_for(std::chrono::milliseconds(2000));

} 

void Executor::executeWaitCommand(prStringAndValueType args){

    auto t = std::get<double>(args.second);
    mockWaitFunc(t);
    std::this_thread::sleep_for(std::chrono::milliseconds(2000));
} 


void Executor::executeInstruction(std::vector<Instruction> instruction){

    stateMachine_.convertState("RUNNING");
    stateMachine_.executeCurrentState();
    
    for(const auto& inst : instruction)
    {        
        
        if(inst.command == "LIN" || inst.command == "PTP" || inst.command == "CIRC" || inst.command == "WAIT")
        {
           
            switch (typeToStringCommand.at(inst.command))
            {
                case CommandCategories::LIN:
                executeLinMotion(inst.args.back());
                break;
                
                case CommandCategories::PTP: 
                executePtpMotion(inst.args.back());
                break;

                case CommandCategories::CIRC: 
                executeCirclMotion(inst.args.back());
                break;

                case CommandCategories::WAIT: 
                executeWaitCommand(inst.args.back());
                break;
            }              
            
        }

    }


}


void Executor::mockLinearMotion(double& x, double& y, double& z){
    std::cout<<"REALTIME LINEAR || "<<
    "x: "<<x<<" "<<"y: "<<y<<" "<<"z: "<<z<<"\n"; 
}

void Executor::mockPtpMotion(double& x, double& y, double& z){
    std::cout<<"REALTIME PTP || "<<
    "x: "<<x<<" "<<"y: "<<y<<" "<<"z: "<<z<<"\n"; 
}

void Executor::mockCircMotion(double& x, double& y, double& z){
    std::cout<<"REALTIME CIRCL || "<<
    "x: "<<x<<" "<<"y: "<<y<<" "<<"z: "<<z<<"\n"; 
}

void Executor::mockWaitFunc(int t){
    std::cout<<"REALTIME WAIT || "<<
    "time: "<<t<<"\n"; 
}



}