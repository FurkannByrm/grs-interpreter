#ifndef EXECUTOR_HPP
#define EXECUTOR_HPP

#include "interpreter/instruction_generator.hpp" 
#include "state_machine.hpp"
#include <queue>

namespace grs_interpreter{

using namespace common;
using prStringAndValueType = const std::pair<std::string, common::ValueType>& ;


    class Executor{

    public:
    Executor();
    void executeInstruction(std::vector<Instruction> instruction);

    void executeLinMotion(prStringAndValueType args);
    void executePtpMotion(prStringAndValueType args);
    void executeCirclMotion(prStringAndValueType args);
    void executeWaitCommand(prStringAndValueType args);
    
    void mockLinearMotion(double& x, double& y, double& z);
    void mockPtpMotion(double& x, double& y, double& z);
    void mockCircMotion(double& x, double& y, double& z);
    void mockWaitFunc(int t);

    private:
    StateMachine stateMachine_;
    void setupStateMachine();


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