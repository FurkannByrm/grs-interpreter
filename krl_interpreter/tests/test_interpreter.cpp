#include <iostream>
#include <fstream>
#include <string>
#include "lexer/lexer.hpp"
#include "lexer/token.hpp"
#include "parser/parser.hpp"
#include "interpreter/instruction_set.hpp"

void printInstructions(const std::vector<krl_interpreter::Instruction>& instructions) {
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
        std::cout << "-------------------" << std::endl;
    }
}

int main() {

 std::ifstream file("/home/cengo/krl-interpreter/krl_interpreter/tests/assign_and_type_convertion_test.txt");
    if(!file.is_open()){
        std::cerr << "Error opening file." << std::endl;
        return 1;
    }
    
    std::string code((std::istreambuf_iterator<char>(file)),
                     std::istreambuf_iterator<char>());
    file.close();

    std::cout << "KRL Code:" << std::endl << code << std::endl;
    std::cout << "-------------------" << std::endl;
    
    // Lexer
    krl_lexer::Lexer lexer;
    auto tokens = lexer.tokenize(code);
    
    std::cout << "Tokens :" << std::endl;
    for (const auto& token : tokens) {
        std::cout << "  " << static_cast<int>(token.getType()) << " - " << token.getValue() << std::endl;
    }
    std::cout << "-------------------" << std::endl;
    
    // Parser
    krl_parser::Parser parser;
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
    krl_interpreter::InstructionGenerator generator;
    auto instructions = generator.generateInstructions(ast);
    
    std::cout << "Instruction numbers: " << instructions.size() << std::endl;
    printInstructions(instructions);
    
    return 0;
}