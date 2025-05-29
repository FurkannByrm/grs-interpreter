#include "interpreter/instruction_set.hpp"
#include <iostream>


namespace krl_interpreter{


InstructionGenerator::InstructionGenerator(){}
InstructionGenerator::~InstructionGenerator(){}


std::vector<Instruction> InstructionGenerator::generateInstructions(const std::shared_ptr<krl_ast::Program>& program){
    instruction_.clear();
    variableValues_.clear();

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

    std::string command = node.getCommand();
    std::vector<std::pair<std::string, double>> args;
    for(const auto& arg : node.getArgs()){
        if(arg.second){
            double value = evaluateExpression(std::dynamic_pointer_cast<krl_ast::Expression>(arg.second));
            args.push_back({arg.first,value});
        }

    }

    instruction_.push_back({command,args});

}

void InstructionGenerator::visit(krl_ast::BinaryExpression& node){
    double left     = evaluateExpression(node.getLeft());
    double right   = evaluateExpression(node.getRight());

    switch(node.getOperator()){

        case krl_lexer::TokenType::PLUS:
            currentValue_ = left + right;
            break;
        case krl_lexer::TokenType::MINUS:
            currentValue_ = left - right;
            break;
        case krl_lexer::TokenType::MULTIPLY:
            currentValue_ = left * right;
            break;
            case krl_lexer::TokenType::DIVIDE: 
            if(right == 0){
                std::cerr<<"Error: Division by zero"<<std::endl;
                currentValue_ = 0;
            }
            else{
                currentValue_= 0;
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
    else if(std::holds_alternative<float>(value)){
        currentValue_ = static_cast<double>(std::get<float>(value));
    }
    else if(std::holds_alternative<bool>(value)){
        currentValue_ = std::get<bool>(value) ? 1.0 : 0.0;
    }
    else if(std::holds_alternative<std::string>(value)){
        currentValue_ = 0;
    }
}

void InstructionGenerator::visit(krl_ast::VariableExpression& node){
    const std::string& name = node.getName();

    if(variableValues_.find(name) != variableValues_.end()){
        currentValue_= variableValues_[name];
    }
    else{
        std::cerr<<"Undefinde variable"<<name<<std::endl;
        currentValue_= 0;
    }

}

void InstructionGenerator::visit(krl_ast::IfStatement& node){
    double condition = evaluateExpression(node.getCondition());

    if(condition != 0) { 
        node.getThenBranch()->accept(*this);
    }
    else if(node.getElseBranch()){
        node.getElseBranch()->accept(*this);
    }

}

double InstructionGenerator::evaluateExpression(const std::shared_ptr<krl_ast::Expression>& expr)
{
    if(expr){
        expr->accept(*this);
        return currentValue_;
    }
    return 0.0;
}

InstructionSet::InstructionSet(){}
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
