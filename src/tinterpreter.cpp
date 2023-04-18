#include<tinterpreter.h>
#include<sstream>
#include<iostream>

bool isNumber(Object* obj) {
    if(obj->instanceof(STRING)) {
        return false;
    }
    return true;
}

bool isLoneBlock(ASTnode* node) {
    if(node->father == nullptr) {
        return true;
    }
    switch(node->father->token.type) {
    case IF:
    case FOR:
    case WHILE:
        return false;
    default:
        break;
    }
    return true;
}

bool isTruthy(Object* value) {
    if(value == nullptr) {
        return true;
    }
    if(value->instanceof(NUMBER)) {
        if(getValue<double>(value)) {
            return true;
        }
    }
    if(value->instanceof(STRING)) {
        if(getValue<std::string>(value) != "") {
            return true;
        }
    }
    return false;
}

Object* copyObject(Object* other) {
    if(other->instanceof(NUMBER)) {
        return new Obj<double>(NUMBER, getValue<double>(other));
    }
    if(other->instanceof(STRING)) {
        return new Obj<std::string>(STRING, getValue<std::string>(other));
    }
    std::cout<<"[DEBUG] returned nullptr at copyObject (other is of type "<<other->type<<")\n";
    return nullptr;
}

RuntimeError::RuntimeError(Token token, const std::string& message) {
    RuntimeError::token = token;
    RuntimeError::message = message;
}

void Interpreter::reportRuntimeError(const RuntimeError& error) {
    std::cout<<"[ERROR] at line "<<error.token.line<<" collumn "<<error.token.collumn<<" at '"<<error.token.text<<"': Runtime error: "<<error.message<<"\n";
}

void Interpreter::popScope() {
    for(auto variable : scopes.back()) {
        delete variable.second;
    }
    scopes.pop_back();
}

void Interpreter::clearScope(std::unordered_set<std::string> omit) {
    for(auto variable : scopes.back()) {
        if(omit.find(variable.first) == omit.end()) {
            delete variable.second;
            scopes.back().erase(variable.first);
        }
    }
}

Object** Interpreter::getVariable(const std::string& name) {
    for(int i = scopes.size() - 1; i >= 0; --i) {
        if(scopes[i].find(name) != scopes[i].end()) {
            return &scopes[i][name];
        }
    }
    return nullptr;
}

Interpreter::Interpreter(std::vector<ASTnode*> stmtList) {
    Interpreter::stmtList = stmtList;
}

void Interpreter::print(ASTnode* node) {
    Object* value = Interpreter::interpretNode(node->childeren[0]);
    if(value->instanceof(STRING)) {
        std::cout<<getValue<std::string>(value)<<"\n";
    } else {
        std::cout<<getValue<double>(value)<<"\n";
    }
    delete value;
}

void Interpreter::executeBlock(ASTnode* block) {
    if(isLoneBlock(block)) {
        scopes.push_back(Scope());
    }
    for(size_t i = 0; i < block->childeren.size(); ++i) {
        Object* interpretedStatement = interpretNode(block->childeren[i]);
        delete interpretedStatement;
    }
    if(isLoneBlock(block)) {
        popScope();
    }
}

void Interpreter::exprStmt(ASTnode* node) {
    for(size_t i = 0; i < node->childeren.size(); ++i) {
         delete interpretNode(node->childeren[i]);
    }
}

void Interpreter::ifStmt(ASTnode* node) {
    scopes.push_back(Scope());
    Object* condition = interpretNode(node->childeren[0]);
    ASTnode* nodeToExecute;
    if(isTruthy(condition)) {
        nodeToExecute = node->childeren[1];
    } else {
        if(node->childeren.size() == 2) return;
        nodeToExecute = node->childeren[2];
        delete node->childeren[1];
    }
    delete condition;
    delete interpretNode(nodeToExecute);
    popScope();
}

void Interpreter::whileStmt(ASTnode* node) {
    scopes.push_back(Scope());
    Object* condition = interpretNode(node->childeren[0]);
    while(isTruthy(condition)) {
        delete condition;
        delete interpretNode(node->childeren[1]);
        popScope();
        scopes.push_back(Scope());
        condition = interpretNode(node->childeren[0]);
    }
    popScope();
    delete condition;
}

void Interpreter::forStmt(ASTnode* node) {
    scopes.push_back(Scope());
    delete interpretNode(node->childeren[0]);
    Object* condition = interpretNode(node->childeren[1]);

    std::unordered_set<std::string> omit;
    for(auto element : scopes.back()) {
        omit.insert(element.first);
    }

    while(isTruthy(condition)) {
        delete condition;
        delete interpretNode(node->childeren[3]);
        delete interpretNode(node->childeren[2]);
        clearScope(omit);
        condition = interpretNode(node->childeren[1]);
    }

    popScope();
}

