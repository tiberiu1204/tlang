#include<tinterpreter.h>
#include<sstream>
#include<iostream>

/** utilities **/

bool isNumber(Object* obj) {
    if(obj->instanceof(STRING)) {
        return false;
    }
    return true;
}

bool isVarDecl(ASTnode* node) {
    if(node->father == nullptr) {
        return false;
    }
    if(node->father->token.type == LET) {
        return true;
    }
    return false;
}

Object* newObject(Object* other) {
    if(other->instanceof(NUMBER)) {
        return new Obj<double>(other);
    }
    return new Obj<std::string>(other);
}

/** end of utilities **/

RuntimeError::RuntimeError(Token token, const std::string& message) {
    RuntimeError::token = token;
    RuntimeError::message = message;
}

void Interpreter::reportRuntimeError(const RuntimeError& error) {
    std::cout<<"[ERROR] at line "<<error.token.line<<" collumn "<<error.token.collumn<<" at '"<<error.token.text<<"': Runtime error: "<<error.message<<"\n";
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

Object* Interpreter::primary(ASTnode* node) {
    return newObject(node->token.value);
}

Object* Interpreter::identifier(ASTnode* node) {

    //check if it is a variable declaration

    if(isVarDecl(node)) {
        if(Interpreter::identMap.find(node->token.text) != Interpreter::identMap.end()) {
            throw RuntimeError(node->token, "variable already declared");
        }
        if(node->childeren.empty()) {
            Interpreter::identMap[node->token.text] = new Obj<double>(NUMBER, 0);
            return new Obj<double>(NUMBER, 0);
        } else {
            Object* value = Interpreter::interpretNode(node->childeren[0]);
            Interpreter::identMap[node->token.text] = value;
            return newObject(value);
        }
    }

    //if not, check if it is an assignment

    if(!node->childeren.empty()) {
        Object* value = Interpreter::interpretNode(node->childeren[0]);
        delete Interpreter::identMap[node->token.text];
        Interpreter::identMap[node->token.text] = value;
        return newObject(value);
    }

    //finally, it must be just a table lookup

    return newObject(Interpreter::identMap[node->token.text]);
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
    Object* value = Interpreter::interpretNode(node->childeren[0]);
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
            throw RuntimeError(node->token, "cannot perfrom '==' comparison between different types");
        }
        result = getValue<double>(left) == getValue<double>(right);
        break;
    case NOTEQ:
        if(left->type != right->type) {
            throw RuntimeError(node->token, "cannot perfrom '!=' comparison between different types");
        }
        result = getValue<double>(left) != getValue<double>(right);
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
        result = newObject(middle);
    } else {
        result = newObject(right);
    }
    delete left;
    delete middle;
    delete right;
    return result;
}

Object* Interpreter::interpretNode(ASTnode* node) {
    Object* result;
    switch(node->token.type) {
    case INTLIT:
    case FLOATLIT:
    case STRINGLIT:
        result = Interpreter::primary(node);
        break;
    case PLUS:
        result = Interpreter::addition(node);
        break;
    case MINUS:
        if(node->childeren.size() == 1) {
            result = Interpreter::negation(node);
        }
        else {
            result = Interpreter::subtraction(node);
        }
        break;
    case STAR:
        result = Interpreter::multiplication(node);
        break;
    case SLASH:
        result = Interpreter::division(node);
        break;
    case NOT:
        result = Interpreter::negation(node);
        break;
    case GT:
    case LT:
    case LTEQ:
    case GTEQ:
    case EQEQ:
    case NOTEQ:
        result = Interpreter::comparison(node);
        break;
    case QMARK:
        result = ternary(node);
        break;
    case PRINT:
        Interpreter::print(node);
        result = nullptr;
        break;
    case IDENT:
        result = Interpreter::identifier(node);
        break;
    case LET:
        result = Interpreter::interpretNode(node->childeren[0]);
        break;
    default:
        result = nullptr;
        break;
    }
    delete node;
    return result;
}

void Interpreter::interpret() {
    try {
        for(size_t i = 0; i < Interpreter::stmtList.size(); ++i) {
            Object* interpretedStatement = Interpreter::interpretNode(Interpreter::stmtList[i]);
            delete interpretedStatement;
        }
    } catch(const RuntimeError& error) {
        Interpreter::reportRuntimeError(error);
    }
}
