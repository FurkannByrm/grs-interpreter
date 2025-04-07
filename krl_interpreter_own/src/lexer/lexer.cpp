#include "../../include/lexer/lexer.hpp"
#include <iostream>

namespace krl {

Lexer::Lexer() {
    initKeywords();
    initTokenPatterns();
}

void Lexer::initKeywords() {
    keywords_["DEF"] = TokenType::DEF;
    keywords_["DEFFCT"] = TokenType::DEFFCT;
    keywords_["ENDFCT"] = TokenType::ENDFCT;
    keywords_["IF"] = TokenType::IF;
    keywords_["THEN"] = TokenType::THEN;
    keywords_["ELSE"] = TokenType::ELSE;
    keywords_["ENDIF"] = TokenType::ENDIF;
    keywords_["FOR"] = TokenType::FOR;
    keywords_["TO"] = TokenType::TO;
    keywords_["ENDFOR"] = TokenType::ENDFOR;
    keywords_["WHILE"] = TokenType::WHILE;
    keywords_["ENDWHILE"] = TokenType::ENDWHILE;
}

void Lexer::initTokenPatterns() {
    // Boşluk ve yorum satırları
    patterns_.push_back({std::regex("^[ \t]+"), TokenType::WHITESPACE});
    patterns_.push_back({std::regex("^;[^\n]*"), TokenType::COMMENT});
    
    // Satır sonu
    patterns_.push_back({std::regex("^\n"), TokenType::EOL});
    
    // Operatörler
    patterns_.push_back({std::regex("^\\+"), TokenType::PLUS});
    patterns_.push_back({std::regex("^-"), TokenType::MINUS});
    patterns_.push_back({std::regex("^\\*"), TokenType::MUL});
    patterns_.push_back({std::regex("^/"), TokenType::DIV});
    patterns_.push_back({std::regex("^=="), TokenType::EQ});
    patterns_.push_back({std::regex("^="), TokenType::ASSIGN});
    patterns_.push_back({std::regex("^<>"), TokenType::NEQ});
    patterns_.push_back({std::regex("^<="), TokenType::LE});
    patterns_.push_back({std::regex("^>="), TokenType::GE});
    patterns_.push_back({std::regex("^<"), TokenType::LT});
    patterns_.push_back({std::regex("^>"), TokenType::GT});
    
    // Parantezler ve ayraçlar
    patterns_.push_back({std::regex("^\\("), TokenType::LPAREN});
    patterns_.push_back({std::regex("^\\)"), TokenType::RPAREN});
    patterns_.push_back({std::regex("^;"), TokenType::SEMICOLON});
    patterns_.push_back({std::regex("^,"), TokenType::COMMA});
    
    // Literaller
    patterns_.push_back({std::regex("^[0-9]+\\.[0-9]+"), TokenType::FLOAT_LITERAL});
    patterns_.push_back({std::regex("^[0-9]+"), TokenType::INT_LITERAL});
    patterns_.push_back({std::regex("^\"[^\"]*\""), TokenType::STRING_LITERAL});
    
    // Tanımlayıcılar (değişken ve fonksiyon isimleri)
    patterns_.push_back({std::regex("^[a-zA-Z_][a-zA-Z0-9_]*"), TokenType::IDENTIFIER});
}

std::vector<Token> Lexer::tokenize(const std::string& code) {
    std::vector<Token> tokens;
    
    std::string remainingCode = code;
    int line = 1;
    int column = 1;
    
    while (!remainingCode.empty()) {
        bool matched = false;
        
        // Tüm regex pattern'lerini dene
        for (const auto& tokenPattern : patterns_) {
            std::smatch match;
            if (std::regex_search(remainingCode, match, tokenPattern.pattern, std::regex_constants::match_continuous)) {
                std::string matchedText = match.str();
                
                TokenType tokenType = tokenPattern.type;
                
                // Eğer tanımlayıcı bir anahtar kelime ise, token tipini güncelle
                if (tokenType == TokenType::IDENTIFIER) {
                    auto keywordIt = keywords_.find(matchedText);
                    if (keywordIt != keywords_.end()) {
                        tokenType = keywordIt->second;
                    }
                }
                
                // Whitespace ve yorum satırlarını atla (istenirse)
                if (tokenType != TokenType::WHITESPACE && tokenType != TokenType::COMMENT) {
                    tokens.push_back(Token(tokenType, matchedText, line, column));
                }
                
                // Satır sonu kontrolü
                if (tokenType == TokenType::EOL) {
                    line++;
                    column = 1;
                } else {
                    column += matchedText.length();
                }
                
                // Eşleşen metni kalan koddan çıkar
                remainingCode = remainingCode.substr(matchedText.length());
                matched = true;
                break;
            }
        }
        
        // Eğer hiçbir pattern eşleşmezse, bilinmeyen karakter
        if (!matched) {
            char unknownChar = remainingCode[0];
            tokens.push_back(Token(TokenType::UNKNOWN, std::string(1, unknownChar), line, column));
            remainingCode = remainingCode.substr(1);
            column++;
        }
    }
    
    // Dosya sonu tokeni ekle
    tokens.push_back(Token(TokenType::EOF_TOKEN, "", line, column));
    
    return tokens;
}

void Lexer::printTokens(const std::vector<Token>& tokens) const {
    for (const auto& token : tokens) {
        std::cout << "Token(Type: " << token.typeToString()
                  << ", Value: \"" << token.getValue()
                  << "\", Line: " << token.getLine()
                  << ", Column: " << token.getColumn() << ")\n";
    }
}

} // namespace krl