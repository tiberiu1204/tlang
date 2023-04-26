#ifndef TRESOLVER_H_INCLUDED
#define TRESOLVER_H_INCLUDED
#include<tinterpreter.h>

class Resolver {
public:
    Resolver(const std::vector<ASTnode*>&, Interpreter*);
    bool run();
private:
    std::vector<ASTnode*> m_StmtList;
    Interpreter* m_pInterpreter;
    std::vector<std::unordered_set<std::string> > m_Scopes;

    void pushScope();
    void popScope();


    void print(ASTnode*);
    void block(ASTnode*);
    void exprStmt(ASTnode*);
    void ifStmt(ASTnode*);
    void whileStmt(ASTnode*);
    void forStmt(ASTnode*);
    void funcDecl(ASTnode*);
    void callFunction(ASTnode*);
    void varDecl(ASTnode*);
    void identifier(ASTnode*);
    void visitNode(ASTnode*);
    void visitNodeChildren(ASTnode*);
};

#endif // TRESOLVER_H_INCLUDED
