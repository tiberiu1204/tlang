#include<stackframe.h>
#include<texceptions.h>
#include<tfunctions.h>

StackFrame::StackFrame() {
    m_Scopes.push_back(Scope());
}

void StackFrame::pushScope() {
    m_Scopes.push_back(Scope());
}

void StackFrame::popScope() {
    m_Scopes.pop_back();
}

#include<iostream>
Object* StackFrame::getObject(const std::string& name, size_t depth) {
    return m_Scopes[m_Scopes.size() - depth - 1][name];
}

void StackFrame::insertObject(const std::string& name, Object* object) {
    m_Scopes.back()[name] = object;
}

void StackFrame::replaceObject(const std::string& name, size_t depth, Object* other, const Token& token) {
    Object* stored = m_Scopes[m_Scopes.size() - depth - 1][name];
    if(stored->type != other->type) {
        throw RuntimeError(token, "cannot store value of different type in variable");
    }
    switch(stored->type) {
    case NUMBER:
        *(double*)(stored + 1) = getValue<double>(other);
        break;
    case STRING:
        *(std::string*)(stored + 1) = getValue<std::string>(other);
        break;
    case FUNCTION:
        *(Function**)(stored + 1) = getValue<Function*>(other);
        break;
    }
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
