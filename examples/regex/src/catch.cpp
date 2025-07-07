#include <regex>
#include <iostream>
#include <string>

int main(){

    std::string text = "DEF move()";

    std::regex pattern(R"(DEF\s+(\w+)\(\))");  // R"()" raw string kullanımı

    std::smatch matches;
    if(std::regex_search(text, matches, pattern)){
        std::cout<<"Match found: "<<matches[0]<<std::endl;
        std::cout<<"Function name: "<<matches[1]<<std::endl;
    }



    return 0;
}