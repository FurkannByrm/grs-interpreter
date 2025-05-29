#ifndef AST_HPP_
#define AST_HPP_

#include <vector>
#include <memory>
#include <variant>
#include <unordered_map>
#include "../lexer/token.hpp"


namespace krl_ast{

enum class ASTNodeType{
    Program,
    Command,
    Expression,
    BinaryExpression,
    UnaryExpression,
    LiteralExpression,
    VariableExpression,
    IfStatement,
    ForStatement,
    SwitchStatement,
    CaseStatement,
    DefaultStatement,
    Assignment,
    MotionCommand,
    WaitCommand,
    DelayCommand,
    ReturnStatement,
    BlockStatement
};

class ASTVisitor;

//Element Interface

class ASTNode{
    public:
    virtual ~ASTNode() = default;
    virtual ASTNodeType getType()const = 0;
    virtual void accept(ASTVisitor& visitor) = 0;
};



class Program : public ASTNode{
    public: 
    Program(std::vector<std::shared_ptr<ASTNode>> statements);
    ASTNodeType getType()const override{ return ASTNodeType::Program;}
    void accept(ASTVisitor& visitor)override;
    const std::vector<std::shared_ptr<ASTNode>>& getStatements()const{return statements_;}
    private:
    std::vector<std::shared_ptr<ASTNode>> statements_;
};

class Command : public ASTNode{
    public:
    Command(const std::string& command, std::vector<std::pair<std::string, std::shared_ptr<ASTNode>>> args);
    ASTNodeType getType() const override{return ASTNodeType::Command;};
    void accept(ASTVisitor& visitor)override;
    const std::string getCommand() const{return command_;}
    const std::vector<std::pair<std::string,std::shared_ptr<ASTNode>>>& getArgs() const{ return args_;}

    private:
    std::string command_;
    std::vector<std::pair<std::string, std::shared_ptr<ASTNode>>> args_;

};

class Expression : public ASTNode{
    public: 
    virtual ~Expression() = default;
};

class BinaryExpression : public Expression{

    public:
    BinaryExpression(krl_lexer::TokenType op, std::shared_ptr<Expression> left, std::shared_ptr<Expression> right);
    ASTNodeType getType()const override {return ASTNodeType::BinaryExpression;}
    void accept(ASTVisitor& visitor)override;
    krl_lexer::TokenType getOperator()const{ return op_;}
    const std::shared_ptr<Expression>& getLeft()const{return left_;}
    const std::shared_ptr<Expression>& getRight()const{return right_;}

    private:
    krl_lexer::TokenType op_;
    std::shared_ptr<Expression> left_;
    std::shared_ptr<Expression> right_;
};

class UnaryExpression : public Expression{
    public:
    UnaryExpression(krl_lexer::TokenType op, std::shared_ptr<Expression> expr);
    ASTNodeType getType()const override{ return ASTNodeType::UnaryExpression; }
    void accept(ASTVisitor& visitor) override;
    krl_lexer::TokenType getOperator() const { return op_;}
    const std::shared_ptr<Expression>& getExpression() const { return expr_;}

    private:
    krl_lexer::TokenType op_;
    std::shared_ptr<Expression> expr_;
};

class LiteraExpression : public Expression{

    public:
    using ValueType = std::variant<int,float,bool,std::string>;

    explicit LiteraExpression(const ValueType& value);
    ASTNodeType getType()const override {return ASTNodeType::LiteralExpression;}
    void accept(ASTVisitor& visitor) override;
    const ValueType& getValue()const{ return value_;}

    private:
    ValueType value_;
};

class VariableExpression : public Expression{

    public:
    explicit VariableExpression(const std::string& name);
    ASTNodeType getType() const override{ return ASTNodeType::VariableExpression;}
    void accept(ASTVisitor& visitor)override;
    const std::string& getName() const  {return name_;}

    private:
    std::string name_;
};

class IfStatement : public ASTNode{

    public:
    IfStatement(std::shared_ptr<Expression> condition, std::shared_ptr<ASTNode> thenBranc, std::shared_ptr<ASTNode> elseBranch);
    void accept(ASTVisitor& visitor) override;
    const std::shared_ptr<Expression>& getCondition() const{return condition_;}
    const std::shared_ptr<ASTNode>& getThenBranch() const {return thenBranch_;}
    const std::shared_ptr<ASTNode>& getElseBranch() const {return elseBranch_;}

    private:
    std::shared_ptr<Expression> condition_;
    std::shared_ptr<ASTNode> thenBranch_;
    std::shared_ptr<ASTNode> elseBranch_;

};

class MotionCommand : public ASTNode{

    public:
    MotionCommand();
    void accept(ASTVisitor& visitor)override;
    private:

};



}




#endif //AST_HPP_