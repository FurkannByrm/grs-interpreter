#ifndef STATE_MACHINE_HPP_
#define STATE_MACHINE_HPP_

#include <string>
#include <functional>
#include <unordered_map>
#include <vector>


namespace krl_interpreter{

enum class State{
    IDLE,
    RUNNING,
    EMERGENCY,
    STOPPED
};

struct StateTransition{
    State from;
    State to;
    std::string event;
};

class StateMachine{

    public:
    StateMachine();
    ~StateMachine();

    State getCurrentState() const;
    void setInitialState(State state);
    bool transitionTo(State newState);
    bool triggerEvent(const std::string& event);

    void addTransition(State from, State to, const std::string& event);
    void setStateEntryHandler(State state, std::function<void()> handler);
    void setStateExitHandler(State state, std::function<void()> handler);
    
    private:
    State currentState_;
    std::vector<StateTransition> transition_;
    std::unordered_map<State, std::function<void()>> entryHandlers_;
    std::unordered_map<State, std::function<void()>> exitHandlers_;

};


} // namespace krl_interpreter



#endif // STATE_MACHINE_HPP