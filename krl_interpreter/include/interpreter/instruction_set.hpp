#ifndef INSTRUCTION_SET_HPP_
#define INSTRUCTION_SET_HPP_

#include "../ast/ast.hpp"
#include "../ast/visitor.hpp"
#include "../common/utils.hpp"
#include "motion_controller.hpp"

namespace krl_interpreter{
    
using common::ValueType;
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

    void visit(krl_ast::FrameDeclaration& node) override;
    void visit(krl_ast::PositionDeclaration& node) override;
    void visit(krl_ast::AxisDeclaration& node) override;
    void visit(krl_ast::IfStatement& node) override;
    // void visit(krl_ast::UnaryExpression& node) override;
    // void visit(krl_ast::MotionCommand& node) override;

    private:
    std::vector<Instruction> instruction_;
    // std::unordered_map<std::string, ValueType> variableValues_;
    ValueType currentValue_;
    ValueType evaluateExpression(const std::shared_ptr<krl_ast::Expression>& expr);
    
    struct TypeValue{
        krl_lexer::TokenType type;
        ValueType value;
    };

    std::unordered_map<std::string, TypeValue> declaredVariables_;
    
    inline bool hasVariable(const std::string& name) const{
        return declaredVariables_.find(name) != declaredVariables_.end();
    }

    inline ValueType getVariableValue(const std::string& name) const{
        auto it = declaredVariables_.find(name);
        return (it != declaredVariables_.end()) ? it->second.value : ValueType(0.0);
    }
    inline void setVariableValue(const std::string& name, const ValueType& value){
        if(hasVariable(name)) {
            declaredVariables_[name].value = value;
        }
    }

    template <typename StrucType>
    void declarationType(StrucType& structObject, const std::string& reference, double value){
        if constexpr(std::is_same_v<StrucType, Position>)
        {   
            if(reference == "x")structObject.x = value;
            else if(reference == "y")structObject.y = value;
            else if(reference == "z")structObject.z = value;
            else if(reference == "a")structObject.a = value;
            else if(reference == "b")structObject.b = value;
            else if(reference == "c")structObject.c = value;
        }

        else if constexpr(std::is_same_v<StrucType, Frame>){
            if(reference == "x")structObject.x = value;
            else if(reference == "y")structObject.y = value;
            else if(reference == "z")structObject.z = value;
            else if(reference == "a")structObject.a = value;
            else if(reference == "b")structObject.b = value;
            else if(reference == "c")structObject.c = value;
        }

        else if constexpr (std::is_same_v<StrucType, Axis>)
        {
            if(reference == "A1")structObject.A1 = value;
            else if(reference == "A2")structObject.A2 = value;
            else if(reference == "A3")structObject.A3 = value;
            else if(reference == "A4")structObject.A4 = value;
            else if(reference == "A5")structObject.A5 = value;
            else if(reference == "A6")structObject.A6 = value;
        }
    }
    template<typename NodeType, class StructType>
    void executeDeclaration(NodeType& node, krl_lexer::TokenType type, const std::string& prefix)
    {   
        StructType structType;
        for(const auto& arg : node.getArgs()){
            auto value = evaluateExpression(arg.second);
            double val = std::get<double>(value);
            declarationType<StructType>(structType,arg.first, val);
        }

        declaredVariables_[node.getName()] = {type, structType};
        Instruction instruction;
        instruction.command = prefix + "_DECL";
        instruction.args.emplace_back("name", node.getName());
        instruction.args.emplace_back(prefix, structType);
        instruction_.push_back(instruction);
    }

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