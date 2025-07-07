#pragma once
#include <string>
#include <iostream>
#include "constexpr_map/constexpr_map.hpp"

namespace krl_lexer {
    enum class TokenType {
        // Program structure
        DEF,        // Program definition
        END,        // Program end
        
        // Variable declarations
        DECL,       // Declaration keyword
        INT,        // Integer type
        REAL,       // Floating point type
        BOOL,       // Boolean type
        CHAR,       // Character type, string type
        
        // Control flow
        IF,         // If statement
        THEN,       // Then clause
        ELSE,       // Else clause
        ENDIF,      // End if
        FOR,        // For loop
        TO,         // To keyword in for loop
        STEP,       // Step in for loop
        ENDFOR,     // End for
        WHILE,      // While loop
        ENDWHILE,   // End while
        REPEAT,     // Repeat loop
        UNTIL,      // Until condition
        SWITCH,     // Switch statement
        CASE,       // Case in switch
        DEFAULT,    // Default case
        ENDSWITCH,  // End switch
        GOTO,       // Goto statement
        HALT,       // Program halt
        RETURN,     // Function return
        
        // Motion commands
        PTP,        // Point-to-point motion
        LIN,        // Linear motion
        CIRC,       // Circular motion
        SPLINE,        // Spline motion
    
        
        // System functions
        WAIT,       // Wait command
        DELAY,      // Delay command
        IN,         // Digital input
        OUT,        // Digital output
        
        // Data types
        FRAME,          // Frame data type
        POSITION_NAME,  //P1, P2, P10, P100, HOME, APPROACH, SAFE_POS, PICK_POS, DROP_POS_1
        AXIS,           // Axis positions
        AXIS_NAME,      // (X, Y, Z, A, B, C)
        E6AXIS,         // Extended axis
        E6POS,          // Extended position
        NUMERIC_VALUE,  // Numeric value P1 {X 100, Y 200, Z 300, A 0, B 0, C 0}
        
        // Operators
        ASSIGN,     // := assignment
        PLUS,       // + addition
        MINUS,      // - subtraction
        MULTIPLY,   // * multiplication
        DIVIDE,     // / division
        AND,        // Logical AND
        OR,         // Logical OR
        NOT,        // Logical NOT
        EQUAL,      // == equality
        NOTEQUAL,   // <> inequality
        GREATER,    // > greater than
        LESS,       // < less than
        GREATEREQ,  // Corrected to match string mapping
        LESSEQ,     // Corrected to match string mapping
        
        // Punctuation
        LPAREN,     // (
        RPAREN,     // )
        LBRACE,     // {
        RBRACE,     // {
        COMMA,      // ,
        SEMICOLON,  // ; (comment)
        AMPERSAND,  // & (line continuation)
        SINGLEQUOTE,// ' (string literal)
  
        
        
        // Literals
        IDENTIFIER, // Variable/function names
        INTEGER,    // Integer literal
        FLOAT,      // Floating point literal
        STRING,     // String literal
        
        //Constant
        TRUE,
        FALSE,
        PI,
        // Special
        ENDOFLINE,  // End of line/statement
        ENDOFFILE,  // End of file
        INVALID     // Invalid token
    };

    inline constexpr auto typeToStringMap = cxmap::ConstexprMap<TokenType, std::string_view, 68>({
        {

        {TokenType::DEF, "DEF"},
        {TokenType::END, "END"},
        {TokenType::DECL, "DECL"},
        {TokenType::INT, "INT"},
        {TokenType::REAL, "REAL"},
        {TokenType::BOOL, "BOOL"},
        {TokenType::CHAR, "CHAR"},
        {TokenType::IF, "IF"},
        {TokenType::THEN, "THEN"},
        {TokenType::ELSE, "ELSE"},
        {TokenType::ENDIF, "ENDIF"},
        {TokenType::FOR, "FOR"},
        {TokenType::TO, "TO"},
        {TokenType::STEP, "STEP"},
        {TokenType::ENDFOR, "ENDFOR"},
        {TokenType::WHILE, "WHILE"},
        {TokenType::ENDWHILE, "ENDWHILE"},
        {TokenType::REPEAT, "REPEAT"},
        {TokenType::UNTIL, "UNTIL"},
        {TokenType::SWITCH, "SWITCH"},
        {TokenType::CASE, "CASE"},
        {TokenType::DEFAULT, "DEFAULT"},
        {TokenType::ENDSWITCH, "ENDSWITCH"},
        {TokenType::GOTO, "GOTO"},
        {TokenType::HALT, "HALT"},
        {TokenType::RETURN, "RETURN"},
        {TokenType::PTP, "PTP"},
        {TokenType::LIN, "LIN"},
        {TokenType::CIRC, "CIRC"},
        {TokenType::SPLINE, "SPL"},
        {TokenType::WAIT, "WAIT"},
        {TokenType::DELAY, "DELAY"},
        {TokenType::IN, "IN"},
        {TokenType::OUT, "OUT"},
        {TokenType::FRAME, "FRAME"},
        {TokenType::AXIS, "AXIS"},
        {TokenType::E6AXIS, "E6AXIS"},
        {TokenType::E6POS, "E6POS"},
        {TokenType::LPAREN, "LPAREN"},
        {TokenType::RPAREN, "RPAREN"},
        {TokenType::COMMA, "COMMA"},
        {TokenType::SEMICOLON, "SEMICOLON"},
        {TokenType::AMPERSAND, "AMPERSAND"},
        {TokenType::SINGLEQUOTE, "SINGLEQUOTE"},
        {TokenType::IDENTIFIER, "IDENTIFIER"},
        {TokenType::STRING, "STRING"},
        {TokenType::INTEGER, "INTEGER"},
        {TokenType::TRUE, "TRUE"},
        {TokenType::FALSE, "FALSE"},
        {TokenType::PI, "PI"},
        {TokenType::FLOAT, "FLOAT"},
        {TokenType::ENDOFLINE, "ENDOFLINE"},
        {TokenType::ENDOFFILE, "ENDOFFILE"},
        {TokenType::INVALID, "INVALID"},
        {TokenType::NOT, "NOT"},
        {TokenType::AND, "AND"},
        {TokenType::OR, "OR"},
        {TokenType::ASSIGN, "ASSIGN"},
        {TokenType::PLUS, "PLUS"},
        {TokenType::MINUS, "MINUS"},
        {TokenType::MULTIPLY, "MULTIPLY"},
        {TokenType::DIVIDE, "DIVIDE"},
        {TokenType::EQUAL, "EQUAL"},
        {TokenType::NOTEQUAL, "NOTEQUAL"},
        {TokenType::LESS, "LESS"},
        {TokenType::GREATER, "GREATER"},
        {TokenType::GREATEREQ, "GREATEREQ"},
        {TokenType::LESSEQ, "LESSEQ"}
    }

    });

class Token {
public:
    Token(TokenType type, const std::string& value, int line, int column);
    TokenType getType() const;
    std::string getValue() const;
    int getLine() const;
    int getColumn() const;
    
    std::string_view typeToString() const; 
    

    
private:
    TokenType type_;
    std::string value_;
    int line_;
    int column_;
    
};

} // namespace krl