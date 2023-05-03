#ifndef TNATIVE_H_INCLUDED
#define TNATIVE_H_INCLUDED
#include<tparser.h>
#include<memory>
#include<unordered_map>
#include<unordered_set>
#include<stackframe.h>

const std::unordered_set<std::string> nativeFunctions =
{
    "clock",
    "floor"
};

class Interpreter;

class Function {
public:
    Function();
    Function(const std::string&);
    Function(const std::string&, const std::vector<std::string>&);
    Function(const std::string&, const std::vector<std::string>&, ASTnode*);
    Function(const std::string&, const std::vector<std::string>&, ASTnode*, const StackFrame&);

    virtual std::unique_ptr<Object> call(const std::vector<std::unique_ptr<Object> >&, const Token&, Interpreter*);
    virtual Function* clone();

    bool isNative();
    virtual ~Function() {}
    size_t arity();
    std::string func_name();
    void setStackFrame(const StackFrame&);
    StackFrame getStackFrame();
protected:
    std::string m_Name;
    ASTnode* m_Body;
    std::vector<std::string> m_Parameters;
    StackFrame m_StackFrame;
    Token m_Token;
};

class ClockFuntion : public Function {
public:
    ClockFuntion();
    std::unique_ptr<Object> call(const std::vector<std::unique_ptr<Object> >&, const Token&, Interpreter*);
};

class FloorFunction : public Function {
public:
    FloorFunction();
    std::unique_ptr<Object> call(const std::vector<std::unique_ptr<Object> >&, const Token&, Interpreter*);
};

Scope defineNativeFunctions();

#endif // TNATIVE_H_INCLUDED
