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

void updateToken(Token*& token, const int& data) {
    size_t line = token->line;
    size_t collumn = token->collumn;
    delete token;
    token = new IntToken(INTLIT, "", line, collumn, data);
}

void updateToken(Token*& token, const double& data) {
    size_t line = token->line;
    size_t collumn = token->collumn;
    delete token;
    token = new FloatToken(FLOATLIT, "", line, collumn, data);
}

void updateToken(Token*& token, const std::string& data) {
    size_t line = token->line;
    size_t collumn = token->collumn;
    delete token;
    token = new StringToken(STRINGLIT, "", line, collumn, data);
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

bool Interpreter::checkIfIdentDeclaration(const ASTnode* ident) {
    if(ident->father == nullptr) {
        return false;
    }
    if(ident->father->token->type == LET) {
        return true;
    }
    return false;
}

void Interpreter::identifier(ASTnode* ident) {
    //check if it is a variable declaration

    if(Interpreter::checkIfIdentDeclaration(ident)) {
        if(Interpreter::identMap.find(ident->token->text) != Interpreter::identMap.end()) {
            throw RuntimeError(ident->token, "variable already declared");
        }
        if(ident->childeren.empty()) {
            Interpreter::identMap[ident->token->text] = new IntToken(INTLIT, "", 0, 0, 0);
        } else {
            Interpreter::identMap[ident->token->text] = ident->childeren[0]->token;
        }
        return;
    }

    //if not, check if it is an assignment

    if(!ident->childeren.empty()) {
        Interpreter::identMap[ident->token->text] = ident->childeren[0]->token;
        return;
    }

    //finally, if it is not one or the other, it means i just need its value for an operation

    Token* mappedToken = Interpreter::identMap.at(ident->token->text);
    if(dynamic_cast<IntToken*>(mappedToken)) {
        updateToken(ident->token, visitor.intValue(mappedToken));
    }
    if(dynamic_cast<FloatToken*>(mappedToken)) {
        updateToken(ident->token, visitor.floatValue(mappedToken));
    }
    if(dynamic_cast<StringToken*>(mappedToken)) {
        updateToken(ident->token, visitor.stringValue(mappedToken));
    }
}

void Interpreter::print(ASTnode* node) {
    switch(node->childeren[0]->token->type) {
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
        std::cout<<stateMap.at(node->childeren[0]->token->type)<<'\n';
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
            updateToken(expr->token, (int)(leftValue + visitor.intValue(right)));
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
            updateToken(expr->token, leftValue + std::to_string(visitor.intValue(right)));
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
            updateToken(expr->token, (int)(leftValue - visitor.intValue(right)));
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
            updateToken(expr->token, (int)(leftValue * visitor.intValue(right)));
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
            updateToken(expr->token, (int)(leftValue / visitor.intValue(right)));
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

    double x = 0, y = 1;
    if(left->type == INTLIT || left->type == FLOATLIT) {
        x = visitor.intValue(left);
        switch(right->type) {
        case INTLIT:
            y = visitor.intValue(right);
            break;
        case FLOATLIT:
            y = visitor.floatValue(right);
            break;
        case STRINGLIT:
            throw RuntimeError(left, "cannot perform comparison between string and number");
            break;
        default:
            break;
        }
    }

    if(left->type == STRINGLIT) {
        if(right->type != STRINGLIT) {
            throw RuntimeError(left, "cannot perform comparison between string and number");
        }
        if(visitor.stringValue(left) == visitor.stringValue(right)) {
            y = 0;
        }
        switch(expr->token->type) {
        case GT:
        case LT:
        case GTEQ:
        case LTEQ:
            throw RuntimeError(expr->token, "cannot perform 'grater than' or 'less than' comparisons on strings");
            break;
        default:
            break;
        }
    }

    switch(expr->token->type) {
    case GT:
        updateToken(expr->token, x > y);
        break;
    case LT:
        updateToken(expr->token, x < y);
        break;
    case GTEQ:
        updateToken(expr->token, x >= y);
        break;
    case LTEQ:
        updateToken(expr->token, x <= y);
        break;
    case EQEQ:
        updateToken(expr->token, x == y);
        break;
    case NOTEQ:
        updateToken(expr->token, x != y);
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
    checkNumberOperand(left, "ternary operator can only be used on numbers");

    double x;
    switch(left->type) {
    case INTLIT:
        x = visitor.intValue(left);
        break;
    case FLOATLIT:
        x = visitor.floatValue(left);
        break;
    default:
        break;
    }
    if(x) {
        switch(middle->type) {
        case INTLIT:
            updateToken(expr->token, visitor.intValue(middle));
            break;
        case FLOATLIT:
            updateToken(expr->token, visitor.floatValue(middle));
            break;
        case STRINGLIT:
            updateToken(expr->token, visitor.stringValue(middle));
            break;
        default:
            break;
        }
    } else {
        switch(right->type) {
        case INTLIT:
            updateToken(expr->token, visitor.intValue(right));
            break;
        case FLOATLIT:
            updateToken(expr->token, visitor.floatValue(right));
            break;
        case STRINGLIT:
            updateToken(expr->token, visitor.stringValue(right));
            break;
        default:
            break;

        }
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
