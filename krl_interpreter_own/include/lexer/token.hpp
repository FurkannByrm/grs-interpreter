#pragma once
#include <string>

namespace krl {

enum class TokenType {
    // Anahtar kelimeler
    DEF,
    DEFFCT,
    ENDFCT,
    IF,
    THEN,
    ELSE,
    ENDIF,
    FOR,
    TO,
    ENDFOR,
    WHILE,
    ENDWHILE,
    
    // Operatörler
    PLUS,  // +
    MINUS, // -
    MUL,   // *
    DIV,   // /
    ASSIGN, // =
    EQ,    // ==
    NEQ,   // <>
    LT,    // <
    GT,    // >
    LE,    // <=
    GE,    // >=
    
    // Parantezler ve ayraçlar
    LPAREN,    // (
    RPAREN,    // )
    SEMICOLON, // ;
    COMMA,     // ,
    
    // Veri tipleri
    IDENTIFIER, // değişken ve fonksiyon isimleri
    INT_LITERAL,
    FLOAT_LITERAL,
    STRING_LITERAL,
    
    // Özel tokenler
    COMMENT,
    WHITESPACE,
    EOL,        // Satır sonu
    EOF_TOKEN,  // Dosya sonu
    
    UNKNOWN     // Bilinmeyen token
};

class Token {
public:
    Token(TokenType type, const std::string& value, int line, int column);
    
    TokenType getType() const;
    std::string getValue() const;
    int getLine() const;
    int getColumn() const;
    
    std::string typeToString() const; // Token tipini string'e dönüştürür (debug için)
    
private:
    TokenType type_;
    std::string value_;
    int line_;
    int column_;
};

} // namespace krl