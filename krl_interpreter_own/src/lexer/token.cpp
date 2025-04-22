#include "../include/lexer/token.hpp" 

namespace krl_lexer {

Token::Token(TokenType type, const std::string_view& value, int line, int column)
    : type_(type), value_(value), line_(line), column_(column) {}


TokenType Token::getType() const{
    return type_;
}

std::string_view Token::getValue() const{
    return value_;
}

int Token::getLine() const{
    return line_;
}

int Token::getColumn() const{
    return column_;
}

std::string Token::typeToString() const{
    switch (type_) {
        case TokenType::DEF: return "DEF";
        case TokenType::END: return "END";
        case TokenType::DECL: return "DECL";
        case TokenType::INT: return "INT";
        case TokenType::REAL: return "REAL";
        case TokenType::BOOL: return "BOOL";
        case TokenType::CHAR: return "CHAR";
        case TokenType::STRUCT: return "STRUCT";
        case TokenType::IF: return "IF";
        case TokenType::THEN: return "THEN";
        case TokenType::ELSE: return "ELSE";
        case TokenType::ENDIF: return "ENDIF";
        case TokenType::FOR: return "FOR";
        case TokenType::TO: return "TO";
        case TokenType::STEP: return "STEP";
        case TokenType::ENDFOR: return "ENDFOR";
        case TokenType::WHILE: return "WHILE";
        case TokenType::ENDWHILE: return "ENDWHILE";
        case TokenType::REPEAT: return "REPEAT";
        case TokenType::UNTIL: return "UNTIL";
        case TokenType::SWITCH: return "SWITCH";
        case TokenType::CASE: return "CASE";
        case TokenType::DEFAULT: return "DEFAULT";
        case TokenType::ENDSWITCH: return "ENDSWITCH";
        case TokenType::GOTO: return "GOTO";
        case TokenType::HALT: return "HALT";
        case TokenType::RETURN: return "RETURN";
        case TokenType::PTP: return "PTP";
        case TokenType::LIN: return "LIN";
        case TokenType::CIRC: return "CIRC";
        case TokenType::SPL: return "SPL";
        case TokenType::SLIN: return "SLIN";
        case TokenType::SCIRC: return "SCIRC";
        case TokenType::WAIT: return "WAIT";
        case TokenType::DELAY: return "DELAY";
        case TokenType::IN: return "IN";
        case TokenType::OUT: return "OUT";
        case TokenType::FRAME: return "FRAME";
        case TokenType::AXIS: return "AXIS";
        case TokenType::E6AXIS: return "E6AXIS";
        case TokenType::E6POS: return "E6POS";
        case TokenType::LPAREN: return "LPAREN";
        case TokenType::RPAREN: return "RPAREN";
        case TokenType::COMMA: return "COMMA";
        case TokenType::SEMICOLON: return "SEMICOLON";
        case TokenType::AMPERSAND: return "AMPERSAND";
        case TokenType::SINGLEQUOTE: return "SINGLEQUOTE";
        case TokenType::IDENTIFIER: return "IDENTIFIER";
        case TokenType::STRING: return "STRING";
        case TokenType::INTEGER: return "INTEGER";
        case TokenType::BOOLEAN: return "BOOLEAN";
        case TokenType::FLOAT: return "FLOAT";
        case TokenType::ENDOFLINE: return "ENDOFLINE";
        case TokenType::ENDOFFILE: return "ENDOFFILE";
        case TokenType::INVALID: return "INVALID";
        case TokenType::NOT: return "NOT";
        case TokenType::AND: return "AND";
        case TokenType::OR: return "OR";
        case TokenType::ASSIGN: return "ASSIGN";
        case TokenType::PLUS: return "PLUS";
        case TokenType::MINUS: return "MINUS";
        case TokenType::MULTIPLY: return "MULTIPLY";
        case TokenType::DIVIDE: return "DIVIDE";
        case TokenType::EQUAL: return "EQUAL";
        case TokenType::NOTEQUAL: return "NOTEQUAL";
        case TokenType::LESS: return "LESS";
        case TokenType::GREATER: return "GREATER";
        case TokenType::GREATEREQ: return "GREATEREQ";  
        case TokenType::LESSEQ: return "LESSEQ";        

    }}
}