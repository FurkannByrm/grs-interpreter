#ifndef AST_HPP_
#define AST_HPP_

#include <vector>
#include <memory>
#include <variant>
#include <unordered_map>
#include "../common/utils.hpp"
#include "../lexer/token.hpp"


namespace grs_ast{


enum class ASTNodeType{
    Program,
    FunctionBlock,
    Expression,
    Command,
    BinaryExpression,
    UnaryExpression,
    LiteralExpression,
    VariableExpression,
    VariableDeclaration,
    PositionDeclaration,
    FunctionDeclaration,
    FrameDeclaration,
    AxisDeclaration,
    IfStatement,
    ForStatement,
    SwitchStatement,
    CaseStatement,
    DefaultStatement,
    Assignment,
    MotionCommand,
    WaitStatement,
    DelayCommand,
    ReturnStatement,
    BlockStatement
};

class ASTVisitor;

//Element Interface
// Base Abstract Class
class ASTNode{
    public:
    ASTNode() = default;
    explicit ASTNode(std::vector<std::pair<int,int>> lineAndColumn);
    virtual ~ASTNode() = default;
    virtual ASTNodeType getType()const = 0;
    virtual void accept(ASTVisitor& visitor) = 0;
    virtual const std::vector<std::pair<int,int>>& getLineColumn()const{return lineAndColumn_;}
    private:
    std::vector<std::pair<int,int>> lineAndColumn_;

};

class Expression : public ASTNode{
    public:
    virtual ~Expression() = default;
};


class FunctionBlock : public ASTNode{
    public:
    FunctionBlock(std::vector<std::shared_ptr<ASTNode>> statements);
    ASTNodeType getType()const override{ return ASTNodeType::Program;}
    void accept(ASTVisitor& visitor)override;
    const std::vector<std::shared_ptr<ASTNode>>& getStatements()const{return statements_;}
    private:
    std::vector<std::shared_ptr<ASTNode>> statements_;
};

class FunctionDeclaration : public ASTNode{

    public:
    FunctionDeclaration(const std::string& name);
    ASTNodeType getType()const override{ return ASTNodeType::FunctionDeclaration;}
    void accept(ASTVisitor& visitor)override;
    private:
    std::string name_;
};

class FrameDeclaration : public ASTNode{
    public:
    FrameDeclaration(const std::string& name, const std::vector<std::pair<std::string, std::shared_ptr<Expression>>>& args, std::vector<std::pair<int,int>>& lineAndColumn);
    ASTNodeType getType()const override{ return ASTNodeType::FrameDeclaration;}
    void accept(ASTVisitor& visitor)override;
    std::string getName()const{return name_;}
    std::vector<std::pair<std::string, std::shared_ptr<Expression>>> getArgs() const{ return args_;}

    private:
    std::string name_;
    std::vector<std::pair<std::string, std::shared_ptr<Expression>>> args_;

};

class PositionDeclaration : public ASTNode{
    public:
    PositionDeclaration(const std::string& name, const std::vector<std::pair<std::string, std::shared_ptr<Expression>>>& args, std::vector<std::pair<int,int>>& lineAndColumn);
    ASTNodeType getType()const override{ return ASTNodeType::PositionDeclaration;}
    void accept(ASTVisitor& visitor)override;
    const std::string getName()const{return name_;}
    const std::vector<std::pair<std::string, std::shared_ptr<Expression>>>& getArgs()const{ return args_;}

    private:
    std::string name_;
    std::vector<std::pair<std::string, std::shared_ptr<Expression>>> args_;

};

class AxisDeclaration : public ASTNode{
    public:
    AxisDeclaration(const std::string& name, const std::vector<std::pair<std::string, std::shared_ptr<Expression>>>& args, std::vector<std::pair<int,int>>& lineAndColumn);
    ASTNodeType getType()const override{return ASTNodeType::AxisDeclaration;}
    void accept(ASTVisitor& visitor)override;
    std::string getName()const{return name_;}
    std::vector<std::pair<std::string,std::shared_ptr<Expression>>> getArgs()const{return args_;}

