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

class Parser {
private:
    std::vector<Token> tokens;
    unsigned long long curPos = 0;
    State peek();
    Token prev();
    Token advance();
    bool isAtEnd();
    bool check(State);
    void consume(State, const char*);
    bool match(std::vector<State>);

    ASTnode* expression();
    ASTnode* equality();
    ASTnode* comparison();
    ASTnode* term();
    ASTnode* factor();
    ASTnode* unary();
    ASTnode* primary();
public:
    Parser(std::vector<Token>);
    ASTnode* parse();
};

#endif // TPARSER_H_INCLUDED
