#ifndef STATE_MACHINE_HPP
#define STATE_MACHINE_HPP

#include <string>
#include <unordered_map>
#include <functional>
#include <stdexcept>

namespace grs_interpreter{

class StateMachine{
    
public:

using stateFunction = std::function<void()>;

StateMachine();

void addState(std::string stateName, stateFunction stateFunc);
void convertState(std::string stateName);
void setInitialState(std::string stateName);
void executeCurrentState();
std::string getCurrentState()const;


private:

std::string currentState_;
std::unordered_map<std::string, stateFunction> stateMap;

};
}

#endif //STATE_MACHINE_HPP