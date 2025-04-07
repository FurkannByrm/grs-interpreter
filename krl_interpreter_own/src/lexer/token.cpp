#include "../../include/lexer/token.hpp"
#include <unordered_map>

namespace krl {

Token::Token(TokenType type, const std::string& value, int line, int column)
    : type_(type), value_(value), line_(line), column_(column) {}

TokenType Token::getType() const {
    return type_;
}

std::string Token::getValue() const {
    return value_;
}

int Token::getLine() const {
    return line_;
}

int Token::getColumn() const {
    return column_;
}

std::string Token::typeToString() const {
    static const std::unordered_map<TokenType, std::string> tokenTypeStrings = {
        {TokenType::DEF, "DEF"},
        {TokenType::DEFFCT, "DEFFCT"},
        {TokenType::ENDFCT, "ENDFCT"},
        {TokenType::IF, "IF"},
        {TokenType::THEN, "THEN"},
        {TokenType::ELSE, "ELSE"},
        {TokenType::ENDIF, "ENDIF"},
        {TokenType::FOR, "FOR"},
        {TokenType::TO, "TO"},
        {TokenType::ENDFOR, "ENDFOR"},
        {TokenType::WHILE, "WHILE"},
        {TokenType::ENDWHILE, "ENDWHILE"},
        
        {TokenType::PLUS, "PLUS"},
        {TokenType::MINUS, "MINUS"},
        {TokenType::MUL, "MUL"},
        {TokenType::DIV, "DIV"},
        {TokenType::ASSIGN, "ASSIGN"},
        {TokenType::EQ, "EQ"},
        {TokenType::NEQ, "NEQ"},
        {TokenType::LT, "LT"},
        {TokenType::GT, "GT"},
        {TokenType::LE, "LE"},
        {TokenType::GE, "GE"},
        
        {TokenType::LPAREN, "LPAREN"},
        {TokenType::RPAREN, "RPAREN"},
        {TokenType::SEMICOLON, "SEMICOLON"},
        {TokenType::COMMA, "COMMA"},
        
        {TokenType::IDENTIFIER, "IDENTIFIER"},
        {TokenType::INT_LITERAL, "INT_LITERAL"},
        {TokenType::FLOAT_LITERAL, "FLOAT_LITERAL"},
        {TokenType::STRING_LITERAL, "STRING_LITERAL"},
        
        {TokenType::COMMENT, "COMMENT"},
        {TokenType::WHITESPACE, "WHITESPACE"},
        {TokenType::EOL, "EOL"},
        {TokenType::EOF_TOKEN, "EOF"},
        
        {TokenType::UNKNOWN, "UNKNOWN"}
    };
    
    auto it = tokenTypeStrings.find(type_);
    if (it != tokenTypeStrings.end()) {
        return it->second;
    }
    return "UNDEFINED";
}

} // namespace krl