#ifndef VIRTUAL_MEMORY_HPP_
#define VIRTUAL_MEMORY_HPP_

#include <string>
#include <unordered_map>
#include <stdexcept>
#include <variant>

namespace krl_interpreter {


    class VirtualMemory{
        public:
        using ValueType = std::variant<int,double,bool,std::string>;
        VirtualMemory();
        ~VirtualMemory();


        void setVariable(const std::string& name, const ValueType& value);
        ValueType getVariable(const std::string& name)const;
        bool hasVariable(const std::string& name) const;
        void removeVariable(const std::string& name);
        void clear();

        void setInt(const std::string& name, int value);
        void setDouble(const std::string& name, double value);
        void setBool(const std::string& name, bool value);
        void setString(const std::string& name, const std::string& value);

        int getInt(const std::string& name) const;
        double getDouble(const std::string& name) const;
        bool getBool(const std::string& name) const;
        std::string getString(const std::string& name) const;
        

        private:
        std::unordered_map<std::string, ValueType> memory_;

    };

    class MemoryError : public std::runtime_error{
        public:
        explicit MemoryError(const std::string& message);
    };


}

#endif  // VIRTUAL_MEMORY_HPP_