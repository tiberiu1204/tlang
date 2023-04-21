#ifndef TINTERPRETER_H_INCLUDED
#define TINTERPRETER_H_INCLUDED
#include<tparser.h>
#include<unordered_map>
#include<unordered_set>
#include<memory>
#include<tparser.h>

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
    std::vector<Scope>* scopes;
    std::map<std::string, ASTnode*> functions;

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
    std::unique_ptr<Object> call(ASTnode*);
    std::unique_ptr<Object> callFunciton(ASTnode*, Object**, const size_t&);
    std::unique_ptr<Object> primary(ASTnode*);
    std::unique_ptr<Object> varDecl(ASTnode*);
    std::unique_ptr<Object> identifier(ASTnode*);
    std::unique_ptr<Object> addition(ASTnode*);
    std::unique_ptr<Object> subtraction(ASTnode*);
    std::unique_ptr<Object> multiplication(ASTnode*);
    std::unique_ptr<Object> division(ASTnode*);
    std::unique_ptr<Object> negation(ASTnode*);
    std::unique_ptr<Object> comparison(ASTnode*);
    std::unique_ptr<Object> ternary(ASTnode*);
    std::unique_ptr<Object> logic_and(ASTnode*);
    std::unique_ptr<Object> logic_or(ASTnode*);
    std::unique_ptr<Object> interpretNode(ASTnode*);
public:
    Interpreter(std::vector<ASTnode*>);
    void interpret();
};

#endif // TINTERPRETER_H_INCLUDED