    private:
    std::string name_;
    std::vector<std::pair<std::string, std::shared_ptr<Expression>>> args_;

};

class MotionCommand : public ASTNode{
    public:
    MotionCommand(const std::string& command, const std::string& name, std::vector<std::pair<std::string, std::shared_ptr<Expression>>> args, std::vector<std::pair<int,int>> lineAndColumn);
    ASTNodeType getType() const override{return ASTNodeType::Command;};
    void accept(ASTVisitor& visitor)override;
    const std::string getCommand() const{return command_;}
    const std::string getName() const{return name_;}
    const std::vector<std::pair<std::string,std::shared_ptr<Expression>>>& getArgs() const{ return args_;}
    private:
    std::string command_;
    std::string name_;
    std::vector<std::pair<std::string, std::shared_ptr<Expression>>> args_;



};
class ReturnStatement : public ASTNode{

    public:
    explicit ReturnStatement();
    ASTNodeType getType()const override {return ASTNodeType::ReturnStatement;}
    void accept(ASTVisitor& visitor)override;

};

class WaitStatement : public ASTNode{

    public:
    explicit WaitStatement(double& waitTime, std::vector<std::pair<int,int>> lineAndColumn);
    ASTNodeType getType()const override {return ASTNodeType::WaitStatement;}
    void accept(ASTVisitor& visitor)override;
    double waitTime_;   

};


class BinaryExpression : public Expression{
    
    public:
    BinaryExpression(grs_lexer::TokenType op, std::shared_ptr<Expression> left, std::shared_ptr<Expression> right);
    ASTNodeType getType()const override {return ASTNodeType::BinaryExpression;}
    void accept(ASTVisitor& visitor)override;
    grs_lexer::TokenType getOperator()const{ return op_;}
    const std::shared_ptr<Expression>& getLeft()const{return left_;}
    const std::shared_ptr<Expression>& getRight()const{return right_;}
    
    private:
    grs_lexer::TokenType op_;
    std::shared_ptr<Expression> left_;
    std::shared_ptr<Expression> right_;
};

class UnaryExpression : public Expression{
    public:
    UnaryExpression(grs_lexer::TokenType op, std::shared_ptr<Expression> expr);
    ASTNodeType getType()const override{ return ASTNodeType::UnaryExpression; }
    void accept(ASTVisitor& visitor) override;
    grs_lexer::TokenType getOperator() const { return op_;}
    const std::shared_ptr<Expression>& getExpression() const { return expr_;}
    
    private:
    grs_lexer::TokenType op_;
    std::shared_ptr<Expression> expr_;
};

class LiteraExpression : public Expression{
    
    public:

    explicit LiteraExpression(const common::ValueType& value);
    ASTNodeType getType()const override {return ASTNodeType::LiteralExpression;}
    void accept(ASTVisitor& visitor) override;
    const common::ValueType& getValue()const{ return value_;}
    
    private:
    common::ValueType value_;
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


class VariableDeclaration : public ASTNode {
    public:
    VariableDeclaration(grs_lexer::TokenType dataType, const std::string& name, std::shared_ptr<Expression> initializer,std::vector<std::pair<int,int>> lineAndColumn);
    ASTNodeType getType()const override{ return ASTNodeType::VariableDeclaration;}
    void accept(ASTVisitor& visitor) override;
    grs_lexer::TokenType getDataType() const{ return dataType_;}
    const std::string& getName() const { return name_; }
    const std::shared_ptr<Expression>& getInitializer()const{ return initializer_;}

    
    private: 
    grs_lexer::TokenType dataType_;
    std::string name_;
    std::shared_ptr<Expression> initializer_;

    
};



class IfStatement : public ASTNode{
    
    public:
    IfStatement(std::shared_ptr<Expression> condition, std::shared_ptr<ASTNode> thenBranc, std::shared_ptr<ASTNode> elseBranch, std::vector<std::pair<int,int>> lineAndColum);
    void accept(ASTVisitor& visitor) override;
    ASTNodeType getType()const override{return ASTNodeType::IfStatement;}
    const std::shared_ptr<Expression>& getCondition() const{return condition_;}
    const std::shared_ptr<ASTNode>& getThenBranch() const {return thenBranch_;}
    const std::shared_ptr<ASTNode>& getElseBranch() const {return elseBranch_;}
    private:
    std::shared_ptr<Expression> condition_;
    std::shared_ptr<ASTNode> thenBranch_;
    std::shared_ptr<ASTNode> elseBranch_;

};




}




#endif //AST_HPP_