#ifndef TINTERPRETER_H_INCLUDED
#define TINTERPRETER_H_INCLUDED
#include<tparser.h>
#include<unordered_map>
#include<unordered_set>
#include<tfunctions.h>

class UserFunction : public Function {
public:
    UserFunction* clone();
    UserFunction(const std::string&, const std::vector<std::string>&, ASTnode*);
    std::unique_ptr<Object> call(const std::vector<std::unique_ptr<Object> >&, const Token&, Interpreter*);
};

typedef std::unordered_map<std::string, std::unique_ptr<Object> > Scope;

class Interpreter {
private:
    std::vector<ASTnode*> stmtList;
    std::vector<Scope>* scopes;

    void reportRuntimeError(const RuntimeError&);
    void popScope();
    void pushScope();
    void clearScope(std::unordered_set<std::string>);
    std::unique_ptr<Object>* getObject(const std::string&);
    friend std::unique_ptr<Object> UserFunction::call(const std::vector<std::unique_ptr<Object> >&, const Token&, Interpreter*);

    void print(ASTnode*);
    void executeBlock(ASTnode*);
    void exprStmt(ASTnode*);
    void ifStmt(ASTnode*);
    void whileStmt(ASTnode*);
    void forStmt(ASTnode*);
    void funcDecl(ASTnode*);
    void returnStmt(ASTnode*);

    std::unique_ptr<Object> callFunction(ASTnode*);
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
