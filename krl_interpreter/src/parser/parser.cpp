#include "parser/parser.hpp"
#include <iostream>
namespace krl_parser{

Parser::Parser() : current_{0} {}

Parser::~Parser(){}

//main parsing function, parsing whole declarations
std::shared_ptr<krl_ast::Program> Parser::parse(const std::vector<krl_lexer::Token>& tokens){
std::cout << "Parser started..." << std::endl;
tokens_ = tokens;
current_ = 0;
errors_.clear();

std::vector<std::shared_ptr<krl_ast::ASTNode>> statement;

while(!isAtEnd()){
    std::cout << "Token is being processed: " << current_ << " - " 
                  << static_cast<int>(peek().getType()) << " - " 
                  << peek().getValue() << std::endl;
    try{
        auto stmt = declaration();
        if(stmt){
            statement.push_back(stmt);
        }
    } catch(const std::exception& e){
        std::cerr << " Error during parsing: "<<e.what()<<std::endl;
        advance();
    }
}
std::cout << "Parsing finished." << std::endl;
return std::make_shared<krl_ast::Program>(statement);

}


//Core Infrastructure 


bool Parser::isAtEnd() const {
    return current_>= tokens_.size() || tokens_[current_].getType() == krl_lexer::TokenType::ENDOFFILE;
}

krl_lexer::Token Parser::peek() const {
    if(isAtEnd()){
        return krl_lexer::Token{krl_lexer::TokenType::ENDOFFILE, "", 0, 0};
    }
    return tokens_[current_];
}

krl_lexer::Token Parser::previous() const {
    if(current_ == 0){
        return krl_lexer::Token{krl_lexer::TokenType::ENDOFFILE, "", 0, 0};
    }
    return tokens_[current_-1];
}
krl_lexer::Token Parser::advance(){
    if(!isAtEnd()){
        current_++;
    std::cout << "Token advanced: " << (current_ - 1) << " -> " << current_ << std::endl;

    }
    return previous();
}

//Token Matching
bool Parser::check(krl_lexer::TokenType type) const {
    if(isAtEnd()) {
        return false;
    }
    return peek().getType() == type;
}

bool Parser::match(std::initializer_list<krl_lexer::TokenType> types){
    for(auto type : types){
        if(check(type)){
            std::cout << "Token is been matched: " << krl_lexer::typeToStringMap.at(type) << std::endl;

            advance();
            return true;
        }
    }
    return false;
}

void Parser::addError(const std::string& message){
    krl_lexer::Token token = peek();
    errors_.push_back({message, token.getLine(), token.getColumn()});
}

void Parser::eraseFirstPosition(){
    if(lineAndColumn_.size() > 0){
        lineAndColumn_.erase(lineAndColumn_.begin());
    }
    lineAndColumn_.emplace_back(std::make_pair(peek().getLine(),peek().getColumn()));
}
// Recursive descent ASTNodes

//top level parsing
std::shared_ptr<krl_ast::ASTNode> Parser::declaration(){
    //krl language variable and function definition
    if(match({krl_lexer::TokenType::DEF})){                
        return functionDeclaration();
    }
    if(match({krl_lexer::TokenType::DECL})){
        return variableDeclaration();
    }

    return statement();
}

std::shared_ptr<krl_ast::ASTNode> Parser::functionDeclaration(){
    std::string functionName;

    if(!check(krl_lexer::TokenType::IDENTIFIER)){
        addError("Expeceted function name after DEF");
        return nullptr;
    }
    functionName = advance().getValue();

        if(!match({krl_lexer::TokenType::LPAREN})){
            addError("Expected '(' after function name");
            return nullptr;
        }
        if(!match({krl_lexer::TokenType::RPAREN})){
            addError("Expected ')' after parameters");
            return nullptr;
        }

        auto body = block();

        if(!match({krl_lexer::TokenType::END})){
            addError({"Expected 'END' after function body"});
            return nullptr;
        }
        return body;
}

std::shared_ptr<krl_ast::ASTNode> Parser::variableDeclaration(){
    //type control
    krl_lexer::TokenType dataType;

    if(match({krl_lexer::TokenType::INT})){
        dataType = krl_lexer::TokenType::INT;
    }else if(match({krl_lexer::TokenType::REAL})){
        dataType = krl_lexer::TokenType::REAL;
    }else if(match({krl_lexer::TokenType::BOOL})){
        dataType = krl_lexer::TokenType::BOOL;
    }else if(match({krl_lexer::TokenType::CHAR})){
        dataType = krl_lexer::TokenType::CHAR;
    }else if(match({krl_lexer::TokenType::FRAME})){
        return frameDeclaration();
    }else if(match({krl_lexer::TokenType::POS})){
        return positionDeclaration();
    }else if(match({krl_lexer::TokenType::AXIS})){
        return axisDeclaration();
    }else{
        addError("Expected data type after DECL");
        return nullptr;
    }
    
    eraseFirstPosition();
    // if(lineAndColumn_.size() > 0){
    //     lineAndColumn_.erase(lineAndColumn_.begin());
    // }
    // lineAndColumn_.emplace_back(std::make_pair(peek().getLine(),peek().getColumn()));
    
    if(!check(krl_lexer::TokenType::IDENTIFIER)){
        addError("Expected variable name");
        return nullptr;
    }
    std::string name = advance().getValue();

    std::shared_ptr<krl_ast::Expression> initializer = nullptr;
    if(match({krl_lexer::TokenType::ASSIGN})){
        initializer = expression();
    }
    
    if(!match({krl_lexer::TokenType::ENDOFLINE})){
        addError("Expected end of line after variable declaration");
    }

    return std::make_shared<krl_ast::VariableDeclaration>(dataType, name, initializer, lineAndColumn_);
}

std::shared_ptr<krl_ast::ASTNode> Parser::frameDeclaration(){

    return parserDeclaration<krl_ast::FrameDeclaration>("FRAME");
}

std::shared_ptr<krl_ast::ASTNode> Parser::positionDeclaration(){
   
    return parserDeclaration<krl_ast::PositionDeclaration>("POSITION");
}


std::shared_ptr<krl_ast::ASTNode> Parser::axisDeclaration(){
   
   return parserDeclaration<krl_ast::AxisDeclaration>("AXIS");
}


std::shared_ptr<krl_ast::ASTNode> Parser::statement(){
    
    if(match({krl_lexer::TokenType::PTP, krl_lexer::TokenType::LIN, krl_lexer::TokenType::CIRC, krl_lexer::TokenType::SPLINE})){
        return motionCommand();
    }
    else if(match({krl_lexer::TokenType::IF})){
        return ifStatement();
    }
    else if(match({krl_lexer::TokenType::RETURN})){
        return returnStatement();
    }
    else if(match({krl_lexer::TokenType::WAIT})){
        return waitStatement();
    }
    else if(match({krl_lexer::TokenType::ENDOFLINE})){
        return nullptr;
    }
    
   return expressionStatement();
}


std::shared_ptr<krl_ast::ASTNode> Parser::motionCommand(){ 
    std::string motionCommandName = static_cast<std::string>(krl_lexer::typeToStringMap.at(previous().getType()));
    

    eraseFirstPosition();
    if(!check(krl_lexer::TokenType::IDENTIFIER)){
        addError("Expected position name after motion command");
        return nullptr;
    }

    std::string positionName = advance().getValue();
    std::vector<std::pair<std::string, std::shared_ptr<krl_ast::Expression>>> arguments;
    arguments.emplace_back("position", std::make_shared<krl_ast::VariableExpression>(positionName));
    return std::make_shared<krl_ast::Command>(motionCommandName, arguments, lineAndColumn_);

}

std::shared_ptr<krl_ast::ASTNode> Parser::waitStatement(){

    eraseFirstPosition();
    if(!match({krl_lexer::TokenType::LPAREN})){
        addError("Expected wait command after '(' ");
        return nullptr;
    }
    
    auto val = std::stoi(advance().getValue());

    if(!match({krl_lexer::TokenType::RPAREN})){
        addError("Expected literal time expression after ')'");
        return nullptr;
    }


    return std::make_shared<krl_ast::WaitStatement>(val, lineAndColumn_);

}

std::shared_ptr<krl_ast::ASTNode> Parser::ifStatement(){

    eraseFirstPosition();
    auto condition = expression();
  
    if(!match({krl_lexer::TokenType::THEN})){
    addError("Expected 'THEN' after 'IF' condition");
    return nullptr;
  }

  if(!match({krl_lexer::TokenType::ENDOFLINE}))
  {
    addError("Expected 'ENDOFLINE' after 'THEN'");
    return nullptr;
  }

  auto thenBrance = block();

  std::shared_ptr<krl_ast::ASTNode> elseBranch = nullptr;

  if(match({krl_lexer::TokenType::ELSE})){
    if(!match({krl_lexer::TokenType::ENDOFLINE}))
    {
        addError("Expected 'ENDOFLINE' after 'ELSE'");
        return nullptr;
    }
    elseBranch = block();
  }

  if(!match({krl_lexer::TokenType::ENDIF})){
    addError("Expected 'ENDIF' to close IF statement");
    return nullptr;
}

return std::make_shared<krl_ast::IfStatement>(condition, thenBrance, elseBranch, lineAndColumn_);

}

std::shared_ptr<krl_ast::ASTNode> Parser::returnStatement(){

    
return nullptr;
}

std::shared_ptr<krl_ast::ASTNode> Parser::expressionStatement(){
    auto expr = expression();
    return expr;
}


std::shared_ptr<krl_ast::ASTNode> Parser::block(){
    std::vector<std::shared_ptr<krl_ast::ASTNode>> statements;

    while(!check(krl_lexer::TokenType::ENDFOR) && 
          !check(krl_lexer::TokenType::ENDIF)  &&
          !check(krl_lexer::TokenType::ELSE)   &&
          !check(krl_lexer::TokenType::END)    &&
          !isAtEnd()) {

            auto stmt = declaration();
            if(stmt){
                statements.push_back(stmt);
            }
        }
    return std::make_shared<krl_ast::Program>(statements);
}