Object* Interpreter::primary(ASTnode* node) {
    return copyObject(node->token.value);
}

Object* Interpreter::varDecl(ASTnode* node) {
    for(size_t i = 0; i < node->childeren.size(); ++i) {
        ASTnode* ident = node->childeren[i];
        if(scopes.back().find(ident->token.text) != scopes.back().end()) {
            throw RuntimeError(ident->token, "variable already declared in this scope");
        }
        if(ident->childeren.empty()) {
            scopes.back()[ident->token.text] = new Obj<double>(NUMBER, 0);
        } else {
            Object* value = interpretNode(ident->childeren[0]);
            scopes.back()[ident->token.text] = value;
        }
    }
    return copyObject(scopes.back()[node->childeren.back()->token.text]);
}

Object* Interpreter::identifier(ASTnode* node) {

    //check if variable declared at all
    Object** storedVariable = getVariable(node->token.text);
    if(storedVariable == nullptr) {
        throw RuntimeError(node->token, "unknown identifier (variable not declared)");
    }

    //if no error, check if it is an assignment

    if(!node->childeren.empty()) {
        Object* temp = *storedVariable;
        *storedVariable = interpretNode(node->childeren[0]);
        delete temp;
        return copyObject(*storedVariable);
    }

    //finally, it must be just a table lookup

    return copyObject(*storedVariable);
}

Object* Interpreter::addition(ASTnode* node) {
    Object* left = interpretNode(node->childeren[0]);
    Object* right = interpretNode(node->childeren[1]);

    if(left->instanceof(NUMBER)) {
        if(right->instanceof(STRING)) {
            std::string result = std::to_string(getValue<double>(left)) + getValue<std::string>(right);
            delete left;
            delete right;
            return new Obj<std::string>(STRING, result);
        } else {
            double result = getValue<double>(left) + getValue<double>(right);
            delete left;
            delete right;
            return new Obj<double>(NUMBER, result);
        }
    } else {
        if(right->instanceof(STRING)) {
            std::string result = getValue<std::string>(left) + getValue<std::string>(right);
            delete left;
            delete right;
            return new Obj<std::string>(STRING, result);
        } else {
            std::string result = getValue<std::string>(left) + std::to_string(getValue<double>(right));
            delete left;
            delete right;
            return new Obj<std::string>(STRING, result);
        }
    }
}

Object* Interpreter::subtraction(ASTnode* node) {
    Object* left = interpretNode(node->childeren[0]);
    Object* right = interpretNode(node->childeren[1]);
    if(!isNumber(left) || !isNumber(right)) {
        throw RuntimeError(node->token, "subtraction can only be performed between numbers");
    }
    int result = getValue<double>(left) - getValue<double>(right);
    delete left;
    delete right;
    return new Obj<double>(NUMBER, result);
}

Object* Interpreter::multiplication(ASTnode* node) {
    Object* left = interpretNode(node->childeren[0]);
    Object* right = interpretNode(node->childeren[1]);
    if(!isNumber(left) || !isNumber(right)) {
        throw RuntimeError(node->token, "multiplication can only be performed between numbers");
    }
    int result = getValue<double>(left) * getValue<double>(right);
    delete left;
    delete right;
    return new Obj<double>(NUMBER, result);
}

Object* Interpreter::division(ASTnode* node) {
    Object* left = interpretNode(node->childeren[0]);
    Object* right = interpretNode(node->childeren[1]);
    if(!isNumber(left) || !isNumber(right)) {
        throw RuntimeError(node->token, "division can only be performed between numbers");
    }
    double result = getValue<double>(left) / getValue<double>(right);
    delete left;
    delete right;
    return new Obj<double>(NUMBER, result);
}

Object* Interpreter::negation(ASTnode* node) {
    Object* value = interpretNode(node->childeren[0]);
    if(!isNumber(value)) {
        throw RuntimeError(node->token, "cannot negate non-number");
    }
    double result;
    switch(node->token.type) {
    case NOT:
        result = !getValue<double>(value);
        break;
    case MINUS:
        result = -getValue<double>(value);
        break;
    default:
        break;
    }
    delete value;
    return new Obj<double>(NUMBER, result);
}

