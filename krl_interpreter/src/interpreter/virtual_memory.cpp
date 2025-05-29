#include "interpreter/virtual_memory.hpp"

namespace krl_interpreter{

VirtualMemory::VirtualMemory(){};
VirtualMemory::~VirtualMemory(){};


void VirtualMemory::setVariable(const std::string& name, const ValueType& value)
{
    memory_[name] = value;
}

VirtualMemory::ValueType VirtualMemory::getVariable(const std::string& name)const {
    auto it = memory_.find(name);
    if(it == memory_.end()){
        throw MemoryError("Variable not found: " + name);
    }
    return it->second;
}

bool VirtualMemory::hasVariable(const std::string& name)const 
{
    return memory_.find(name) != memory_.end();
}

void VirtualMemory::removeVariable(const std::string& name) {
    auto it = memory_.find(name);
    if(it != memory_.end()) {
        memory_.erase(it);
    }
}

void VirtualMemory::clear(){
    memory_.clear();
}
void VirtualMemory::setInt(const std::string& name, int value){
    memory_[name] = value;
}
void VirtualMemory::setDouble(const std::string& name, double value){
    memory_[name] = value;
}
void VirtualMemory::setBool(const std::string& name, bool value){
    memory_[name] = value;
}
void VirtualMemory::setString(const std::string& name, const std::string& value){
    memory_[name] = value;
}

int VirtualMemory::getInt(const std::string& name) const {
    auto value = getVariable(name);
    if(std::holds_alternative<int>(value)){
        return std::get<int>(value);
    }
    throw MemoryError("Varible is not an integer: " + name);
}
double VirtualMemory::getDouble(const std::string& name) const {
    auto value = getVariable(name);
    if(std::holds_alternative<double>(value)){
        return std::get<double>(value);
    } else if(std::holds_alternative<int>(value))
    {
        return static_cast<double>(std::get<int>(value));
    }
    throw MemoryError("Variable is not a number: " + name);
}

bool VirtualMemory::getBool(const std::string& name)const{
    auto value = getVariable(name);
    if(std::holds_alternative<bool>(value))
    {
        return std::get<bool>(value);
    }
    throw MemoryError("Variable is not a boolean: "+ name);
}

std::string VirtualMemory::getString(const std::string& name)const{
    auto value = getVariable(name);
    if(std::holds_alternative<std::string>(value)){
        return std::get<std::string>(value);
    }
    throw MemoryError("Variable is not a string" + name);
}

MemoryError::MemoryError(const std::string& message) : std::runtime_error(message){}


}