#include<stackframe.h>

StackFrame::StackFrame() {
    m_Scopes.push_back(Scope());
}

void StackFrame::pushScope() {
    m_Scopes.push_back(Scope());
}

void StackFrame::popScope() {
    m_Scopes.pop_back();
}

Object* StackFrame::getObject(const std::string& name, size_t depth) {
    return m_Scopes[m_Scopes.size() - depth - 1][name];
}

void StackFrame::insertObject(const std::string& name, Object* object) {
    m_Scopes.back()[name] = object;
}

void StackFrame::replaceObject(const std::string& name, size_t depth, Object* other) {
    delete m_Scopes[m_Scopes.size() - depth - 1][name];
    m_Scopes[m_Scopes.size() - depth - 1][name] = other;
}

Scope& StackFrame::operator[](size_t index) {
    if(index >= m_Scopes.size()) {
        throw std::out_of_range("[DEBUG] Trying to access scope that is out of range");
    }
    return m_Scopes[index];
}

size_t StackFrame::size() {
    return m_Scopes.size();
}

Scope& StackFrame::back() {
    return m_Scopes.back();
}
