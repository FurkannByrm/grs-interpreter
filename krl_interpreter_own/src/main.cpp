#include <iostream>
#include <string>
#include "../include/lexer/lexer.hpp"

int main() {
    std::string code = 
    "DEF example()\n"
    "  ; This is a comment\n"
    "  x = 10 + 20 * (30 - 5)\n"
    "  IF x > 100 THEN\n"
    "    PRINT(\"x is greater than 100\")\n"
    "  ELSE\n"
    "    PRINT(\"x is less than or equal to 100\")\n"
    "  ENDIF\n"
    "END";
    
    krl::Lexer lexer;
    std::vector<krl::Token> tokens = lexer.tokenize(code);
    
    for (const auto& token : tokens) {
        std::cout << "Line: " << token.getLine() 
                  << ", Col: " << token.getColumn()
                  << ", Type: " << token.typeToString()
                  << ", Value: '" << token.getValue() << "'" << std::endl;
    }
    
    return 0;
}