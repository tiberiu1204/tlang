#include<tfunctions.h>

Function::Function() {
    m_Name = "";
    m_ArgumentCount = 0;
    m_Body = nullptr;
}

Function::Function(const std::string& name, const size_t& argumentCount, ASTnode* body) {
    m_Name = name;
    m_ArgumentCount = argumentCount;
    m_Body = body;
}

virtual Object* Function::call() {
    return nullptr;
}

bool Function::isNative() {
    if(m_Body == nullptr) {
        return true;
    }
    return false;
}

size_t Function::arity() {
    return m_ArgumentCount;
}
std::string Function::func_name() {
    return m_Name;
}
