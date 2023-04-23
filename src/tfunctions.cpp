#include<tfunctions.h>

//Base Class of Function

Function::Function() {
    m_Name = "";
    m_Body = nullptr;
}

Function::Function(const std::string& name) {
    m_Name = name;
    m_body = nullptr;
}

Function::Function(const std::string& name, const std::vector<const char*>& arguments, ASTnode* body) {
    m_Name = name;
    m_Body = body;
    m_Arguments = arguments;
}

std::unique_ptr<Object> Function::call(const std::vector<Object*>& arguments) {
    return nullptr;
}

bool Function::isNative() {
    if(m_Body == nullptr) {
        return true;
    }
    return false;
}

size_t Function::arity() {
    return m_Arguments.size();
}
std::string Function::func_name() {
    return m_Name;
}

//clock() native function
//takes no argument and returns unix time in miliseconds

#include <chrono>

ClockFuntion::ClockFuntion() {
    Function("clock", std::vector<const char*>(), nullptr);
}

std::unique_ptr<Object> ClockFuntion::call(const std::vector<Object*>& arguments) {
    if(arguments.size() != m_Arguments.size()) {
        //do stuff here
    }
    using namespace std::chrono;
    return std::unique_ptr<Object>(new Obj<double>(NUMBER, (double)duration_cast<milliseconds>(system_clock::now().time_since_epoch()).count()));
}

std::unordered_map<std::string, std::unique_ptr<Function> > defineNativeFunctions() {

    //NFM = Native Function Map

    std::unordered_map<std::string, std::unique_ptr<Function>> NFM;

    NFM["clock"] = std::unique_ptr<Function>(new ClockFuntion());

    return NFM;
}
