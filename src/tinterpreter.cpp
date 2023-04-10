#include<tinterpreter.h>
#include<sstream>
#include<iostream>

/** utilities **/

void clearNodeChildren(ASTnode* node) {
    while(!node->childeren.empty()) {
        delete node->childeren.back();
        node->childeren.pop_back();
    }
}

void updateToken(Token* token, const int& data) {
    size_t line = token->line;
    size_t collumn = token->collumn;

    token = new IntToken(INTLIT, "", line, collumn, data);
}

void updateToken(Token* token, const double& data) {
    size_t line = token->line;
    size_t collumn = token->collumn;

    token = new FloatToken(FLOATLIT, "", line, collumn, data);
}

void updateToken(Token* token, const std::string& data) {
    size_t line = token->line;
    size_t collumn = token->collumn;

    token = new StringToken(STRINGLIT, "", line, collumn, data);
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

void checkNumberOperand(Token* token, std::string errorMsg) {
    switch(token->type) {
    case INTLIT:
    case FLOATLIT:
        break;
    default:
        throw RuntimeError(token, errorMsg);
    }
}

void checkNumberOperands(Token* left, Token* right, std::string errorMsg) {
    switch(left->type) {
    case INTLIT:
    case FLOATLIT:
        break;
    default:
        throw RuntimeError(left, errorMsg);
    }
    switch(right->type) {
    case INTLIT:
    case FLOATLIT:
        break;
    default:
        throw RuntimeError(right, errorMsg);
    }
}

Visitor visitor;

/** end of utilities **/

RuntimeError::RuntimeError(Token* token, const std::string& message) {
    RuntimeError::token = token;
    RuntimeError::message = message;
}

void Interpreter::reportRuntimeError(const RuntimeError& error) {
    std::cout<<"[ERROR] at line "<<error.token->line<<" collumn "<<error.token->collumn<<" at '"<<error.token->text<<"': Runtime error: "<<error.message<<"\n";
}

Interpreter::Interpreter(std::vector<ASTnode*> stmtList) {
    Interpreter::stmtList = stmtList;
}

bool Interpreter::checkIfIdentDeclared(Token* ident) {
    if(Interpreter::identMap.find(ident->text) != Interpreter::identMap.end()) {
        return true;
    }
    throw RuntimeError(ident, "'" + ident->text + "' was not declared in this scope");
    return false;
}

bool Interpreter::checkIfIdentDeclaration(const ASTnode* ident) {
    if(ident->father == nullptr) {
        return false;
    }
    switch(ident->father->token->type) {
    case INT:
    case BOOL:
    case CHAR:
    case FLOAT:
    case STRING:
        return true;
    default:
        return false;
    }
}

void Interpreter::identifier(ASTnode* ident) {
    /*if(ident->childeren.empty() && Interpreter::checkIfIdentDeclared(ident->token)) {
        updateToken(ident->token, Interpreter::identMap[ident->token->text]->type, Interpreter::identMap[ident->token->text]->text);
        return;
    }
    Interpreter::identMap[ident->token->text] = ident->childeren[0]->token;*/
}

void Interpreter::print(ASTnode* node) {
    switch(node->token->type) {
    case INTLIT:
        std::cout<<visitor.intValue(node->childeren[0]->token)<<'\n';
        break;
    case FLOATLIT:
        std::cout<<visitor.floatValue(node->childeren[0]->token)<<'\n';
        break;
    case STRINGLIT:
        std::cout<<visitor.stringValue(node->childeren[0]->token)<<'\n';
        break;
    default:
        std::cout<<node->childeren[0]->token->text<<'\n';
        break;
    }
}

void Interpreter::addition(ASTnode* expr) {
    Token* left = expr->childeren[0]->token;
    Token* right = expr->childeren[1]->token;

    if(left->type == INTLIT) {
        int leftValue = visitor.intValue(left);
        switch(right->type) {
        case INTLIT:
            updateToken(expr->token, leftValue + visitor.intValue(right));
            break;
        case FLOATLIT:
            updateToken(expr->token, leftValue + visitor.floatValue(right));
            break;
        case STRINGLIT:
            updateToken(expr->token, std::to_string(leftValue) + visitor.stringValue(right));
            break;
        default:
            break;
        }
    }

    if(left->type == FLOATLIT) {
        double leftValue = visitor.floatValue(left);
        switch(right->type) {
        case INTLIT:
            updateToken(expr->token, leftValue + visitor.intValue(right));
            break;
        case FLOATLIT:
            updateToken(expr->token, leftValue + visitor.floatValue(right));
            break;
        case STRINGLIT:
            updateToken(expr->token, std::to_string(leftValue) + visitor.stringValue(right));
            break;
        default:
            break;
        }
    }

    if(left->type == STRINGLIT) {
        std::string leftValue = visitor.stringValue(left);
        switch(right->type) {
        case INTLIT:
            updateToken(expr->token, leftValue + std::to_string(visitor.floatValue(right)));
            break;
        case FLOATLIT:
            updateToken(expr->token, leftValue + std::to_string(visitor.floatValue(right)));
            break;
        case STRINGLIT:
            updateToken(expr->token, leftValue + visitor.stringValue(right));
            break;
        default:
            break;
        }
    }

    clearNodeChildren(expr);
}

void Interpreter::subtraction(ASTnode* expr) {
    Token* left = expr->childeren[0]->token;
    Token* right = expr->childeren[1]->token;
    checkNumberOperands(left, right, "subtraction can only be performed between numbers");

    if(left->type == INTLIT) {
        int leftValue = visitor.intValue(left);
        switch(right->type) {
        case INTLIT:
            updateToken(expr->token, leftValue - visitor.intValue(right));
            break;
        case FLOATLIT:
            updateToken(expr->token, leftValue - visitor.floatValue(right));
            break;
        default:
            break;
        }
    }

    if(left->type == FLOATLIT) {
        double leftValue = visitor.floatValue(left);
        switch(right->type) {
        case INTLIT:
            updateToken(expr->token, leftValue - visitor.intValue(right));
            break;
        case FLOATLIT:
            updateToken(expr->token, leftValue - visitor.floatValue(right));
            break;
        default:
            break;
        }
    }

    clearNodeChildren(expr);
}

void Interpreter::multiplication(ASTnode* expr) {
    Token* left = expr->childeren[0]->token;
    Token* right = expr->childeren[1]->token;
    checkNumberOperands(left, right, "multiplication can only be performed between numbers");

    if(left->type == INTLIT) {
        int leftValue = visitor.intValue(left);
        switch(right->type) {
        case INTLIT:
            updateToken(expr->token, leftValue * visitor.intValue(right));
            break;
        case FLOATLIT:
            updateToken(expr->token, leftValue * visitor.floatValue(right));
            break;
        default:
            break;
        }
    }

    if(left->type == FLOATLIT) {
        double leftValue = visitor.floatValue(left);
        switch(right->type) {
        case INTLIT:
            updateToken(expr->token, leftValue * visitor.intValue(right));
            break;
        case FLOATLIT:
            updateToken(expr->token, leftValue * visitor.floatValue(right));
            break;
        default:
            break;
        }
    }
    clearNodeChildren(expr);
}

void Interpreter::division(ASTnode* expr) {
    Token* left = expr->childeren[0]->token;
    Token* right = expr->childeren[1]->token;
    checkNumberOperands(left, right, "division can only be performed between numbers");

    if(left->type == INTLIT) {
        int leftValue = visitor.intValue(left);
        switch(right->type) {
        case INTLIT:
            updateToken(expr->token, leftValue / visitor.intValue(right));
            break;
        case FLOATLIT:
            updateToken(expr->token, leftValue / visitor.floatValue(right));
            break;
        default:
            break;
        }
    }

    if(left->type == FLOATLIT) {
        double leftValue = visitor.floatValue(left);
        switch(right->type) {
        case INTLIT:
            updateToken(expr->token, leftValue / visitor.intValue(right));
            break;
        case FLOATLIT:
            updateToken(expr->token, leftValue / visitor.floatValue(right));
            break;
        default:
            break;
        }
    }
    clearNodeChildren(expr);
}

void Interpreter::negation(ASTnode* expr) {
    Token* token = expr->childeren[0]->token;
    checkNumberOperand(token, "undefined behavior of unary expression");
    if(expr->token->type== NOT) {
        switch(token->type) {
        case INTLIT:
            updateToken(expr->token, !visitor.intValue(token));
            break;
        case FLOATLIT:
            updateToken(expr->token, !visitor.floatValue(token));
            break;
        default:
            break;
        }
    }
    if(expr->token->type== MINUS) {
        switch(token->type) {
        case INTLIT:
            updateToken(expr->token, -visitor.intValue(token));
            break;
        case FLOATLIT:
            updateToken(expr->token, -visitor.floatValue(token));
            break;
        default:
            break;
        }
    }
    clearNodeChildren(expr);
}

void Interpreter::comparison(ASTnode* expr) {
    Token* left = expr->childeren[0]->token;
    Token* right = expr->childeren[1]->token;

    double x = stringToDouble(left->text), y = stringToDouble(right->text);

    switch(expr->token->type) {
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
    Token* left = expr->childeren[0]->token;
    Token* middle = expr->childeren[1]->token;
    Token* right = expr->childeren[2]->token;
    checkNumberOperand(left, "undefined behavior of '?' operator");

    double x = stringToDouble(left->text);
    if(x) {
        updateToken(expr->token, middle->type, middle->text);
    } else {
        updateToken(expr->token, right->type, right->text);
    }
    clearNodeChildren(expr);
}

void Interpreter::interpretNode(ASTnode* node) {

    for(size_t i = 0; i < node->childeren.size(); ++i) {
        Interpreter::interpretNode(node->childeren[i]);
    }

    switch(node->token->type) {
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
    case PRINT:
        Interpreter::print(node);
        return;
    case IDENT:
        Interpreter::identifier(node);
    default:
        return;
    }
}

void Interpreter::interpret() {
    try {
        for(size_t i = 0; i < Interpreter::stmtList.size(); ++i) {
            Interpreter::interpretNode(Interpreter::stmtList[i]);
        }
    } catch(const RuntimeError& error) {
        Interpreter::reportRuntimeError(error);
        return;
    }
}