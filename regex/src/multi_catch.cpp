#include <iostream>
#include <regex>
#include <string>
#include <vector>
#include <fstream>

struct Axis{

    double x;
    double y;
    double z;
};



int main()
{
    
    std::regex pattern(R"(\bPTP\s*\(\s*([+-]?\d+(\.\d+)?)\s*,\s*([+-]?\d+(\.\d+)?)\s*,\s*([+-]?\d+(\.\d+)?)\s*\))");
    std::smatch matches;
    Axis axis;
    std::vector<Axis> axisList;

    std::fstream file ("text.txt");
    if(!file.is_open()){
        std::cerr<<"File not found"<<std::endl;
        return 1;
    }
    std::string line;
    while(std::getline(file, line)){
        std::regex_search(line,matches,pattern);
        axis.x = std::stod(matches[1]);
        axis.y = std::stod(matches[3]);
        axis.z = std::stod(matches[5]);
        axisList.push_back(axis);
    }
    
    file.close();

    
    for(auto & a : axisList)
    { 
        std::cout<<"X: "<<a.x<<std::endl;
        std::cout<<"Y: "<<a.y<<std::endl;
        std::cout<<"Z: "<<a.z<<std::endl;
    }


return 0;
}