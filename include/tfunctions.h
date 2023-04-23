#ifndef TNATIVE_H_INCLUDED
#define TNATIVE_H_INCLUDED
#include<tparser.h>
#include<memory>
#include<unordered_map>


class Function {
public:
    Function();
    Function(const std::string&);
    Function(const std::string&, const std::vector<const char*>&, ASTnode*);
    virtual std::unique_ptr<Object> call(const std::vector<Object*>&);
    bool isNative();
    virtual ~Function();
    size_t arity();
    std::string func_name();
protected:
    std::string m_Name;
    ASTnode* m_Body;
    std::vector<const char*> m_Arguments;
};

class ClockFuntion : public Function {
public:
    ClockFuntion();
    std::unique_ptr<Object> call(const std::vector<Object*>&);
};

std::unordered_map<std::string, std::unique_ptr<Function> > defineNativeFunctions();

#endif // TNATIVE_H_INCLUDED
