#ifndef TINTERPRETER_H_INCLUDED
#define TINTERPRETER_H_INCLUDED
#include<tparser.h>

class RuntimeError : std::exception {
public:
    Token token;
    std::string message;
    RuntimeError(Token, const std::string&);
};

typedef std::map<std::string, Object*> Scope;

class Interpreter {
private:
    std::vector<ASTnode*> stmtList;
    std::vector<Scope> scopes;

    void reportRuntimeError(const RuntimeError&);
    Object* getVariable(const std::string&);

    void print(ASTnode*);
    void executeBlock(ASTnode*);
    Object* primary(ASTnode*);
    Object* identifier(ASTnode*);
    Object* addition(ASTnode*);
    Object* subtraction(ASTnode*);
    Object* multiplication(ASTnode*);
    Object* division(ASTnode*);
    Object* negation(ASTnode*);
    Object* comparison(ASTnode*);
    Object* ternary(ASTnode*);
    Object* interpretNode(ASTnode*);
public:
    Interpreter(std::vector<ASTnode*>);
    void interpret();
};

#endif // TINTERPRETER_H_INCLUDED
