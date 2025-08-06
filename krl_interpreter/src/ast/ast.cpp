#include "ast/ast.hpp"
#include "ast/visitor.hpp"
namespace krl_ast {

    //Program
    Program::Program(std::vector<std::shared_ptr<ASTNode>> statements) 
    : statements_{std::move(statements)}{}

    void Program::accept(ASTVisitor& visitor){
        visitor.visit(*this);
    }

    //Command
    Command::Command(const std::string& command,std::vector<std::pair<std::string,std::shared_ptr<Expression>>> args, std::vector<std::pair<int,int>> lineAndColumn) 
    : command_{command},args_{std::move(args)}, lineAndColumn_{lineAndColumn} {}

    void Command::accept(ASTVisitor& visitor){
        visitor.visit(*this);
    }


    IfStatement::IfStatement(std::shared_ptr<Expression> condition, std::shared_ptr<ASTNode> thenBranch, std::shared_ptr<ASTNode> elseBranch,std::vector<std::pair<int,int>> lineAndColumn) 
    : condition_{std::move(condition)}, elseBranch_{std::move(elseBranch)}, thenBranch_{std::move(thenBranch)}, lineAndColumn_{lineAndColumn} {}
    void IfStatement::accept(ASTVisitor& visitor){
        visitor.visit(*this);
    }

    WaitStatement::WaitStatement(int& waitTime, std::vector<std::pair<int,int>> lineAndColumn) : waitTime_{waitTime}, lineAndColumn_{lineAndColumn} {}
    void WaitStatement::accept(ASTVisitor& visitor){
        visitor.visit(*this);
    }

    FrameDeclaration::FrameDeclaration(const std::string& name, const std::vector<std::pair<std::string,std::shared_ptr<Expression>>>& args, std::vector<std::pair<int,int>>& lineAndColumn)
    : name_(name), args_(args), lineAndColumn_{lineAndColumn} {}
    void FrameDeclaration::accept(ASTVisitor& visitor){
    visitor.visit(*this);    
    }

    PositionDeclaration::PositionDeclaration(const std::string& name, const std::vector<std::pair<std::string,std::shared_ptr<Expression>>>& args, std::vector<std::pair<int,int>>& lineAndColumn)
    : name_{name}, args_{args}, lineAndColumn_{lineAndColumn}{}
    void PositionDeclaration::accept(ASTVisitor& visitor){
        visitor.visit(*this);
    }

    AxisDeclaration::AxisDeclaration(const std::string& name, const std::vector<std::pair<std::string, std::shared_ptr<Expression>>>& args, std::vector<std::pair<int,int>>& lineAndColumn) 
    : name_{name}, args_{args}, lineAndColumn_{lineAndColumn} {}   
    void AxisDeclaration::accept(ASTVisitor& visitor){
        visitor.visit(*this);
    }


    //BinaryExpression
    BinaryExpression::BinaryExpression(krl_lexer::TokenType op, std::shared_ptr<Expression> left, std::shared_ptr<Expression> right) 
    : op_{op}, left_{std::move(left)}, right_{std::move(right)} {}

    void BinaryExpression::accept(ASTVisitor& visitor){
        visitor.visit(*this);
    }

    //UnaryExpression
    UnaryExpression::UnaryExpression(krl_lexer::TokenType op, std::shared_ptr<Expression> expr) 
    : op_{op}, expr_{std::move(expr)} {}
    
    void UnaryExpression::accept(ASTVisitor& visitor){
        visitor.visit(*this);
    }


    //literalExpression
    LiteraExpression::LiteraExpression(const ValueType& value) 
    : value_{value} {}
    
    void LiteraExpression::accept(ASTVisitor& visitor){
        visitor.visit(*this);
    }

    //VariableExpression 
    VariableExpression::VariableExpression(const std::string& name) 
    : name_{name} {}
    
    void VariableExpression::accept(ASTVisitor& visitor){
        visitor.visit(*this);
    }

    //VariableDeclaration
    VariableDeclaration::VariableDeclaration(krl_lexer::TokenType dataType, const std::string& name, std::shared_ptr<Expression> initializer,std::vector<std::pair<int,int>> lineAndColumn) 
    : dataType_{dataType}, name_{name}, initializer_{initializer}, lineAndColumn_{lineAndColumn} {}  
    void VariableDeclaration::accept(ASTVisitor& visitor){
        visitor.visit(*this);
    }

  




}