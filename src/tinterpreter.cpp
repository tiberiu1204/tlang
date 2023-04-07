#include<tinterpreter.h>
#include<sstream>
#include<iostream>

/** utilities **/

namespace std {
    string to_string(const std::string& str) {
        return str;
    }
}

void clearNodeChildren(ASTnode* node) {
    while(!node->childeren.empty()) {
        delete node->childeren.back();
        node->childeren.pop_back();
    }
}

void updateToken(Token& token, State type, std::string text) {
    token.type = type;
    token.text = text;
}

int stringToInt(const std::string& str) {
    std::stringstream stream;
    int x;
    stream<<str;
    stream>>x;
    return x;
}

double stringToDouble(const std::string& str) {
    std::stringstream stream;
    double x;
    stream<<str;
    stream>>x;
    return x;
}

void checkNumberOperand(Token token, std::string errorMsg) {
    switch(token.type) {
    case INTLIT:
    case FLOATLIT:
        break;
    default:
        throw RuntimeError(token, errorMsg);
    }
}

void checkNumberOperands(Token left, Token right, std::string errorMsg) {
    switch(left.type) {
    case INTLIT:
    case FLOATLIT:
        break;
    default:
        throw RuntimeError(left, errorMsg);
    }
    switch(right.type) {
    case INTLIT:
    case FLOATLIT:
        break;
    default:
        throw RuntimeError(right, errorMsg);
    }
}

/** end of utilities **/

RuntimeError::RuntimeError(const Token& token, const std::string& message) {
    RuntimeError::token = token;
    RuntimeError::message = message;
}

void Interpreter::reportRuntimeError(const RuntimeError& error) {
    std::cout<<"[ERROR] at line "<<error.token.line<<" collumn "<<error.token.collumn<<" at '"<<error.token.text<<"': Runtime error: "<<error.message<<"\n";
}

Interpreter::Interpreter(ASTnode* root) {
    Interpreter::ASTroot = root;
}

void Interpreter::addition(ASTnode* expr) {
    Token left = expr->childeren[0]->token;
    Token right = expr->childeren[1]->token;

    if(left.type == right.type && right.type == INTLIT) {
        int x = stringToInt(left.text), y = stringToInt(right.text);
        updateToken(expr->token, INTLIT, std::to_string(x + y));
    }
    if(left.type == STRINGLIT || right.type == STRINGLIT) {
        updateToken(expr->token, STRINGLIT, std::to_string(left.text) + std::to_string(right.text));
    }
    if(left.type == FLOATLIT || right.type == FLOATLIT) {
        double x = stringToDouble(left.text), y = stringToDouble(right.text);
        updateToken(expr->token, FLOATLIT, std::to_string(x + y));
    }
    clearNodeChildren(expr);
}

void Interpreter::subtraction(ASTnode* expr) {
    Token left = expr->childeren[0]->token;
    Token right = expr->childeren[1]->token;
    checkNumberOperands(left, right, "subtraction can only be performed between numbers");

    if(left.type == right.type && right.type == INTLIT) {
        int x = stringToInt(left.text), y = stringToInt(right.text);
        updateToken(expr->token, INTLIT, std::to_string(x - y));
    }
    if(left.type == FLOATLIT || right.type == FLOATLIT) {
        double x = stringToDouble(left.text), y = stringToDouble(right.text);
        updateToken(expr->token, FLOATLIT, std::to_string(x - y));
    }
    clearNodeChildren(expr);
}

void Interpreter::multiplication(ASTnode* expr) {
    Token left = expr->childeren[0]->token;
    Token right = expr->childeren[1]->token;
    checkNumberOperands(left, right, "multiplication can only be performed between numbers");

    if(left.type == right.type && right.type == INTLIT) {
        int x = stringToInt(left.text), y = stringToInt(right.text);
        updateToken(expr->token, INTLIT, std::to_string(x * y));
    }
    if(left.type == FLOATLIT || right.type == FLOATLIT) {
        double x = stringToDouble(left.text), y = stringToDouble(right.text);
        updateToken(expr->token, FLOATLIT, std::to_string(x * y));
    }
    clearNodeChildren(expr);
}

