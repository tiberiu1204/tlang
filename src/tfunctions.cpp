#include<tfunctions.h>

//Base Class of Function

Function::Function() {
    m_Name = "";
    m_Body = nullptr;
}

Function::Function(const std::string& name, const std::vector<Object*>& arguments, ASTnode* body) {
    m_Name = name;
    m_Body = body;
    m_Arguments = arguments;
}

std::unique_ptr<Object> Function::call() {
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

Function::~Function() {
    for(size_t i = 0; i < m_Arguments.size(); ++i) {
        delete m_Arguments[i];
    }
}

//clock() native function
//takes no argument and returns unix time in miliseconds

#include <chrono>

ClockFuntion::ClockFuntion() {
    Function("clock", std::vector<Object*>(), nullptr);
}

std::unique_ptr<Object> ClockFuntion::call() {
    using namespace std::chrono;
    return std::unique_ptr<Object>(new Obj<double>(NUMBER, (double)duration_cast<milliseconds>(system_clock::now().time_since_epoch()).count()));
}

std::unordered_map<std::string, Function*> defineNativeFunctions() {

    //NFM = Native Function Map

    std::unordered_map<std::string, Function*> NFM;

    NFM["clock"] = new ClockFuntion();

    return NFM;
}
