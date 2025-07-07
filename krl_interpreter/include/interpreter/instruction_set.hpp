#ifndef INSTRUCTION_SET_HPP_
#define INSTRUCTION_SET_HPP_

#include "../ast/ast.hpp"
#include "../ast/visitor.hpp"

namespace krl_interpreter{
    
using ValueType = std::variant<int, double, std::string>;
struct Instruction{

    std::string command;
    std::vector<std::pair<std::string, ValueType>> args;

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
    void visit(krl_ast::LiteraExpression& node) override;
    void visit(krl_ast::VariableExpression& node) override;
    void visit(krl_ast::VariableDeclaration& node) override;
    // void visit(krl_ast::UnaryExpression& node) override;
    // void visit(krl_ast::IfStatement& node) override;
    // void visit(krl_ast::MotionCommand& node) override;

    private:
    std::vector<Instruction> instruction_;
    std::unordered_map<std::string, ValueType> variableValues_;
    ValueType currentValue_;
    ValueType evaluateExpression(const std::shared_ptr<krl_ast::Expression>& expr);
    struct TypeValue{
        krl_lexer::TokenType type;
        ValueType value;
    };

    std::unordered_map<std::string, TypeValue> declaredVariables_;
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