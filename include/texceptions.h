#ifndef TEXCEPTIONS_H_INCLUDED
#define TEXCEPTIONS_H_INCLUDED
#include <tlexer.h>

class ParseError : public std::exception {};

class RuntimeError : std::exception {
public:
    Token token;
    std::string message;
    RuntimeError(Token, const std::string&);
};

class ContinueStmt : std::exception {};

class BreakStmt : std::exception {};

#endif // TEXCEPTIONS_H_INCLUDED
