#ifndef TNATIVE_H_INCLUDED
#define TNATIVE_H_INCLUDED
#include<tparser.h>

class Function {
public:
    Function();
    Function(const std::string&, const size_t&, ASTnode*);
    virtual Object* call();
    bool isNative();
    virtual ~Function() {}
    size_t arity();
    std::string func_name();
private:
    size_t m_ArgumentCount;
    std::string m_Name;
    ASTnode* m_Body;
};

#endif // TNATIVE_H_INCLUDED
