#ifndef TINTERPRETER_H_INCLUDED
#define TINTERPRETER_H_INCLUDED
#include<tparser.h>

class RuntimeError : std::exception {
public:
    Token token;
    std::string message;
    RuntimeError(const Token&, const std::string&);
};

class Interpreter {
private:
    ASTnode* ASTroot;

    void reportRuntimeError(const RuntimeError&);
    void interpretNode(ASTnode*);
    void addition(ASTnode*);
    void subtraction(ASTnode*);
    void multiplication(ASTnode*);
    void division(ASTnode*);
    void negation(ASTnode*);
    void comparison(ASTnode*);
    void ternary(ASTnode*);
public:
    Interpreter(ASTnode*);
    ASTnode* interpret();
};

#endif // TINTERPRETER_H_INCLUDED
