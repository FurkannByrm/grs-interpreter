#include "../include/lexer/lexer.hpp"
namespace grs_lexer {

    Lexer::Lexer(){
        initTokenPatterns();
        initKeywords();
    }


    void Lexer::initKeywords() {
        keywords_["DEF"] = TokenType::DEF;
        keywords_["END"] = TokenType::END;   
        keywords_["DECL"] = TokenType::DECL;
        keywords_["INT"] = TokenType::INT;
        keywords_["REAL"] = TokenType::REAL;
        keywords_["BOOL"] = TokenType::BOOL;
        keywords_["CHAR"] = TokenType::CHAR;
        keywords_["IF"] = TokenType::IF;
        keywords_["THEN"] = TokenType::THEN;
        keywords_["ELSE"] = TokenType::ELSE;
        keywords_["ENDIF"] = TokenType::ENDIF;
        keywords_["FOR"] = TokenType::FOR;
        keywords_["TO"] = TokenType::TO;
        keywords_["STEP"] = TokenType::STEP;
        keywords_["ENDFOR"] = TokenType::ENDFOR;
        keywords_["WHILE"] = TokenType::WHILE;
        keywords_["ENDWHILE"] = TokenType::ENDWHILE;
        keywords_["REPEAT"] = TokenType::REPEAT;
        keywords_["UNTIL"] = TokenType::UNTIL;
        keywords_["SWITCH"] = TokenType::SWITCH;
        keywords_["CASE"] = TokenType::CASE;
        keywords_["DEFAULT"] = TokenType::DEFAULT;
        keywords_["ENDSWITCH"] = TokenType::ENDSWITCH;
        keywords_["GOTO"] = TokenType::GOTO;
        keywords_["HALT"] = TokenType::HALT;
        keywords_["RETURN"] = TokenType::RETURN;
    
        // Motion commands
        keywords_["PTP"] = TokenType::PTP;
        keywords_["LIN"] = TokenType::LIN;
        keywords_["CIRC"] = TokenType::CIRC;
        keywords_["SPLINE"] = TokenType::SPLINE;
     
        keywords_["PTP_REL"] = TokenType::PTP_REL;
        keywords_["LIN_REL"] = TokenType::LIN_REL;
        keywords_["CIRC_REL"] = TokenType::CIRC_REL;
        keywords_["SPLINE_REL"] = TokenType::SPLINE_REL;
    
        // System functions
        keywords_["WAIT"] = TokenType::WAIT;
        keywords_["DELAY"] = TokenType::DELAY;
    
        // Data types
        keywords_["FRAME"] = TokenType::FRAME;
        keywords_["AXIS"] = TokenType::AXIS;
        keywords_["POS"] = TokenType::POS;
    
        // Boolean and constant literals
        keywords_["TRUE"] = TokenType::TRUE;
        keywords_["FALSE"] = TokenType::FALSE;
        keywords_["PI"]    = TokenType::PI;
    }
    
