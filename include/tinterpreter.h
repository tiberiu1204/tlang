#ifndef TINTERPRETER_H_INCLUDED
#define TINTERPRETER_H_INCLUDED
#include<tparser.h>

class RuntimeError : std::exception {
public:
    Token token;
    std::string message;
    RuntimeError(Token, const std::string&);
};

class Interpreter {
private:
    std::vector<ASTnode*> stmtList;
    std::map<std::string, Object*> identMap;

    bool checkIfIdentDeclared(Token);
    bool checkIfIdentDeclaration(const ASTnode*);

    void reportRuntimeError(const RuntimeError&);

    void print(ASTnode*);
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
