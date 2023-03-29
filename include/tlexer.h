#ifndef TLEXER_H_INCLUDED
#define TLEXER_H_INCLUDED
#include<vector>

enum Type {
    Num,
    Plus,
    Minus,
    Times,
    Divided,
    Equal,
    Identifier,
    True,
    False,
    eof
};

struct Token {
    Type type;
    char* text;
    long long startPos;

    Token(Type, char*, long long);
};

class Lexer {
private:
    char* input;
    char* delim;
public:
    Lexer();
    Lexer(char*);
    std::vector<Token*> lex();
};

#endif // TLEXER_H_INCLUDED
