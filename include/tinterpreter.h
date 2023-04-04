#ifndef TINTERPRETER_H_INCLUDED
#define TINTERPRETER_H_INCLUDED
#include<tparser.h>

class Interpreter {
private:
    ASTnode* ASTroot;
public:
    Interpreter(ASTnode*);
};

#endif // TINTERPRETER_H_INCLUDED