 //recursive descent Expression
 
std::shared_ptr<krl_ast::Expression> Parser::expression(){
    std::cout << "expression() called \n";

    return assignment();
}

std::shared_ptr<krl_ast::Expression> Parser::assignment(){
    auto expr = logicalOr();
    if(match({krl_lexer::TokenType::ASSIGN})){
        krl_lexer::Token equals = previous();
        auto value = assignment();
        if(auto varExpr = std::dynamic_pointer_cast<krl_ast::VariableExpression>(expr)){
            return std::make_shared<krl_ast::BinaryExpression>(krl_lexer::TokenType::ASSIGN,
                                                                                    std::move(expr), 
                                                                                    std::move(value));
        }
        
        addError("Invalid assignment target");
    }

    return expr;
}

std::shared_ptr<krl_ast::Expression> Parser::logicalOr(){
    auto expr = logicalAnd();

    while(match({krl_lexer::TokenType::OR})){
        krl_lexer::Token op = previous();
        auto right = logicalAnd();
        expr = std::make_shared<krl_ast::BinaryExpression>(op.getType(), std::move(expr), std::move(right));
    }
    
    return expr;
}

std::shared_ptr<krl_ast::Expression> Parser::logicalAnd(){
    auto expr = equality();

    while(match({krl_lexer::TokenType::AND})){
        krl_lexer::Token op = previous();
        auto right = equality();
        expr = std::make_shared<krl_ast::BinaryExpression>(op.getType(), std::move(expr), std::move(right));
    }

    return expr;
}


std::shared_ptr<krl_ast::Expression> Parser::equality(){
    auto expr = comparison();

    while (match({krl_lexer::TokenType::EQUAL, krl_lexer::TokenType::NOTEQUAL})){
        krl_lexer::Token op = previous();
        auto right = comparison();
        expr = std::make_shared<krl_ast::BinaryExpression>(op.getType(), std::move(expr),std::move(right));
    }
    
    return expr;
    
}

std::shared_ptr<krl_ast::Expression> Parser::comparison(){
    auto expr = term();

    while(match({krl_lexer::TokenType::LESS, krl_lexer::TokenType::GREATER,
                krl_lexer::TokenType::LESSEQ,krl_lexer::TokenType::GREATEREQ})){
                    krl_lexer::Token op = previous();
                    auto right = term();
                    expr = std::make_shared<krl_ast::BinaryExpression>(op.getType(), std::move(expr), std::move(right));
                }
                return expr;
}



std::shared_ptr<krl_ast::Expression> Parser::term(){
    auto expr = factor();
    while (match({krl_lexer::TokenType::PLUS, krl_lexer::TokenType::MINUS}))
    {
        krl_lexer::Token op = previous();
        auto right = factor();
        expr = std::make_shared<krl_ast::BinaryExpression>(op.getType(), std::move(expr), std::move(right));
    }
    return expr;
    
}
std::shared_ptr<krl_ast::Expression> Parser::factor(){
    auto expr = unary();

    while (match({krl_lexer::TokenType::MULTIPLY, krl_lexer::TokenType::DIVIDE}))
    {
        krl_lexer::Token op = previous();
        auto right = unary();
        expr = std::make_shared<krl_ast::BinaryExpression>(op.getType(), std::move(expr),std::move(right));
    }
    return expr;    
}


std::shared_ptr<krl_ast::Expression> Parser::unary(){
    if(match({krl_lexer::TokenType::MINUS, krl_lexer::TokenType::NOT})){
        krl_lexer::Token op = previous();
        auto right = unary();
        return std::make_shared<krl_ast::UnaryExpression>(op.getType(), std::move(right));
    }
    return primary();
}

std::shared_ptr<krl_ast::Expression> Parser::primary(){
    std::cout << "primary() called: " << peek().getValue()
       << " (Type: " << krl_lexer::typeToStringMap.at(peek().getType()) << ")" << std::endl;

    if(match({krl_lexer::TokenType::FALSE, krl_lexer::TokenType::TRUE}))
    {
        bool value = previous().getType() == krl_lexer::TokenType::TRUE;
        return std::make_shared<krl_ast::LiteraExpression>(value);
    }


   if(match({krl_lexer::TokenType::INTEGER})) {
        int value = std::stoi(previous().getValue());
        return std::make_shared<krl_ast::LiteraExpression>(value);
    }

     if (match({krl_lexer::TokenType::FLOAT}))
    {
        double value = std::stod(std::string(previous().getValue()));
        return std::make_shared<krl_ast::LiteraExpression>(value);
    }
    
    if (match({krl_lexer::TokenType::STRING}))
    {
        std::string value = previous().getValue();
        
        if(value.size() >= 2){
            value = value.substr(1, value.size()- 2);
        }
        return std::make_shared<krl_ast::LiteraExpression>(value);

    }
    
    if (match({krl_lexer::TokenType::IDENTIFIER}))
    {
        return std::make_shared<krl_ast::VariableExpression>(std::string(previous().getValue()));
    }

    if(match({krl_lexer::TokenType::LPAREN}))
    {
    auto expr = expression();
    if(!match({krl_lexer::TokenType::RPAREN}))
    {
        addError("Expected ')' after expression");
    }
    return expr;
    }
    addError("Expected expression");
    
    return nullptr;
}





}