    void Lexer::initTokenPatterns() {
        patterns_.push_back({std::regex(R"(\$IN\[[0-9]+\])"), TokenType::IN});
        patterns_.push_back({std::regex(R"(\$OUT\[[0-9]+\])"), TokenType::OUT});
        patterns_.push_back({std::regex(R"(:=|:|=)"), TokenType::ASSIGN});
        patterns_.push_back({std::regex(R"(==)"), TokenType::EQUAL});
        patterns_.push_back({std::regex(R"(<>)"), TokenType::NOTEQUAL});
        patterns_.push_back({std::regex(R"(<=)"), TokenType::LESSEQ});
        patterns_.push_back({std::regex(R"(>=)"), TokenType::GREATEREQ});
        patterns_.push_back({std::regex(R"(\+)"), TokenType::PLUS});
        patterns_.push_back({std::regex(R"(-)"), TokenType::MINUS});
        patterns_.push_back({std::regex(R"(\*)"), TokenType::MULTIPLY});
        patterns_.push_back({std::regex(R"(/)"), TokenType::DIVIDE});
        patterns_.push_back({std::regex(R"(<)"), TokenType::LESS});
        patterns_.push_back({std::regex(R"(>)"), TokenType::GREATER});
        patterns_.push_back({std::regex(R"(\bAND\b)"), TokenType::AND});
        patterns_.push_back({std::regex(R"(\bOR\b)"), TokenType::OR});
        patterns_.push_back({std::regex(R"(\bNOT\b)"), TokenType::NOT});
        // float before int
        patterns_.push_back({std::regex(R"([0-9]+\.[0-9]+([eE][+-]?[0-9]+)?)"), TokenType::FLOAT});
        patterns_.push_back({std::regex(R"([0-9]+)"), TokenType::INTEGER});
        patterns_.push_back({std::regex(R"("([^"\\]|\\.)*")"), TokenType::STRING});

        patterns_.push_back({std::regex(R"(&)"), TokenType::AMPERSAND});
        patterns_.push_back({std::regex(R"(\()"), TokenType::LPAREN});
        patterns_.push_back({std::regex(R"(\))"), TokenType::RPAREN});
        patterns_.push_back({std::regex(R"(\{)"), TokenType::LBRACE});
        patterns_.push_back({std::regex(R"(\})"), TokenType::RBRACE});
        patterns_.push_back({std::regex(R"(,)"), TokenType::COMMA});
        patterns_.push_back({std::regex(R"(;)"), TokenType::SEMICOLON});
        patterns_.push_back({std::regex(R"(')"), TokenType::SINGLEQUOTE});
        patterns_.push_back({std::regex(R"(\r\n|\n|\r)"), TokenType::ENDOFLINE});
        
        patterns_.push_back({std::regex(R"([A-Za-z_][A-Za-z0-9_]*)"), TokenType::IDENTIFIER});
    }

    std::vector<Token> Lexer::tokenize(const std::string& code) {
        std::vector<Token> tokens;
        std::istringstream stream(code);
        std::string line;
        int lineNumber = 1;
        
        std::vector<std::pair<TokenType, std::regex>> compiled_patterns;
        for (const auto& pattern : patterns_) {
            compiled_patterns.emplace_back(pattern.type, pattern.pattern);
        }
        
        while (std::getline(stream, line)) {
            int columnNumber = 1;
            size_t pos = 0;
            const size_t line_length = line.size();
            
            if (line.empty()) {
                tokens.emplace_back(TokenType::ENDOFLINE, "\n", lineNumber, columnNumber);
                lineNumber++;
                continue;
            }
            
            while (pos < line_length) {
                // Skip whitespace 
                while (pos < line_length && isspace(line[pos])) {
                    columnNumber++;
                    pos++;
                }
                if (pos >= line_length) break;
                
                bool found = false;
                std::string_view remaining(line.data() + pos, line_length - pos);
                
                // Check patterns in order of priority
                for (const auto& [type, regex] : compiled_patterns) {
                    std::cmatch match;
                    const char* start = line.c_str() + pos;
                    
                    if (std::regex_search(start, start + remaining.length(), 
                                         match, regex, 
                                         std::regex_constants::match_continuous)) {
                        std::string value = match.str();
                        TokenType actual_type = type;
                        
                        if (type == TokenType::IDENTIFIER) {
                            auto it = keywords_.find(value);
                            if (it != keywords_.end()) {
                                actual_type = it->second;
                            }
                        }
                        
                        tokens.emplace_back(actual_type, value, lineNumber, columnNumber);
                        pos += match.length();
                        columnNumber += match.length();
                        found = true;

                        // If SEMICOLON IS FOUND SKIP THE REST OF THE LINE
                        if (actual_type == TokenType::SEMICOLON) {
                            pos = line_length; // Move to the end of the line
                        }
                        break;
                    }
                }
                
                if (!found) {
                    size_t invalid_length = 1;
                    while (pos + invalid_length < line_length && 
                           !isspace(line[pos + invalid_length])) {
                        invalid_length++;
                    }
                    
                    tokens.emplace_back(TokenType::INVALID, 
                                      line.substr(pos, invalid_length), 
                                      lineNumber, columnNumber);
                    pos += invalid_length;
                    columnNumber += invalid_length;
                }
            }
            
            if (!line.empty()) {
                tokens.emplace_back(TokenType::ENDOFLINE, "\n", lineNumber, columnNumber);
            }
            lineNumber++;
        }
        
        tokens.emplace_back(TokenType::ENDOFFILE, "", lineNumber, 1);
        return tokens;
    }

    void Lexer::printTokens(const std::vector<Token>& tokens) const {
        for(const auto& token : tokens) {
            std::cout << "Token: " << token.typeToString() 
                      << ", Value: " << token.getValue() 
                      << ", Line: " << token.getLine() 
                      << ", Column: " << token.getColumn() << std::endl;
        }
    }

}