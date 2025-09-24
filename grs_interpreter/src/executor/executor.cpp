#include "executor/executor.hpp"

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


void Executor::executeLinMotion(const Instruction& instruction){

    for(const auto& args : instruction.args)
    {
        
        if(args.first == "Position Information"){
            std::cout << "  " << "LIN Command is executing to Position "<< " => ";
            auto pos = std::get<common::Position>(args.second);
            
            std::cout<<""<<pos<<"\n";
        }
        
    }


}    

void Executor::executePtpMotion(const Instruction& instruction){

    for(const auto& args : instruction.args)
    {
        
        if(args.first == "Position Information"){
            std::cout << "  " << "PTP Command is executing to Position"<< " => ";
            auto pos = std::get<common::Position>(args.second);
            std::cout<<pos<<"\n";
        }
    }


}    


void Executor::executeCirclMotion(const Instruction& instruction){

    for(const auto& args : instruction.args)
    {
        
        if(args.first == "Position Information"){
            std::cout << "  " << "CIRC Command is executing to Position "<< " => ";
            auto pos = std::get<common::Position>(args.second);
            std::cout<<pos<<"\n";
        }
    }


} 

void Executor::executeWaitCommand(const Instruction& instruction){

    for(const auto& args : instruction.args)
    {
        
        if(args.first == "duration_time"){
            std::cout << "  " <<"Waiting Command is executing as " << " = ";
            auto time = std::get<int>(args.second);
            std::cout<<time<<"\n";
        }
    }


} 



void Executor::executeInstruction(std::vector<Instruction> instruction){

    stateMachine_.convertState("RUNNING");

    
    for(const auto& inst : instruction)
    {        
        if(inst.command == "LIN" || inst.command == "PTP" || inst.command == "CIRC" || inst.command == "WAIT")
        {
            
            instQueue.push(inst);

            while(instQueue.empty() == 0)
                {
                    switch (typeToStringCommand.at(instQueue.front().command))
                    {
                    case CommandCategories::LIN:
                    executeLinMotion(instQueue.front());
                    break;
                    
                    case CommandCategories::PTP: 
                    executePtpMotion(instQueue.front());
                    break;
        
                    case CommandCategories::CIRC: 
                    executeCirclMotion(instQueue.front());
                    break;
        
                    case CommandCategories::WAIT: 
                    executeWaitCommand(instQueue.front());
                    break;
                    }
                    instQueue.pop();
        
                }
    
            
        }



    }


}



}