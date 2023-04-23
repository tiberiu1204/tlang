#include<tfunctions.h>
#include<texceptions.h>

//Base Class of Function

Function::Function() {
    m_Name = "";
    m_Body = nullptr;
}

Function::Function(const std::string& name) {
    m_Name = name;
    m_Body = nullptr;
}

Function::Function(const std::string& name, const std::vector<std::string>& parameters, ASTnode* body) {
    m_Name = name;
    m_Body = body;
    m_Parameters = parameters;
}


std::unique_ptr<Object> Function::call(const std::vector<std::unique_ptr<Object> >& arguments, const Token& token, Interpreter* interpreter) {
    return nullptr;
}

Function* Function::clone() {
    return new Function(*this);
}

bool Function::isNative() {
    if(m_Body == nullptr) {
        return true;
    }
    return false;
}

size_t Function::arity() {
    return m_Parameters.size();
}
std::string Function::func_name() {
    return m_Name;
}

FunctionObject::FunctionObject(Function* val) :
    Object(FUNCTION), value(val) {}

FunctionObject::FunctionObject(const FunctionObject& other) {
    value = other.value->clone();
}

FunctionObject* FunctionObject::clone() {
    return new FunctionObject(*this);
}

FunctionObject::~FunctionObject() {
    delete value;
}

//clock() native function
//takes no argument and returns unix time in miliseconds

#include <chrono>

ClockFuntion::ClockFuntion() :
    Function("clock", std::vector<std::string>(), nullptr) {}


std::unique_ptr<Object> ClockFuntion::call(const std::vector<std::unique_ptr<Object> >& arguments, const Token& token, Interpreter* interpreter) {
    if(arguments.size() != 0) {
        throw RuntimeError(token, "expected 0 arguments, got " + std::to_string(arguments.size()));
    }
    using namespace std::chrono;
    return std::unique_ptr<Object>(new Obj<double>(NUMBER, (double)duration_cast<milliseconds>(system_clock::now().time_since_epoch()).count()));
}

void defineNativeFunctions(std::unordered_map<std::string, std::unique_ptr<Object> >& scope) {

    //NFM = Native Function Map


    scope["clock"] = std::unique_ptr<Object>(new Obj<Function*>(FUNCTION, new ClockFuntion));
}
