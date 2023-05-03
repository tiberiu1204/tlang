#ifndef STACKFRAME_H_INCLUDED
#define STACKFRAME_H_INCLUDED
#include<tlexer.h>
#include<unordered_map>
#include<memory>

typedef std::unordered_map<std::string, Object* > Scope;

struct StackFrame {
public:
    StackFrame();
    void pushScope();
    void popScope();
    void insertObject(const std::string&, Object*);
    void replaceObject(const std::string&, size_t, Object*, const Token&);
    size_t size();
    Object* getObject(const std::string&, size_t);
    Scope& back();
    Scope& operator[](size_t);
private:
    std::vector<Scope> m_Scopes;
};

#endif // STACKFRAME_H_INCLUDED