Object* Interpreter::comparison(ASTnode* node) {
    Object* left = interpretNode(node->childeren[0]);
    Object* right = interpretNode(node->childeren[1]);
    double result;
    switch(node->token.type) {
    case GT:
        if(!isNumber(left) || !isNumber(right)) {
            throw RuntimeError(node->token, "can only perform '>' comparison between two numbers");
        }
        result = getValue<double>(left) > getValue<double>(right);
        break;
    case LT:
        if(!isNumber(left) || !isNumber(right)) {
            throw RuntimeError(node->token, "can only perform '<' comparison between two numbers");
        }
        result = getValue<double>(left) < getValue<double>(right);
        break;
    case GTEQ:
        if(!isNumber(left) || !isNumber(right)) {
            throw RuntimeError(node->token, "can only perform '>=' comparison between two numbers");
        }
        result = getValue<double>(left) >= getValue<double>(right);
        break;
    case LTEQ:
        if(!isNumber(left) || !isNumber(right)) {
            throw RuntimeError(node->token, "can only perform '<=' comparison between two numbers");
        }
        result = getValue<double>(left) <= getValue<double>(right);
        break;
    case EQEQ:
        if(left->type != right->type) {
            result = 0;
        } else {
            if(left->instanceof(NUMBER)) {
                result = getValue<double>(left) == getValue<double>(right);
            }
            if(left->instanceof(STRING)) {
                result = getValue<std::string>(left) == getValue<std::string>(right);
            }
        }
        break;
    case NOTEQ:
        if(left->type != right->type) {
            result = 1;
        }
        else {
            if(left->instanceof(NUMBER)) {
                result = getValue<double>(left) == getValue<double>(right);
            }
            if(left->instanceof(STRING)) {
                result = getValue<std::string>(left) != getValue<std::string>(right);
            }
        }
        break;
    default:
        break;
    }
    delete left;
    delete right;
    return new Obj<double>(NUMBER, result);
}

Object* Interpreter::ternary(ASTnode*node ) {
    Object* left = interpretNode(node->childeren[0]);
    Object* middle = interpretNode(node->childeren[1]);
    Object* right = interpretNode(node->childeren[2]);

    Object* result;

    if(!isNumber(left)) {
        throw RuntimeError(node->token, "ternary '?' operator can only be used on numbers");
    }
    if(getValue<double>(left)) {
        result = copyObject(middle);
    } else {
        result = copyObject(right);
    }
    delete left;
    delete middle;
    delete right;
    return result;
}

Object* Interpreter::logic_and(ASTnode* node) {
    Object* left = interpretNode(node->childeren[0]);
    Object* right = interpretNode(node->childeren[1]);
    Object* result;

    if(isTruthy(left) && isTruthy(right)) {
        result = new Obj<double>(NUMBER, 1);
    } else {
        result = new Obj<double>(NUMBER, 0);
    }
    delete left;
    delete right;
    return result;
}

Object* Interpreter::logic_or(ASTnode* node) {
    Object* left = interpretNode(node->childeren[0]);
    Object* right = interpretNode(node->childeren[1]);
    Object* result;

    if(isTruthy(left) || isTruthy(right)) {
        result = new Obj<double>(NUMBER, 1);
    } else {
        result = new Obj<double>(NUMBER, 0);
    }
    delete left;
    delete right;
    return result;
}

Object* Interpreter::interpretNode(ASTnode* node) {
    if(node == nullptr) {
        return nullptr;
    }
    Object* result;
    switch(node->token.type) {
    case FLOATLIT:
    case STRINGLIT:
        result = primary(node);
        break;
    case PLUS:
        result = addition(node);
        break;
    case MINUS:
        if(node->childeren.size() == 1) {
            result = negation(node);
        }
        else {
            result = subtraction(node);
        }
        break;
    case STAR:
        result = multiplication(node);
        break;
    case SLASH:
        result = division(node);
        break;
    case NOT:
        result = negation(node);
        break;
    case GT:
    case LT:
    case LTEQ:
    case GTEQ:
    case EQEQ:
    case NOTEQ:
        result = comparison(node);
        break;
    case QMARK:
        result = ternary(node);
        break;
    case PRINT:
        print(node);
        result = nullptr;
        break;
    case IDENT:
        result = identifier(node);
        break;
    case LET:
        result = varDecl(node);
        break;
    case LBRACE:
        executeBlock(node);
        result = nullptr;
        break;
    case COMA:
        exprStmt(node);
        result = nullptr;
        break;
    case ANDAND:
        result = logic_and(node);
        break;
    case OROR:
        result = logic_or(node);
        break;
    case IF:
        ifStmt(node);
        result = nullptr;
        break;
    case WHILE:
        whileStmt(node);
        result = nullptr;
        break;
    case FOR:
        forStmt(node);
        result = nullptr;
        break;
    default:
        result = nullptr;
        break;
    }
    return result;
}

void Interpreter::interpret() {
    if(stmtList[0] == nullptr) {
        return;
    }
    scopes.push_back(Scope());
    try {
        for(size_t i = 0; i < Interpreter::stmtList.size(); ++i) {
            Object* interpretedStatement = Interpreter::interpretNode(Interpreter::stmtList[i]);
            delete interpretedStatement;
        }
    } catch(const RuntimeError& error) {
        Interpreter::reportRuntimeError(error);
    }
}
