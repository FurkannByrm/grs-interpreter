#ifndef INSTRUCTION_EXECUTOR_HPP_
#define INSTRUCTION_EXECUTOR_HPP_
#include <vector>
#include "virtual_memory.hpp"
#include "motion_controller.hpp"
#include "instruction_set.hpp"

namespace krl_interpreter{


class InstructionExecutor{
    public:
    InstructionExecutor();
    ~InstructionExecutor();

    void execute(const std::vector<Instruction>& instruction, VirtualMemory& memory);

    private:
    void executeMove(const Instruction& instruction, VirtualMemory& memory);
    void executeWait(const Instruction& instruction);
};


} //namespace krl_interpreter


#endif // INSTRUCTION_EXECUTOR_HPP_