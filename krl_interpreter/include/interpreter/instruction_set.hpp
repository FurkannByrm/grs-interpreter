#ifndef INSTRUCTION_SET_HPP_
#define INSTRUCTION_SET_HPP_

#include "../ast/ast.hpp"
#include "../ast/visitor.hpp"

namespace krl_interpreter{

struct Instruction{

    std::string command;
    std::vector<std::pair<std::string, double>> args;
};

class InstructionGenerator : public krl_ast::ASTVisitorBase{

    public:
    InstructionGenerator();
    ~InstructionGenerator();

    std::vector<Instruction> generateInstructions(const std::shared_ptr<krl_ast::Program>& program);

    //visit methods
    void visit(krl_ast::Program& node) override;
    void visit(krl_ast::Command& node) override;
    void visit(krl_ast::BinaryExpression& node) override;
    void visit(krl_ast::UnaryExpression& node) override;
    void visit(krl_ast::LiteraExpression& node) override;
    void visit(krl_ast::VariableExpression& node) override;
    void visit(krl_ast::IfStatement& node) override;
    void visit(krl_ast::MotionCommand& node) override;

    private:
    std::vector<Instruction> instruction_;
    std::unordered_map<std::string,double> variableValues_;
    double currentValue_;
    double evaluateExpression(const std::shared_ptr<krl_ast::Expression>& expr);

};

class InstructionSet{

    public:
    InstructionSet() = default;
    ~InstructionSet();

    void addInstruction(const Instruction& instruction);
    void clear();
    const std::vector<Instruction>& getInstructions() const;
    size_t size() const;
    const Instruction& getInstruction(size_t index) const;
    private:
    std::vector<Instruction> instructions_;


};



}

#endif //INSTRUCTION_SET_HPP_