#include <iostream>
#include <fstream>
#include <string>
#include "lexer/lexer.hpp"
#include "lexer/token.hpp"
#include "parser/parser.hpp"
#include "common/utils.hpp"
#include "interpreter/instruction_generator.hpp"
#include "executor/executor.hpp"
#include <typeinfo>

void printInstructions(const std::vector<grs_interpreter::Instruction>& instructions) {
    std::cout << "Commands:" << std::endl;
    for (const auto& inst : instructions) {
        std::cout << "command: " << inst.command << std::endl;        
   
        for (const auto& arg : inst.args) {
            std::cout << "  " << arg.first << " = ";
            std::visit([](const auto& value){
                std::cout<<value;
            },  arg.second);
            std::cout<< std::endl;
        }
          for (const auto& location : inst.commandLocationInfo) {
            std::cout << "  Location: Line " << location.first 
                      << ", Column " << location.second << std::endl;
        }

        std::cout << "-------------------" << std::endl;
        

    }
}

int main() {

 std::ifstream file("/home/cengo/grs-interpreter/grs_interpreter/tests/pos_type_convertion.txt");
    if(!file.is_open()){
        std::cerr << "Error opening file." << std::endl;
        return 1;
    }
    
    std::string code((std::istreambuf_iterator<char>(file)),
                     std::istreambuf_iterator<char>());
    file.close();

    std::cout << "grs Code:" << std::endl << code << std::endl;
    std::cout << "-------------------" << std::endl;
    
    // Lexer
    grs_lexer::Lexer lexer;
    auto tokens = lexer.tokenize(code);
    
    std::cout << "Tokens :" << std::endl;
    for (const auto& token : tokens) {
        std::cout << "  " << static_cast<int>(token.getType()) << " - " << token.getValue() << std::endl;
    }
    std::cout << "-------------------" << std::endl;
    
    // Parser
    grs_parser::Parser parser;
    auto ast = parser.parse(tokens);
    
    if (parser.hasErrors()) {
        std::cout << "Parser Errors:" << std::endl;
        for (const auto& error : parser.getErrors()) {
            std::cout << "  " << error.message << " (Line: " << error.line << ")" << std::endl;
        }
        return 1;
    }
    
    if (!ast) {
        std::cout << "AST not become!" << std::endl;
        return 1;
    }
    
    std::cout << "AST succeed!" << std::endl;
    std::cout << "AST statements numbers: " << ast->getStatements().size() << std::endl;
    std::cout << "-------------------" << std::endl;
    
    // Instruction Generator
    grs_interpreter::InstructionGenerator generator;
    auto instructions = generator.generateInstructions(ast);
    
    std::cout << "Instruction numbers: " << instructions.size() << std::endl;
    printInstructions(instructions);

    // grs_interpreter::Executor executor;

    // executor.executeInstruction(instructions);

        
    return 0;
}