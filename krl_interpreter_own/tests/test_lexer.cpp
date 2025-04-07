#include <iostream>
#include <cassert>
#include <string>
#include "../include/lexer/lexer.hpp"

void testLexer() {
    krl::Lexer lexer;
    
    // Test 1: Basit ifade
    {
        std::string code = "x = 10 + 20";
        auto tokens = lexer.tokenize(code);
        
        // Whitespace ve yorum satırlarını hariç tutarsak 6 token bekliyoruz
        // x, =, 10, +, 20, EOF
        assert(tokens.size() == 6);
        assert(tokens[0].getType() == krl::TokenType::IDENTIFIER);
        assert(tokens[1].getType() == krl::TokenType::ASSIGN);
        assert(tokens[2].getType() == krl::TokenType::INT_LITERAL);
        assert(tokens[3].getType() == krl::TokenType::PLUS);
        assert(tokens[4].getType() == krl::TokenType::INT_LITERAL);
        assert(tokens[5].getType() == krl::TokenType::EOF_TOKEN);
    }
    
    // Test 2: Anahtar kelimeler
    {
        std::string code = "IF x > 10 THEN\ny = 20\nENDIF";
        auto tokens = lexer.tokenize(code);
        
        // IF, x, >, 10, THEN, EOL, y, =, 20, EOL, ENDIF, EOF şeklinde 12 token bekliyoruz
        assert(tokens.size() == 12);
        assert(tokens[0].getType() == krl::TokenType::IF);
        assert(tokens[4].getType() == krl::TokenType::THEN);
        assert(tokens[10].getType() == krl::TokenType::ENDIF);
    }
    
    std::cout << "Tüm lexer testleri başarıyla geçti!\n";
}

int main() {
    testLexer();
    return 0;
}