#ifndef EXECUTOR_HPP
#define EXECUTOR_HPP

#include "interpreter/instruction_generator.hpp" 
#include "state_machine.hpp"
#include <queue>

namespace grs_interpreter{

    class Executor{

    public:
    Executor();
    void executeInstruction(std::vector<Instruction> instruction);
    void executeLinMotion(const Instruction& instruction);
    void executePtpMotion(const Instruction& instruction);
    void executeCirclMotion(const Instruction& instruction);
    void executeWaitCommand(const Instruction& instruction);

    private:
    StateMachine stateMachine_;
    void setupStateMachine();
    std::queue<Instruction> instQueue;
    

};

enum class CommandCategories{

    LIN,
    PTP,
    CIRC,
    WAIT

};

inline constexpr auto typeToStringCommand = cxmap::ConstexprMap<std::string_view, CommandCategories, 4>({{

{"LIN",CommandCategories::LIN},
{"PTP", CommandCategories::PTP},
{"CIRC", CommandCategories::CIRC},
{"WAIT", CommandCategories::WAIT}
 
}
});


}



#endif //EXECUTOR_HPP