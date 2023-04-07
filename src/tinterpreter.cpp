#include<tinterpreter.h>
#include<sstream>

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
    if((left.type == right.type && right.type == FLOATLIT) ||
            (left.type == INTLIT && right.type == FLOATLIT) ||
            (left.type == FLOATLIT && right.type == INTLIT)) {
        double x = stringToDouble(left.text), y = stringToDouble(right.text);
        updateToken(expr->token, FLOATLIT, std::to_string(x + y));
    }
    clearNodeChildren(expr);
}

void Interpreter::subtraction(ASTnode* expr) {
    Token left = expr->childeren[0]->token;
    Token right = expr->childeren[1]->token;

    if(left.type == right.type && right.type == INTLIT) {
        int x = stringToInt(left.text), y = stringToInt(right.text);
        updateToken(expr->token, INTLIT, std::to_string(x - y));
    }
    if((left.type == right.type && right.type == FLOATLIT) ||
       (left.type == INTLIT && right.type == FLOATLIT) ||
       (left.type == FLOATLIT && right.type == INTLIT)) {
        double x = stringToDouble(left.text), y = stringToDouble(right.text);
        updateToken(expr->token, FLOATLIT, std::to_string(x - y));
    }
    clearNodeChildren(expr);
}

void Interpreter::multiplication(ASTnode* expr) {
    Token left = expr->childeren[0]->token;
    Token right = expr->childeren[1]->token;

    if(left.type == right.type && right.type == INTLIT) {
        int x = stringToInt(left.text), y = stringToInt(right.text);
        updateToken(expr->token, INTLIT, std::to_string(x * y));
    }
    if((left.type == right.type && right.type == FLOATLIT) ||
       (left.type == INTLIT && right.type == FLOATLIT) ||
       (left.type == FLOATLIT && right.type == INTLIT)) {
        double x = stringToDouble(left.text), y = stringToDouble(right.text);
        updateToken(expr->token, FLOATLIT, std::to_string(x * y));
    }
    clearNodeChildren(expr);
}

void Interpreter::division(ASTnode* expr) {
    Token left = expr->childeren[0]->token;
    Token right = expr->childeren[1]->token;

    if(left.type == right.type && right.type == INTLIT) {
        int x = stringToInt(left.text), y = stringToInt(right.text);
        updateToken(expr->token, INTLIT, std::to_string(x / y));
    }
    if((left.type == right.type && right.type == FLOATLIT) ||
       (left.type == INTLIT && right.type == FLOATLIT) ||
       (left.type == FLOATLIT && right.type == INTLIT)) {
        double x = stringToDouble(left.text), y = stringToDouble(right.text);
        updateToken(expr->token, FLOATLIT, std::to_string(x / y));
    }
    clearNodeChildren(expr);
}

void Interpreter::negation(ASTnode* expr) {
    Token token = expr->childeren[0]->token;
    if(expr->token.type == NOT) {
        if(token.type == FLOATLIT) {
            double x = stringToDouble(token.text);
            updateToken(expr->token, FLOATLIT, std::to_string(!x));
        }
        if(token.type == INTLIT) {
            int x = stringToInt(token.text);
            updateToken(expr->token, INTLIT, std::to_string(!x));
        }
    }
    if(expr->token.type == MINUS) {
        if(token.type == FLOATLIT) {
            double x = stringToDouble(token.text);
            updateToken(expr->token, FLOATLIT, std::to_string(-x));
        }
        if(token.type == INTLIT) {
            int x = stringToInt(token.text);
            updateToken(expr->token, INTLIT, std::to_string(-x));
        }
    }
    clearNodeChildren(expr);
}

void Interpreter::comparison(ASTnode* expr) {
    Token left = expr->childeren[0]->token;
    Token right = expr->childeren[1]->token;

    double x = stringToDouble(left.text), y = stringToDouble(right.text);

    if(expr->token.type == GT) {
        updateToken(expr->token, INTLIT, std::to_string(x > y));
    }
    if(expr->token.type == LT) {
        updateToken(expr->token, INTLIT, std::to_string(x < y));
    }
    if(expr->token.type == GTEQ) {
        updateToken(expr->token, INTLIT, std::to_string(x >= y));
    }
    if(expr->token.type == LTEQ) {
        updateToken(expr->token, INTLIT, std::to_string(x <= y));
    }
    if(expr->token.type == EQEQ) {
        updateToken(expr->token, INTLIT, std::to_string(x == y));
    }
    if(expr->token.type == NOTEQ) {
        updateToken(expr->token, INTLIT, std::to_string(x != y));
    }
    clearNodeChildren(expr);
}

void Interpreter::ternary(ASTnode* expr) {
    Token left = expr->childeren[0]->token;
    Token middle = expr->childeren[1]->token;
    Token right = expr->childeren[2]->token;

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
    Interpreter::interpretNode(Interpreter::ASTroot);
    return Interpreter::ASTroot;
}
