#ifndef TEXCEPTIONS_H_INCLUDED
#define TEXCEPTIONS_H_INCLUDED
#include <tlexer.h>

class ParseError : public std::exception {};

class RuntimeError : public std::exception {
public:
    Token token;
    std::string message;
    RuntimeError(Token, const std::string&);
};

class ContinueStmt : public std::exception {};

class BreakStmt : public std::exception {};

class ReturnStmt : public std::exception {
public:
    Object* value;
    ReturnStmt(Object*);
};

#endif // TEXCEPTIONS_H_INCLUDED
