#include "interpreter/instruction_generator.hpp"
#include <iostream>


namespace grs_interpreter{


InstructionGenerator::InstructionGenerator(){}
InstructionGenerator::~InstructionGenerator(){}


std::vector<Instruction> InstructionGenerator::generateInstructions(const std::shared_ptr<grs_ast::FunctionBlock>& program){
    instruction_.clear();
    if(program){
        program->accept(*this);
    }
    return instruction_;
}

void InstructionGenerator::visit(grs_ast::FunctionBlock& node){
    for(const auto& statement : node.getStatements()){
        if (statement)
        {
            statement->accept(*this);
        }
        
    }
}


void InstructionGenerator::visit(grs_ast::MotionCommand& node){
    Instruction instruction;
    instruction.command = node.getCommand();
    instruction.commandLocationInfo = node.getLineColumn();
    for (const auto& arg : node.getArgs()) {
        instruction.args.emplace_back(arg.first, node.getName());
    }
    instruction.args.emplace_back("Position Information",getVariableValue(node.getName()));
    instruction_.emplace_back(instruction);
}

void InstructionGenerator::visit(grs_ast::PositionDeclaration& node){
   executeDeclaration<grs_ast::PositionDeclaration&,common::Position>(node, grs_lexer::TokenType::POS, "Position");
}

void InstructionGenerator::visit(grs_ast::FrameDeclaration& node){
    executeDeclaration<grs_ast::FrameDeclaration&, common::Frame>(node, grs_lexer::TokenType::FRAME, "Frame");
}

void InstructionGenerator::visit(grs_ast::AxisDeclaration& node){
    executeDeclaration<grs_ast::AxisDeclaration&, common::Axis>(node, grs_lexer::TokenType::AXIS, "Axis");
}

void InstructionGenerator::visit(grs_ast::ExecutePosAndAxisExpression& node){
    
    auto assigmentValue = evaluateExpression(node.getExpr());    
    auto val  = std::get<double>(assigmentValue);
    assignPosAndAxisExpression(node.getName(),node.getArg(),val);

}   



void InstructionGenerator::visit(grs_ast::BinaryExpression& node){
    auto leftExpr =  node.getLeft();
    auto rightExpr = node.getRight();

    if(double baseVal = 0.0; node.getOperator() == grs_lexer::TokenType::ASSIGN){
        auto varExpr = std::reinterpret_pointer_cast<grs_ast::VariableExpression>(leftExpr);
        if(!varExpr){
            std::cerr<<"assignment left side must be a variable \n";
            return;
        }
        std::string varName = varExpr->getName();

        if(!hasVariable(varName)){
            std::cerr<<"Undefined variable: "<< varName<<std::endl;
            return;
        }


        common::ValueType rightValue = evaluateExpression(rightExpr);

        //preparation before type convertions 
        if (std::holds_alternative<double>(rightValue)) {
            baseVal = std::get<double>(rightValue);
        } else if (std::holds_alternative<int>(rightValue)) {
            baseVal = static_cast<double>(std::get<int>(rightValue));
        }else if (std::holds_alternative<bool>(rightValue)) {
            baseVal = std::get<bool>(rightValue) ? 1.0 : 0.0;
        }
        
        //type convertions setting
        grs_lexer::TokenType targetType = declaredVariables_[varName].type;
        switch (targetType) {
            case grs_lexer::TokenType::INT:
                setVariableValue(varName, static_cast<int>(baseVal));
                currentValue_ = static_cast<double>(baseVal);
                break;
            case grs_lexer::TokenType::CHAR:
                setVariableValue(varName,std::to_string(static_cast<int>(baseVal)));
                currentValue_ = baseVal; 
                break;
            case grs_lexer::TokenType::BOOL:
                setVariableValue(varName, (baseVal != 0.0));
                currentValue_ = (baseVal != 0.0) ? 1.0 : 0.0;
                break;
            case grs_lexer::TokenType::REAL:
            default:
                setVariableValue(varName, baseVal);
                currentValue_ = baseVal;
                break;
        }

        std::vector<std::pair<std::string, common::ValueType>> args;
        args.push_back({"variable", varName});
        args.push_back({"value", getVariableValue(varName)});
        // args.push_back({"type", static_cast<double>(static_cast<int>(targetType))});
        instruction_.push_back({"ASSIGN_" + varName, args});
        return;
    }

    

    common::ValueType leftValue     = evaluateExpression(leftExpr);
    common::ValueType rightValue   = evaluateExpression(rightExpr);

    double leftVal = 0.0, rightVal = 0.0;

    // Left value conversion
    if (std::holds_alternative<double>(leftValue)) {
        leftVal = std::get<double>(leftValue);
    } else if (std::holds_alternative<int>(leftValue)) {
        leftVal = static_cast<double>(std::get<int>(leftValue));
    } else if (std::holds_alternative<bool>(leftValue)) {
        leftVal = std::get<bool>(leftValue) ? 1.0 : 0.0;
    } else {
        std::cerr << "Cannot convert left operand to numeric value" << std::endl;
        currentValue_ = 0.0;
        return;
    }
    
    // Right value conversion
    if (std::holds_alternative<double>(rightValue)) {
        rightVal = std::get<double>(rightValue);
    } else if (std::holds_alternative<int>(rightValue)) {
        rightVal = static_cast<double>(std::get<int>(rightValue));
    } else if (std::holds_alternative<bool>(rightValue)) {
        rightVal = std::get<bool>(rightValue) ? 1.0 : 0.0;
    } else {
        std::cerr << "Cannot convert right operand to numeric value" << std::endl;
        currentValue_ = 0.0;
        return;
    }

    // operating as operator
    switch(node.getOperator()){
        // Arithmetic operators
        case grs_lexer::TokenType::PLUS:
            currentValue_ = leftVal + rightVal;
            break;
            
        case grs_lexer::TokenType::MINUS:
            currentValue_ = leftVal - rightVal;
            break;
            
        case grs_lexer::TokenType::MULTIPLY:
            currentValue_ = leftVal * rightVal;
            break;
            
        case grs_lexer::TokenType::DIVIDE:
            if(rightVal == 0.0){
                std::cerr << "Error: Division by zero" << std::endl;
                currentValue_ = 0.0;
            } else {
                currentValue_ = leftVal / rightVal;
            }
            break;
            
        // Comparison operators for IF statement
        case grs_lexer::TokenType::LESS:
            currentValue_ = (leftVal < rightVal) ? 1.0 : 0.0;
            break;
            
        case grs_lexer::TokenType::LESSEQ:
            currentValue_ = (leftVal <= rightVal) ? 1.0 : 0.0;
            break;
            
        case grs_lexer::TokenType::GREATER:
            currentValue_ = (leftVal > rightVal) ? 1.0 : 0.0;
            break;
            
        case grs_lexer::TokenType::GREATEREQ:
            currentValue_ = (leftVal >= rightVal) ? 1.0 : 0.0;
            break;
            
        case grs_lexer::TokenType::EQUAL:
            currentValue_ = (leftVal == rightVal) ? 1.0 : 0.0;
            break;
            
        case grs_lexer::TokenType::NOTEQUAL:
            currentValue_ = (leftVal != rightVal) ? 1.0 : 0.0;
            break;
            
        // Logical operators
        case grs_lexer::TokenType::AND:
            currentValue_ = ((leftVal != 0.0) && (rightVal != 0.0)) ? 1.0 : 0.0;
            break;
            
        case grs_lexer::TokenType::OR:
            currentValue_ = ((leftVal != 0.0) || (rightVal != 0.0)) ? 1.0 : 0.0;
            break;
            
        default:
            std::cerr << "Unsupported binary operator: " << static_cast<int>(node.getOperator()) << std::endl;
            currentValue_ = 0.0;
            break;
    }

}

void InstructionGenerator::visit(grs_ast::LiteraExpression& node){
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

void InstructionGenerator::visit(grs_ast::UnaryExpression& node){
    common::ValueType value = evaluateExpression(node.getExpression());

     if(std::holds_alternative<int>(value)){
        currentValue_ = !std::get<int>(value); 
    }
    else if(std::holds_alternative<double>(value)){
        currentValue_ = !std::get<double>(value);
    }
    else if(std::holds_alternative<bool>(value)){
        currentValue_ = !std::get<bool>(value);
    }
}


void InstructionGenerator::visit(grs_ast::VariableDeclaration& node){
   
    std::string name = node.getName();
    grs_lexer::TokenType type = node.getDataType();

    VariableInfo value = {type, 0.0};

    if(double baseVal = 0.0; node.getInitializer()){
        common::ValueType initValue = evaluateExpression(node.getInitializer());

        if(auto val = std::get_if<int>(&initValue))
            baseVal = static_cast<double>(*val);
        else if(auto val = std::get_if<double>(&initValue))
            baseVal = *val;

        switch(type){
            case grs_lexer::TokenType::INT:
            value.value = static_cast<int>(baseVal);
            break;
            case grs_lexer::TokenType::CHAR:
            value.value = std::to_string(baseVal);
            break;
            case grs_lexer::TokenType::BOOL:
            value.value = (baseVal != 0.0);
            break;
            case grs_lexer::TokenType::REAL:
            default:
            value.value = baseVal;
            break;
        }
    }
    

    declaredVariables_[name] = value;
    std::vector<std::pair<std::string, common::ValueType>> args; 
    args.push_back({"type",static_cast<std::string>(grs_lexer::typeToStringMap.at(type))});
    args.push_back({"value",value.value});
    instruction_.push_back({"DECL_" + name, args, node.getLineColumn()});
}

void InstructionGenerator::visit(grs_ast::VariableExpression& node){
    const std::string& name = node.getName();

    if(hasVariable(name)){
        auto varValue = getVariableValue(name);

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
        // else if(std::holds_alternative<TypeValue>(varValue)){
            
        // }
    }
    else{
        std::cerr<<"Undefined variable "<<name<<std::endl;
            currentValue_= 0.0;
    }

}



void InstructionGenerator::visit(grs_ast::IfStatement& node){
    auto conditionValue = evaluateExpression(node.getCondition());

    bool conditionResult = false;
    if (std::holds_alternative<double>(conditionValue)) {
        conditionResult = (std::get<double>(conditionValue) != 0.0);
    } else if (std::holds_alternative<int>(conditionValue)) {
        conditionResult = (std::get<int>(conditionValue) != 0);
    } else if (std::holds_alternative<bool>(conditionValue)) {
        conditionResult = std::get<bool>(conditionValue);
    }
  
    Instruction ifstartInst;
    ifstartInst.command = "IF_START";
    ifstartInst.commandLocationInfo = node.getLineColumn();
    ifstartInst.args.emplace_back("condition", conditionResult);
    instruction_.push_back(ifstartInst);

    if(conditionResult && node.getThenBranch()){
        Instruction thenInst;
        thenInst.command = "THEN_BLOCK";
        instruction_.push_back(thenInst);
        
        node.getThenBranch()->accept(*this);
    }

    else if(!conditionResult && node.getElseBranch()){
        Instruction elseInst;
        elseInst.command = "ELSE_BLOCK";
        elseInst.commandLocationInfo = node.getLineColumn();
        instruction_.push_back(elseInst);

        node.getElseBranch()->accept(*this);
    }

    Instruction ifEndInst;
    ifEndInst.command = "IF_END";
    instruction_.push_back(ifEndInst);
}
void InstructionGenerator::visit(grs_ast::WaitStatement& node){

    Instruction instruction;
    double wtime = node.waitTime_;
    instruction.command = "WAIT";
    instruction.commandLocationInfo = node.getLineColumn();
    instruction.args.emplace_back("duration_time",wtime);
    instruction_.push_back(instruction);
    
}

void InstructionGenerator::visit(grs_ast::FunctionDeclaration& node){

    

}

common::ValueType InstructionGenerator::evaluateExpression(const std::shared_ptr<grs_ast::Expression>& expr)
{
    if(expr){
        expr->accept(*this);
        return currentValue_;
    }
    return 0.0;
}

}
