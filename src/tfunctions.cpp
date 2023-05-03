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

Function::Function(const std::string& name, const std::vector<std::string>& parameters) {
    m_Name = name;
    m_Parameters = parameters;
    m_Body = nullptr;
}

Function::Function(const std::string& name, const std::vector<std::string>& parameters, ASTnode* body) {
    m_Name = name;
    m_Body = body;
    m_Parameters = parameters;
}

Function::Function(const std::string& name, const std::vector<std::string>& parameters, ASTnode* body, const StackFrame& stackFrame) {
    m_Name = name;
    m_Body = body;
    m_Parameters = parameters;
    m_StackFrame = stackFrame;
}

void Function::setStackFrame(const StackFrame& stackFrame) {
    m_StackFrame = stackFrame;
    m_StackFrame.pushScope();
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

StackFrame Function::getStackFrame() {
    return m_StackFrame;
}

//floor() native function
//has one parameter("number") and returns the floored number

FloorFunction::FloorFunction() :
    Function("floor", std::vector<std::string>({"number"})) {}

std::unique_ptr<Object> FloorFunction::call(const std::vector<std::unique_ptr<Object> >& arguments, const Token& token, Interpreter* interpreter) {
    if(arguments.size() != 1) {
        throw RuntimeError(token, "expected 1 arguments, got " + std::to_string(arguments.size()));
    }
    Object* number = arguments[0].get();
    if(!number->instanceof(NUMBER)) {
        throw RuntimeError(token, "expected number argument");
    }
    int flooredNumber = (int)getValue<double>(number);
    return std::unique_ptr<Object>(new Obj<double>(NUMBER, (double)flooredNumber));
}

//clock() native function
//takes no argument and returns unix time in miliseconds

#include <chrono>

ClockFuntion::ClockFuntion() :
    Function("clock") {}


std::unique_ptr<Object> ClockFuntion::call(const std::vector<std::unique_ptr<Object> >& arguments, const Token& token, Interpreter* interpreter) {
    if(arguments.size() != 0) {
        throw RuntimeError(token, "expected 0 arguments, got " + std::to_string(arguments.size()));
    }
    using namespace std::chrono;
    return std::unique_ptr<Object>(new Obj<double>(NUMBER, (double)duration_cast<milliseconds>(system_clock::now().time_since_epoch()).count()));
}


Scope defineNativeFunctions() {

    Scope scope;

    scope["clock"] = new Obj<Function*>(FUNCTION, new ClockFuntion);
    scope["floor"] = new Obj<Function*>(FUNCTION, new FloorFunction);

    return scope;
}
