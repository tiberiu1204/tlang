#ifndef TNATIVE_H_INCLUDED
#define TNATIVE_H_INCLUDED
#include<tparser.h>
#include<memory>
#include<unordered_map>
class Interpreter;

class Function {
public:
    Function();
    Function(const std::string&);
    Function(const std::string&, const std::vector<std::string>&, ASTnode*);

    virtual std::unique_ptr<Object> call(const std::vector<std::unique_ptr<Object> >&, const Token&, Interpreter*);
    virtual Function* clone();

    bool isNative();
    virtual ~Function() {}
    size_t arity();
    std::string func_name();
protected:
    std::string m_Name;
    ASTnode* m_Body;
    std::vector<std::string> m_Parameters;
    Token m_Token;
};

class FunctionObject : public Object {
public:
    Function* value;
    FunctionObject(Function*);
    FunctionObject(const FunctionObject&);
    FunctionObject* clone();
    ~FunctionObject();
};

class ClockFuntion : public Function {
public:
    ClockFuntion();
    std::unique_ptr<Object> call(const std::vector<std::unique_ptr<Object> >&, const Token&, Interpreter*);
};

void defineNativeFunctions(std::unordered_map<std::string, std::unique_ptr<Object> >&);

#endif // TNATIVE_H_INCLUDED
