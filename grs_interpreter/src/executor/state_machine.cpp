#include "executor/state_machine.hpp"

namespace grs_interpreter{
 
    StateMachine::StateMachine() : currentState_{"---"} {}

    void StateMachine::addState(std::string stateName, stateFunction stateFunc){
    
        stateMap[stateName] = stateFunc;
    
    }

    void StateMachine::convertState(std::string stateName){
        if(stateMap.find(stateName) != stateMap.end())
        {
            currentState_ = stateName;
        }
        else
        {
            throw std::runtime_error("State not found: " + stateName);
        }
    }

    void StateMachine::setInitialState(std::string stateName){
        currentState_ = stateName;
    }

   void StateMachine::executeCurrentState(){

        if(stateMap.find(currentState_) != stateMap.end()){
            stateMap[currentState_]();
        }
        else
        {
            throw std::runtime_error("No valid state set.");
        }

    }

    std::string StateMachine::getCurrentState() const{
        return currentState_;
    }



}