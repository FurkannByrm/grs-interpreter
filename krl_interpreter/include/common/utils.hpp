#ifndef COMMON_UTILS_HPP_
#define COMMON_UTILS_HPP_


#include <variant>
#include <string>
#include <memory>

#include "ast/visitor.hpp"
namespace common
{
using ValueType = std::variant<int, double, float, bool, std::string, std::shared_ptr<krl_ast::Expression>>;  
  
} 

#endif //COMMON_UTILS_HPP_