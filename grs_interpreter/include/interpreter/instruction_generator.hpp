#ifndef INSTRUCTION_GENERATOR_HPP_
#define INSTRUCTION_GENERATOR_HPP_

#include "../ast/ast.hpp"
#include "../ast/visitor.hpp"
#include "../common/utils.hpp"

namespace grs_interpreter{
    

struct Instruction{

    std::string command;
    std::vector<std::pair<std::string, common::ValueType>> args;
    std::vector<std::pair<int,int>> commandLocationInfo;
};

struct VariableInfo{
    grs_lexer::TokenType type;
    common::ValueType value;
};

class InstructionGenerator : public grs_ast::ASTVisitorBase{

    public:
    InstructionGenerator();
    ~InstructionGenerator();

    std::vector<Instruction> generateInstructions(const std::shared_ptr<grs_ast::FunctionBlock>& program);

    //visit methods
    void visit(grs_ast::FunctionBlock& node) override;
    void visit(grs_ast::MotionCommand& node) override;
    void visit(grs_ast::BinaryExpression& node) override;
    void visit(grs_ast::LiteraExpression& node) override;
    void visit(grs_ast::VariableExpression& node) override;
    void visit(grs_ast::VariableDeclaration& node) override;
    
    void visit(grs_ast::FrameDeclaration& node) override;
    void visit(grs_ast::PositionDeclaration& node) override;
    void visit(grs_ast::AxisDeclaration& node) override;
    void visit(grs_ast::ExecutePosAndAxisExpression& node) override;
    void visit(grs_ast::IfStatement& node) override;
    void visit(grs_ast::WaitStatement& node) override;
    void visit(grs_ast::FunctionDeclaration& node) override;
    void visit(grs_ast::UnaryExpression& node) override;
    
    private:
    std::vector<Instruction> instruction_;
    common::ValueType currentValue_;
    common::ValueType evaluateExpression(const std::shared_ptr<grs_ast::Expression>& expr);
    
    std::unordered_map<std::string, VariableInfo> declaredVariables_;
    
    inline bool hasVariable(const std::string& name) const{
        return declaredVariables_.find(name) != declaredVariables_.end();
    }

    inline common::ValueType getVariableValue(const std::string& name) const{
        auto it = declaredVariables_.find(name);
        return (it != declaredVariables_.end()) ? it->second.value : common::ValueType(0.0);
    }
    inline void setVariableValue(const std::string& name, const common::ValueType& value){
        if(hasVariable(name)) {
            declaredVariables_[name].value = value;
        }
    }
    
    inline void assignPosAndAxisExpression( const std::string name, const std::string& argument, const double& value){
        auto type = declaredVariables_[name].type;
        std::cout<<grs_lexer::typeToStringMap.at(type)<<std::endl;
        switch (type)
        {
        case grs_lexer::TokenType::POS:{
            auto& posStruct = std::get<common::Position>(declaredVariables_[name].value);
            if(argument == "x")posStruct.x = value;
            else if(argument == "y")posStruct.y = value;
            else if(argument == "z")posStruct.z = value;
            else if(argument == "a")posStruct.a = value;
            else if(argument == "b")posStruct.b = value;
            else if(argument == "c")posStruct.c = value;}
            break;
        case grs_lexer::TokenType::AXIS:
            {auto& axisStruct = std::get<common::Axis>(declaredVariables_[name].value);
            if(argument == "a1" )axisStruct.A1 = value;
            else if(argument == "a2")axisStruct.A2 = value;
            else if(argument == "a3")axisStruct.A3 = value;
            else if(argument == "a4")axisStruct.A4 = value;
            else if(argument == "a5")axisStruct.A5 = value;
            else if(argument == "a6")axisStruct.A6 = value;}
            break;
        default:
            std::cerr << "Type '" << grs_lexer::typeToStringMap.at(type) 
            << "' does not support member assignment \n";
            break;
        }
    }
    

    template <typename StrucType>
    void declarationType(StrucType& structObject, const std::string& reference, double value){
        if constexpr(std::is_same_v<StrucType, common::Position>)
        {   
            if(reference == "x")structObject.x = value;
            else if(reference == "y")structObject.y = value;
            else if(reference == "z")structObject.z = value;
            else if(reference == "a")structObject.a = value;
            else if(reference == "b")structObject.b = value;
            else if(reference == "c")structObject.c = value;
        }

        else if constexpr(std::is_same_v<StrucType, common::Frame>){
            if(reference == "x")structObject.x = value;
            else if(reference == "y")structObject.y = value;
            else if(reference == "z")structObject.z = value;
            else if(reference == "a")structObject.a = value;
            else if(reference == "b")structObject.b = value;
            else if(reference == "c")structObject.c = value;
        }

        else if constexpr (std::is_same_v<StrucType, common::Axis>)
        {
            if(reference == "A1")structObject.A1 = value;
            else if(reference == "A2")structObject.A2 = value;
            else if(reference == "A3")structObject.A3 = value;
            else if(reference == "A4")structObject.A4 = value;
            else if(reference == "A5")structObject.A5 = value;
            else if(reference == "A6")structObject.A6 = value;
        }
    }
    template<typename NodeType, class StrucType>
    void executeDeclaration(NodeType& node, grs_lexer::TokenType type, const std::string& prefix)
    {   
        StrucType strucType;
        for(const auto& arg : node.getArgs()){
            auto value = evaluateExpression(arg.second);
            auto val = std::get<double>(value);
            declarationType<StrucType>(strucType,arg.first, val);
        }

        declaredVariables_[node.getName()] = {type, strucType};
        Instruction instruction;
        instruction.command = prefix + "_DECL";
        instruction.commandLocationInfo = node.getLineColumn();
        instruction.args.emplace_back("name", node.getName());
        instruction.args.emplace_back(prefix, strucType);
        instruction_.push_back(instruction);
    }
    

    

};



}

#endif //INSTRUCTION_GENERATOR_HPP_