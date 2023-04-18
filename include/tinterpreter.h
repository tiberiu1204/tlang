#ifndef TINTERPRETER_H_INCLUDED
#define TINTERPRETER_H_INCLUDED
#include<tparser.h>
#include<unordered_map>
#include<unordered_set>
#include<memory>

class RuntimeError : std::exception {
public:
    Token token;
    std::string message;
    RuntimeError(Token, const std::string&);
};

class ContinueStmt : std::exception {};

class BreakStmt : std::exception {};

typedef std::unordered_map<std::string, std::unique_ptr<Object> > Scope;

class Interpreter {
private:
    std::vector<ASTnode*> stmtList;
    std::vector<Scope> scopes;

    void reportRuntimeError(const RuntimeError&);
    void popScope();
    void clearScope(std::unordered_set<std::string>);
    std::unique_ptr<Object>* getVariable(const std::string&);

    void print(ASTnode*);
    void executeBlock(ASTnode*);
    void exprStmt(ASTnode*);
    void ifStmt(ASTnode*);
    void whileStmt(ASTnode*);
    void forStmt(ASTnode*);
    Object* primary(ASTnode*);
    Object* varDecl(ASTnode*);
    Object* identifier(ASTnode*);
    Object* addition(ASTnode*);
    Object* subtraction(ASTnode*);
    Object* multiplication(ASTnode*);
    Object* division(ASTnode*);
    Object* negation(ASTnode*);
    Object* comparison(ASTnode*);
    Object* ternary(ASTnode*);
    Object* logic_and(ASTnode*);
    Object* logic_or(ASTnode*);
    Object* interpretNode(ASTnode*);
public:
    Interpreter(std::vector<ASTnode*>);
    void interpret();
};

#endif // TINTERPRETER_H_INCLUDED