void Interpreter::division(ASTnode* expr) {
    Token left = expr->childeren[0]->token;
    Token right = expr->childeren[1]->token;
    checkNumberOperands(left, right, "division can only be performed between numbers");

    if(left.type == right.type && right.type == INTLIT) {
        int x = stringToInt(left.text), y = stringToInt(right.text);
        updateToken(expr->token, INTLIT, std::to_string(x / y));
    }
    if(left.type == FLOATLIT || right.type == FLOATLIT) {
        double x = stringToDouble(left.text), y = stringToDouble(right.text);
        updateToken(expr->token, FLOATLIT, std::to_string(x / y));
    }
    clearNodeChildren(expr);
}

void Interpreter::negation(ASTnode* expr) {
    Token token = expr->childeren[0]->token;
    checkNumberOperand(token, "undefined behavior of unary expression");
    if(expr->token.type == NOT) {
        switch(token.type) {
        case INTLIT:
            updateToken(expr->token, INTLIT, std::to_string(!stringToInt(token.text)));
            break;
        case FLOATLIT:
            updateToken(expr->token, FLOATLIT, std::to_string(!stringToDouble(token.text)));
            break;
        default:
            break;
        }
    }
    if(expr->token.type == MINUS) {
        switch(token.type) {
        case INTLIT:
            updateToken(expr->token, INTLIT, std::to_string(-stringToInt(token.text)));
        case FLOATLIT:
            updateToken(expr->token, FLOATLIT, std::to_string(-stringToDouble(token.text)));
        default:
            break;
        }
    }
    clearNodeChildren(expr);
}

void Interpreter::comparison(ASTnode* expr) {
    Token left = expr->childeren[0]->token;
    Token right = expr->childeren[1]->token;

    double x = stringToDouble(left.text), y = stringToDouble(right.text);

    switch(expr->token.type) {
    case GT:
        checkNumberOperands(left, right, "undefined behavior of '>' operator");
        updateToken(expr->token, INTLIT, std::to_string(x > y));
        break;
    case LT:
        checkNumberOperands(left, right, "undefined behavior of '<' operator");
        updateToken(expr->token, INTLIT, std::to_string(x < y));
        break;
    case GTEQ:
        checkNumberOperands(left, right, "undefined behavior of '>=' operator");
        updateToken(expr->token, INTLIT, std::to_string(x >= y));
        break;
    case LTEQ:
        checkNumberOperands(left, right, "undefined behavior '<=' operator");
        updateToken(expr->token, INTLIT, std::to_string(x <= y));
        break;
    case EQEQ:
        updateToken(expr->token, INTLIT, std::to_string(x == y));
        break;
    case NOTEQ:
        updateToken(expr->token, INTLIT, std::to_string(x != y));
        break;
    default:
        break;
    }

    clearNodeChildren(expr);
}

void Interpreter::ternary(ASTnode* expr) {
    Token left = expr->childeren[0]->token;
    Token middle = expr->childeren[1]->token;
    Token right = expr->childeren[2]->token;
    checkNumberOperand(left, "undefined behavior of '?' operator");

    double x = stringToDouble(left.text);
    if(x) {
        updateToken(expr->token, middle.type, middle.text);
    } else {
        updateToken(expr->token, right.type, right.text);
    }
    clearNodeChildren(expr);
}

void Interpreter::interpretNode(ASTnode* node) {

    for(size_t i = 0; i < node->childeren.size(); ++i) {
        Interpreter::interpretNode(node->childeren[i]);
    }

    switch(node->token.type) {
    case QMARK:
        Interpreter::ternary(node);
        return;
    case NOTEQ:
    case EQEQ:
    case GT:
    case GTEQ:
    case LT:
    case LTEQ:
        Interpreter::comparison(node);
        return;
    case MINUS:
        if(node->childeren.size() == 2) Interpreter::subtraction(node);
        else Interpreter::negation(node);
        return;
    case PLUS:
        Interpreter::addition(node);
        return;
    case SLASH:
        Interpreter::division(node);
        return;
    case STAR:
        Interpreter::multiplication(node);
        return;
    case NOT:
        Interpreter::negation(node);
        return;
    default:
        return;
    }
}

ASTnode* Interpreter::interpret() {
    try {
        Interpreter::interpretNode(Interpreter::ASTroot);
    } catch(const RuntimeError& error) {
        Interpreter::reportRuntimeError(error);
        return nullptr;
    }
    return Interpreter::ASTroot;
}
