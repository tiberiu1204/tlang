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
    std::vector<ASTnode*> stmtList;

    void reportRuntimeError(const RuntimeError&);
    void interpretNode(ASTnode*);
    void print(ASTnode*);
    void addition(ASTnode*);
    void subtraction(ASTnode*);
    void multiplication(ASTnode*);
    void division(ASTnode*);
    void negation(ASTnode*);
    void comparison(ASTnode*);
    void ternary(ASTnode*);
public:
    Interpreter(std::vector<ASTnode*>);
    void interpret();
};

#endif // TINTERPRETER_H_INCLUDED
