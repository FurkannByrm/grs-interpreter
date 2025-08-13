#ifndef PARSER_HPP_
#define PARSER_HPP_

#include "../lexer/token.hpp"
#include "../ast/ast.hpp"

namespace grs_parser{
struct ParserError{
    std::string message;
    int line;
    int column;
};
class Parser{

    public:
    Parser();
    ~Parser();
    std::shared_ptr<grs_ast::FunctionBlock> parse(const std::vector<grs_lexer::Token>& tokens);
    bool hasErrors()const {return !errors_.empty();}
    const std::vector<ParserError>& getErrors()const {return errors_;}
    std::vector<std::pair<int,int>> getLineAndColumn()const{ return lineAndColumn_;}

    private:
    std::vector<grs_lexer::Token> tokens_;
    size_t current_;
    std::vector<ParserError> errors_;
    std::vector<std::pair<int,int>> lineAndColumn_;    
    
    bool isAtEnd() const;
    grs_lexer::Token peek() const;
    grs_lexer::Token previous() const;
    grs_lexer::Token advance();
    bool check(grs_lexer::TokenType type) const;
    bool match(std::initializer_list<grs_lexer::TokenType> types);
    void addError(const std::string& message);
    void eraseFirstPosition();

    //recursive descent ASTNodes
    std::shared_ptr<grs_ast::ASTNode> declaration();
    std::shared_ptr<grs_ast::ASTNode> functionDeclaration();
    std::shared_ptr<grs_ast::ASTNode> variableDeclaration();
    std::shared_ptr<grs_ast::ASTNode> block();
    std::shared_ptr<grs_ast::ASTNode> statement();
    std::shared_ptr<grs_ast::ASTNode> ifStatement();
    std::shared_ptr<grs_ast::ASTNode> forStatement();
    std::shared_ptr<grs_ast::ASTNode> repeatStatement();
    std::shared_ptr<grs_ast::ASTNode> returnStatement();
    std::shared_ptr<grs_ast::ASTNode> commandStatement();
    std::shared_ptr<grs_ast::ASTNode> expressionStatement();
    std::shared_ptr<grs_ast::ASTNode> motionCommand();
    std::shared_ptr<grs_ast::ASTNode> waitStatement();
    std::shared_ptr<grs_ast::ASTNode> positionDeclaration();
    std::shared_ptr<grs_ast::ASTNode> frameDeclaration();
    std::shared_ptr<grs_ast::ASTNode> axisDeclaration();
    
    //recursive descent Expression
    std::shared_ptr<grs_ast::Expression> expression();
    std::shared_ptr<grs_ast::Expression> assignment();
    std::shared_ptr<grs_ast::Expression> logicalOr();
    std::shared_ptr<grs_ast::Expression> logicalAnd();
    std::shared_ptr<grs_ast::Expression> equality();
    std::shared_ptr<grs_ast::Expression> comparison();
    std::shared_ptr<grs_ast::Expression> term();
    std::shared_ptr<grs_ast::Expression> factor();
    std::shared_ptr<grs_ast::Expression> unary();
    std::shared_ptr<grs_ast::Expression> primary();

    template<class DeclarationType>
    std::shared_ptr<grs_ast::ASTNode> parserDeclaration(const std::string& typeName){
                
        eraseFirstPosition();
        
        if(!check(grs_lexer::TokenType::IDENTIFIER)){
        addError("Expected " + typeName + " name");
        return nullptr;
    }
        std::string structName = advance().getValue();

        if(!match({grs_lexer::TokenType::ASSIGN})){
        addError("Expected '=' after " + typeName + " name");
        return nullptr;
    }
    
        if(!match({grs_lexer::TokenType::LBRACE})){
        addError("Expected '{' after " + typeName + " assignment");
        return nullptr;
    }
    
    std::vector<std::pair<std::string,std::shared_ptr<grs_ast::Expression>>> arguments;
    
    while (!check(grs_lexer::TokenType::RBRACE) && !isAtEnd()){
        if(!check(grs_lexer::TokenType::IDENTIFIER)){
            if constexpr (std::is_same_v<DeclarationType, grs_ast::AxisDeclaration>){
                addError("Expected axis name {A1, A2, A3, A4, A5, A6}");
            }
            else{
                addError("Expected axis name {X, Y, Z, A, B, C} ");
                return nullptr;
            }
        }
        
        std::string argName = advance().getValue();
        auto argValue = expression();
        arguments.emplace_back(argName, argValue);
    
        if(!match({grs_lexer::TokenType::COMMA}) && !check(grs_lexer::TokenType::RBRACE)){
            addError("Expected ',' or '}' in " + typeName + " arguments");
            return nullptr;
        }
    }
    
    if(!match({grs_lexer::TokenType::RBRACE})){
        addError("Expected '}' after " + typeName + "arguments");
        return nullptr;
    }

    return std::make_shared<DeclarationType>(structName, arguments, lineAndColumn_ );

    }



};


    
}


#endif //PARSER_HPP_