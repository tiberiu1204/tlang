#ifndef TNATIVE_H_INCLUDED
#define TNATIVE_H_INCLUDED
#include<tparser.h>
#include<memory>
#include<unordered_map>

class Function {
public:
    Function();
    Function(const std::string&, const std::vector<Object*>&, ASTnode*);
    virtual std::unique_ptr<Object> call();
    bool isNative();
    virtual ~Function();
    size_t arity();
    std::string func_name();
protected:
    std::string m_Name;
    ASTnode* m_Body;
    std::vector<Object*> m_Arguments;
};

class ClockFuntion : public Function {
public:
    ClockFuntion();
    std::unique_ptr<Object> call();
};

std::unordered_map<std::string, Function*> defineNativeFunctions();

#endif // TNATIVE_H_INCLUDED
