#include "../include/lexer/token.hpp" 
namespace grs_lexer {

Token::Token(TokenType type, const std::string& value, int line, int column)
    : type_(type), value_(value), line_(line), column_(column) {}


TokenType Token::getType() const{
    return type_;
}

std::string Token::getValue() const{
    return value_;
}

int Token::getLine() const{
    return line_;
}

int Token::getColumn() const{
    return column_;
}

std::string_view Token::typeToString() const{

    return typeToStringMap.at(type_);
}




}