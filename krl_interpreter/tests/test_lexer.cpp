#include "lexer/lexer.hpp"


int main(){

    std::ifstream file("/home/cengo/interpreter_demos/krl_interpreter_own/tests/krl.txt");
    if(!file.is_open()){
        std::cerr << "Error opening file." << std::endl;
        return 1;
    }
    
  
    std::string code((std::istreambuf_iterator<char>(file)),
                     std::istreambuf_iterator<char>());
    file.close();

    krl_lexer::Lexer lexer;
    std::vector<krl_lexer::Token> tokens = lexer.tokenize(code);

    lexer.printTokens(tokens);

    if (lexer.hasErrors()) {
        for (const auto& error : lexer.getErrors()) {
            std::cerr << error.getFormattedMessage() << std::endl;
        }
    }
    else {
        std::cout << "No errors found." << std::endl;
    }


    return 0;




}