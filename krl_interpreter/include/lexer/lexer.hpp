#pragma once
#include <iostream>
#include <fstream>
#include <string>
#include <vector>
#include <regex>
#include <unordered_map>
#include <stdexcept>

#include "lexer/token.hpp"

namespace krl_lexer {
    

    class LexerError : public std::runtime_error {
    public:
        LexerError(const std::string& message, int line, int column) 
            : std::runtime_error(message), line_(line), column_(column) {}
        
        int getLine() const { return line_; }
        int getColumn() const { return column_; }
        
        std::string getFormattedMessage() const {
            return "Lexer Error at line " + std::to_string(line_) + 
                   ", column " + std::to_string(column_) + ": " + what();
        }
        
    private:
        int line_;
        int column_;
    };
    
    class Lexer {
        public:
        Lexer();
        std::vector<Token> tokenize(const std::string& code);
        void printTokens(const std::vector<Token>& tokens) const;
        
        bool hasErrors() const { return !errors_.empty(); }
        const std::vector<LexerError>& getErrors() const { return errors_; }
        void clearErrors() { errors_.clear(); }
        struct TokenPattern {
        std::regex pattern;
        TokenType type;
    };

private:
    
    std::vector<TokenPattern> patterns_;
    std::unordered_map<std::string_view, TokenType> keywords_;
    std::vector<LexerError> errors_; 
    
    void initTokenPatterns();
    void initKeywords();
    
    
    void addError(const std::string& message, int line, int column) {
        errors_.emplace_back(message, line, column);
    }
};


} // namespace krl