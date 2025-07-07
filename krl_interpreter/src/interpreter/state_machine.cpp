#include "interpreter/state_machine.hpp"
#include <iostream>


namespace  krl_interpreter{

StateMachine::StateMachine() : currentState_(State::IDLE) {}
StateMachine::~StateMachine(){}

State StateMachine::getCurrentState() const {
    return currentState_;
}

void StateMachine::setInitialState(State state){
    currentState_ = state;
    auto it = entryHandlers_.find(currentState_);
    if(it != entryHandlers_.end() && it->second){
        it->second();
    }
}
bool StateMachine::transitionTo(State newState){
    auto exitIt = exitHandlers_.find(currentState_);
    if(exitIt != exitHandlers_.end() && exitIt->second)
    {
        exitIt->second();
    }

State oldState = currentState_;
currentState_ = newState;

auto entryIt = entryHandlers_.find(currentState_);
if(entryIt != entryHandlers_.end() && entryIt->second)
{
    entryIt->second();
}

std::cout<<"State transition: "<<static_cast<int>(oldState)
<<"->"<<static_cast<int>(currentState_)<<std::endl;

return true;

}

bool StateMachine::triggerEvent(const std::string& event){
    for(const auto& transition : transition_){
        if(transition.from == currentState_ && transition.event == event){
            return transitionTo(transition.to);
        }
    }

    std::cout<<"No transition found for event: "<< event
             <<"in state:"<<static_cast<int>(currentState_)<<std::endl;
             return false;
}

void StateMachine::addTransition(State from, State to, const std::string& event){
        transition_.push_back({from, to, event});
}

void StateMachine::setStateEntryHandler(State state, std::function<void()> handler){
    entryHandlers_[state] = handler;
}
void StateMachine::setStateExitHandler(State state, std::function<void()> handler){
    exitHandlers_[state] = handler;
}


}
