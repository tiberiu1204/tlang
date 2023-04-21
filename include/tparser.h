#ifndef TPARSER_H_INCLUDED
#define TPARSER_H_INCLUDED
#include<tlexer.h>

struct ASTnode {
    Token token;
    ASTnode* father = nullptr;
    std::vector<ASTnode*> childeren;

    ASTnode(Token);
    void addChild(ASTnode*);
    ~ASTnode();
};

class ParseError : public std::exception {
private:

public:
    ParseError();
};

class Parser {
private:
    std::vector<Token> tokens;
    unsigned long long curPos = 0;
    bool inLoop = false;

    Token peek();
    Token prev();
    Token advance();
    bool isAtEnd();
    bool check(State);
    void consume(State, const char*);
    bool match(std::vector<State>);
    ParseError error(const Token&, const std::string&);
    void printErrorMsg(const Token&, const std::string&);
    void synchronize();

    ASTnode* declaration();
    ASTnode* varDeclStmt();
    ASTnode* varDecl();
    ASTnode* declExpr();
    ASTnode* statement();
    ASTnode* block();
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
