#ifndef TPARSER_H_INCLUDED
#define TPARSER_H_INCLUDED
#include<tlexer.h>
#include<texceptions.h>
#include<unordered_map>

struct ASTnode {
    Token token;
    ASTnode* father = nullptr;
    std::vector<ASTnode*> childeren;

    ASTnode(Token);
    void addChild(ASTnode*);
    ~ASTnode();
};

class Parser {
private:
    std::vector<Token> tokens;
    size_t curPos = 0;
    bool inLoop = false;
    bool inFunction = false;

    void addScope();
    void popScope();
    Token peek();
    Token prev();
    Token advance();
    bool isAtEnd();
    bool check(State);
    void consume(State, const char*);
    bool match(std::vector<State>);
    void synchronize();

    ASTnode* declaration();
    ASTnode* funcDecl();
    ASTnode* functionProduction();
    ASTnode* parameters();
    ASTnode* varDeclStmt();
    ASTnode* varDecl();
    ASTnode* declExpr();
    ASTnode* statement();
    ASTnode* block();
    ASTnode* returnStmt();
    ASTnode* ifStmt();
    ASTnode* whileStmt();
    ASTnode* forStmt();
    ASTnode* exprStmt();
    ASTnode* exprBlock();
    ASTnode* printStmt();
    ASTnode* expression();
    ASTnode* assignment();
    ASTnode* logic_or();
    ASTnode* logic_and();
    ASTnode* ternary();
    ASTnode* equality();
    ASTnode* comparison();
    ASTnode* term();
    ASTnode* factor();
    ASTnode* unary();
    ASTnode* call();
    ASTnode* primary();
public:
    Parser(std::vector<Token>);
    std::vector<ASTnode*> parse();
};

#endif // TPARSER_H_INCLUDED
