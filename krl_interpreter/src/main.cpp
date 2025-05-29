#include <iostream>
#include <fstream>
#include <sstream>
#include <string>
#include <vector>
#include <memory>
#include <chrono>
#include <thread>

#include "lexer/lexer.hpp"
#include "parser/parser.hpp"
#include "interpreter/instruction_executor.hpp"
#include "interpreter/instruction_set.hpp"
#include "interpreter/motion_controller.hpp"
#include "interpreter/state_machine.hpp"
#include "interpreter/motion_controller.hpp"


void printUsage( const std::string& programName){
    std::cout<<"Usage: "<< programName<<" <krl_file>"<<std::endl;
}

std::string readFile(const std::string& filename){
    std::ifstream file(filename);
    if(!file.is_open()){
        throw std::runtime_error("Failed to open file: " + filename);
    }
    std::stringstream buffer;
    buffer<<file.rdbuf();
    return buffer.str();
}


int main(int argc, char* argv[])
{   
    if(argc < 2){
        printUsage(argv[0]);
        return 1;
    }
    try {
        //Read Code
        std::string krlCode = readFile(argv[1]);
        std::cout<<"Read KRL code from file: "<< argv[1]<< std::endl;

        //Tokenize
        krl_lexer::Lexer lexer;
        auto tokens = lexer.tokenize(krlCode);
        if(lexer.hasErrors()){
            std::cout<<"Lexer errors: "<<std::endl;
            for(const auto& error : lexer.getErrors())
            {
                std::cout<<error.getFormattedMessage()<<std::endl;
            }
            return 1;
        }
        std::cout<<"Tokenization copleted. "<<tokens.size()<<std::endl;

        //Parse
        krl_parser::Parser parser;
        auto ast = parser.parse(tokens);
        if(parser.hasErrors()){
            std::cout<<"Parser errors: "<<std::endl;
            for(const auto& error : parser.getErrors()){
                std::cout<<"Line "<< error.line<< ", Column "<<error.column
                        <<": "<<error.message<<std::endl;
            }
            return 1;
        }
        std::cout<<"Parsing completed. AST generated."<<std::endl;

        //Instruction Generation
        krl_interpreter::InstructionGenerator generator;
        auto instructions = generator.generateInstructions(ast);
        std::cout<<"Instructions generation completed." << instructions.size()
                 <<" instructions generated"<<std::endl;

        krl_interpreter::VirtualMemory memory;
        krl_interpreter::StateMachine stateMachine;
        krl_interpreter::MotionController motionController;
        krl_interpreter::InstructionExecutor executor;

        //state machine initialization
        stateMachine.setInitialState(krl_interpreter::State::IDLE);
        stateMachine.addTransition(krl_interpreter::State::IDLE, krl_interpreter::State::RUNNING, "start");
        stateMachine.addTransition(krl_interpreter::State::RUNNING, krl_interpreter::State::STOPPED, "stop");
        stateMachine.addTransition(krl_interpreter::State::RUNNING, krl_interpreter::State::EMERGENCY, "emergency");
        stateMachine.addTransition(krl_interpreter::State::STOPPED, krl_interpreter::State::IDLE, "reset");
        stateMachine.addTransition(krl_interpreter::State::EMERGENCY, krl_interpreter::State::IDLE, "reset");
        
        //star program execution
        std::cout<<"Starting program execution..."<<std::endl;
        stateMachine.triggerEvent("start");

        //Execute instruction
        if(stateMachine.getCurrentState() == krl_interpreter::State::RUNNING){
            std::cout<< "Execution instruction..."<<std::endl;

            krl_interpreter::InstructionSet instructionSet;
            for(const auto& instruction : instructions){
                instructionSet.addInstruction(instruction);
            }
            executor.execute(instructionSet.getInstructions(), memory);
        }
        std::cout<<"Execution completed"<<std::endl;
        stateMachine.triggerEvent("stop");
        return 0;
    } catch(const std::exception& e){
        std::cerr <<"Error: "<<e.what()<<std::endl;
        return -1;
    }

}