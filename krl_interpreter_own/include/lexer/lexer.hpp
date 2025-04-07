#pragma once

#include <string>
#include <vector>
#include <regex>
#include <unordered_map>
#include "token.hpp"

namespace krl {

class Lexer {
public:
    Lexer();
    std::vector<Token> tokenize(const std::string& code);
    void printTokens(const std::vector<Token>& tokens) const;

private:
    struct TokenPattern {
        std::regex pattern;
        TokenType type;
    };
    
    std::vector<TokenPattern> patterns_;
    std::unordered_map<std::string, TokenType> keywords_;
    
    void initTokenPatterns();
    void initKeywords();
};

} // namespace krl