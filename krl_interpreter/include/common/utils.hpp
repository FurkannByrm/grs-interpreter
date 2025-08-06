#ifndef COMMON_UTILS_HPP_
#define COMMON_UTILS_HPP_

#include <iostream>
#include <variant>
#include <string>
#include <memory>
#include <unordered_map>
#include <map>
#include "ast/visitor.hpp"
namespace common{

    struct LineColumnExpr{

        std::string message;
        int line;
        int column;
    };
    
    struct Position{
    double x = 0.0;
    double y = 0.0;
    double z = 0.0;
    double a = 0.0; //roll 
    double b = 0.0; //pich 
    double c = 0.0; //yaw
    
    
};

struct  Frame{
    double x = 0.0;
    double y = 0.0;
    double z = 0.0;
    double a = 0.0; //roll 
    double b = 0.0; //pich 
    double c = 0.0; //yaw

};

struct Axis{
    double A1 = 0.0;
    double A2 = 0.0;
    double A3 = 0.0;
    double A4 = 0.0; 
    double A5 = 0.0; 
    double A6 = 0.0; 
    
};

struct MotionParameters {
    double velocity = 100.0;  // mm/s
    double acceleration = 1000.0;  // mm/s^2
    double deceleration = 1000.0;  // mm/s^2
    double jerk = 5000.0;  // mm/s^3
};


enum class MotionType{

    PTP,
    LIN,
    CIRC,
    SPL

};

template<typename StructType>
std::string toString(const StructType& type){
    if constexpr(std::is_same_v<StructType, Position>)
    {
        return "{x : " + std::to_string(type.x) + " , y : " +  
        std::to_string(type.y) + " , z : " +std::to_string(type.z) + 
        " , a : " + std::to_string(type.a) + " , b : " + std::to_string(type.b) +
        " , c : " + std::to_string(type.c) + " }";
    } 
    else if constexpr(std::is_same_v<StructType, Frame>)
    {
        return "{x : " + std::to_string(type.x) + " , y : " +  
        std::to_string(type.y) + " , z : " + std::to_string(type.z) + 
        " , a : " + std::to_string(type.a) + " , b : " + std::to_string(type.b) +
        " , c : " + std::to_string(type.c) + " }";
        
    }    
    else if constexpr(std::is_same_v<StructType, Axis>)
    {
        return "{A1 : " + std::to_string(type.A1) + " , A2 : " +  
        std::to_string(type.A2) + " , A3 : " +std::to_string(type.A3) + 
        " , A4 : " + std::to_string(type.A4) + " , A5 : " + std::to_string(type.A5) +
        " , A6 : " + std::to_string(type.A6) + " }";
    }
    else{
        return "unknown type";
    }
        
} 

template<typename StructType>
std::enable_if_t<std::is_same_v<StructType, Position> ||
                std::is_same_v<StructType, Frame>     ||
                std::is_same_v<StructType, Axis>,
                std::ostream&>
    inline operator<<(std::ostream& os, StructType type){
        return os << toString(type);
    }


using ValueType = std::variant<int, double, bool, std::string, std::shared_ptr<krl_ast::Expression>, Position, Frame, Axis>;  



} 


#endif //COMMON_UTILS_HPP_