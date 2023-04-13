#ifndef TPARSER_H_INCLUDED
#define TPARSER_H_INCLUDED
#include<tlexer.h>

struct ASTnode {
    Token token;
    ASTnode* father = nullptr;
    std::vector<ASTnode*> childeren;

    ASTnode(Token);
    void addChild(ASTnode*);
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

    Token peek();
    Token prev();
    Token advance();
    bool isAtEnd();
    bool check(State);
    void consume(State, const char*);
    bool match(std::vector<State>);
    ParseError error(Token, std::string);
    void printErrorMsg(Token, std::string);
    void synchronize();

    ASTnode* declaration();
    ASTnode* varDecl();
    ASTnode* statement();
    ASTnode* exprStmt();
    ASTnode* printStmt();
    ASTnode* expression();
    ASTnode* assignment();
    ASTnode* ternary();
    ASTnode* equality();
    ASTnode* comparison();
    ASTnode* term();
    ASTnode* factor();
    ASTnode* unary();
    ASTnode* primary();
public:
    Parser(std::vector<Token>);
    std::vector<ASTnode*> parse();
};

#endif // TPARSER_H_INCLUDED
