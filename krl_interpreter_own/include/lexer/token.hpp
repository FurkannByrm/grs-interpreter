#pragma once
#include <string>

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
        CHAR,       // Character type
        STRUCT,     // Structure type
        
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
        SPL,        // Spline motion
        SLIN,       // Synchronized linear
        SCIRC,      // Synchronized circular
        
        // System functions
        WAIT,       // Wait command
        DELAY,      // Delay command
        IN,         // Digital input
        OUT,        // Digital output
        
        // Data types
        FRAME,      // Frame data type
        AXIS,       // Axis positions
        E6AXIS,     // Extended axis
        E6POS,      // Extended position
        
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
        COMMA,      // ,
        SEMICOLON,  // ; (comment)
        AMPERSAND,  // & (line continuation)
        SINGLEQUOTE,// ' (string literal)
  
        
        
        // Literals
        IDENTIFIER, // Variable/function names
        INTEGER,    // Integer literal
        FLOAT,      // Floating point literal
        BOOLEAN,    // True/False literal
        STRING,     // String literal
        
        // Special
        ENDOFLINE,  // End of line/statement
        ENDOFFILE,  // End of file
        INVALID     // Invalid token
    };

class Token {
public:
    Token(TokenType type, const std::string_view& value, int line, int column);
    
    TokenType getType() const;
    std::string_view getValue() const;
    int getLine() const;
    int getColumn() const;
    
    std::string typeToString() const; 
    
private:
    TokenType type_;
    std::string value_;
    int line_;
    int column_;
};

} // namespace krl