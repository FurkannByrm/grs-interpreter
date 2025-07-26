#include "interpreter/instruction_set.hpp"
#include <iostream>


namespace krl_interpreter{


InstructionGenerator::InstructionGenerator(){}
InstructionGenerator::~InstructionGenerator(){}


std::vector<Instruction> InstructionGenerator::generateInstructions(const std::shared_ptr<krl_ast::Program>& program){
    instruction_.clear();
    if(program){
        program->accept(*this);
    }
    return instruction_;
}

void InstructionGenerator::visit(krl_ast::Program& node){
    for(const auto& statement : node.getStatements())
    {
        if(statement)
        {
            statement->accept(*this);
        }
    }
}

void InstructionGenerator::visit(krl_ast::Command& node){
    Instruction instruction;
    instruction.command = node.getCommand();

    for (const auto& arg : node.getArgs()) {
        instruction.args.emplace_back(arg.first, evaluateExpression(arg.second));
    }

    instruction_.push_back(instruction);
}

void InstructionGenerator::visit(krl_ast::BinaryExpression& node){
    auto leftExpr = std::dynamic_pointer_cast<krl_ast::Expression>(node.getLeft());
    auto rightExpr = std::dynamic_pointer_cast<krl_ast::Expression>(node.getRight());

    if(double baseVal = 0.0; node.getOperator() == krl_lexer::TokenType::ASSIGN){
        auto varExpr = std::dynamic_pointer_cast<krl_ast::VariableExpression>(node.getLeft());
        if(!varExpr){
            std::cerr<<"assignment left side must be a variable \n";
            return;
        }
        std::string varName = varExpr->getName();

        if(!hasVariable(varName)){
            std::cerr<<"Undefined variable: "<< varName<<std::endl;
            return;
        }


        ValueType rightValue = evaluateExpression(node.getRight());

        krl_lexer::TokenType targetType = declaredVariables_[varName].type;
        if (std::holds_alternative<double>(rightValue)) {
            baseVal = std::get<double>(rightValue);
        } else if (std::holds_alternative<int>(rightValue)) {
            baseVal = static_cast<double>(std::get<int>(rightValue));
        }

        switch (targetType) {
            case krl_lexer::TokenType::INT:
                setVariableValue(varName, static_cast<int>(baseVal));
                break;
            case krl_lexer::TokenType::CHAR:
                setVariableValue(varName,static_cast<char>(static_cast<int>(baseVal))); 
                break;
            case krl_lexer::TokenType::BOOL:
                setVariableValue(varName, (baseVal != 0.0));
                break;
            case krl_lexer::TokenType::REAL:
            default:
                setVariableValue(varName, baseVal);
                break;
        }

        std::vector<std::pair<std::string, ValueType>> args;
        args.push_back({"value", getVariableValue(varName)});
        args.push_back({"type", static_cast<double>(static_cast<int>(targetType))});
        instruction_.push_back({"ASSIGN_" + varName, args});
        return;
    }

    

    ValueType leftValue     = evaluateExpression(leftExpr);
    ValueType rightValue   = evaluateExpression(rightExpr);

    switch(node.getOperator()){

        case krl_lexer::TokenType::PLUS:
            currentValue_ = std::get<double>(leftValue) + std::get<double>(rightValue);
            break;
        case krl_lexer::TokenType::MINUS:
            currentValue_ = std::get<double>(leftValue) - std::get<double>(rightValue);
            break;
        case krl_lexer::TokenType::MULTIPLY:
            currentValue_ = std::get<double>(leftValue) * std::get<double>(rightValue);
            break;
            case krl_lexer::TokenType::DIVIDE: 
            if(std::get<double>(rightValue) == 0){
                std::cerr<<"Error: Division by zero"<<std::endl;
                currentValue_ = 0;
            }
            else{
                currentValue_= std::get<double>(leftValue) / std::get<double>(rightValue);
            }
            break;
        default:
            std::cerr<<"unsupported binary operator"<<std::endl;
            currentValue_ = 0;
            break;
    }
}

void InstructionGenerator::visit(krl_ast::LiteraExpression& node){
    const auto& value = node.getValue();

    if(std::holds_alternative<int>(value)){
        currentValue_ = static_cast<double>(std::get<int>(value)); 
    }
    else if(std::holds_alternative<double>(value)){
        currentValue_ = std::get<double>(value);
    }
    else if(std::holds_alternative<bool>(value)){
        currentValue_ = std::get<bool>(value) ? 1.0 : 0.0;
    }
    else if(std::holds_alternative<std::string>(value)){
        currentValue_ = std::get<std::string>(value);
    }
}

void InstructionGenerator::visit(krl_ast::VariableExpression& node){
    const std::string& name = node.getName();

    if(hasVariable(node.getName())){
        auto varValue = getVariableValue(node.getName());

        if (std::holds_alternative<int>(varValue)) {
            currentValue_ = static_cast<double>(std::get<int>(varValue));
        } 
        else if (std::holds_alternative<double>(varValue)) {
            currentValue_ = std::get<double>(varValue);
        } 
        else if (std::holds_alternative<std::string>(varValue)) {
            std::cerr << "Cannot convert string to double" << "\n";
            currentValue_ = 0.0;
        }
        else if(std::holds_alternative<Position>(varValue))
            currentValue_ = static_cast<std::string>(node.getName());
            // currentValue_ = static_cast<std::string>(std::get<Position>(varValue).getPosition());
        }
    else{
        std::cerr<<"Undefinde variable "<<name<<std::endl;
            currentValue_= 0.0;
    }

}

void InstructionGenerator::visit(krl_ast::VariableDeclaration& node){
    std::string name = node.getName();
    krl_lexer::TokenType type = node.getDataType();

    TypeValue value = {type, 0.0};

    if(double baseVal = 0.0; node.getInitializer()){
        ValueType initValue = evaluateExpression(node.getInitializer());

        if(auto val = std::get_if<int>(&initValue))
            baseVal = static_cast<double>(*val);
        else if(auto val = std::get_if<double>(&initValue))
            baseVal = *val;

        switch(type){
            case krl_lexer::TokenType::INT:
            value.value = static_cast<int>(baseVal);
            break;
            case krl_lexer::TokenType::CHAR:
            value.value = std::to_string(baseVal);
            break;
            case krl_lexer::TokenType::BOOL:
            value.value = (baseVal != 0.0);
            break;
            case krl_lexer::TokenType::REAL:
            default:
            value.value = baseVal;
            break;
        }
    }

    declaredVariables_[name] = value;

    std::vector<std::pair<std::string, ValueType>> args;
    args.push_back({"type",static_cast<std::string>(krl_lexer::typeToStringMap.at(type))});
    args.push_back({"value",value.value});
    instruction_.push_back({"DECL_" + name, args});
}

void InstructionGenerator::visit(krl_ast::PositionDeclaration& node){
   executeDeclaration<krl_ast::PositionDeclaration&,Position>(node, krl_lexer::TokenType::POS, "POSITION");
}

void InstructionGenerator::visit(krl_ast::FrameDeclaration& node){
    executeDeclaration<krl_ast::FrameDeclaration&, Frame>(node, krl_lexer::TokenType::FRAME, "FRAME");
}

void InstructionGenerator::visit(krl_ast::AxisDeclaration& node){
    executeDeclaration<krl_ast::AxisDeclaration&, Axis>(node, krl_lexer::TokenType::AXIS, "AXIS");
}


ValueType InstructionGenerator::evaluateExpression(const std::shared_ptr<krl_ast::Expression>& expr)
{
    if(expr){
        expr->accept(*this);
        return currentValue_;
    }
    return 0.0;
}

InstructionSet::~InstructionSet(){}

void InstructionSet::addInstruction(const Instruction& instruction){
    instructions_.push_back(instruction);
}

void InstructionSet::clear(){
    instructions_.clear();
}

const std::vector<Instruction>& InstructionSet::getInstructions() const {
    return instructions_;
}

size_t InstructionSet::size() const{
    return instructions_.size();
}

const Instruction& InstructionSet::getInstruction(size_t index) const{
    return instructions_.at(index);
}
}
