#include<tinterpreter.h>
#include<sstream>
#include<iostream>

/** utilities **/

template < typename T >
T getValue(Object* obj) {
    return *(T*)(obj + 1);
}

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
}

Object* Interpreter::primary(ASTnode* node) {
    return node->token.value;
}

Object* Interpreter::identifier(ASTnode* node) {

    //check if it is a variable declaration

    if(isVarDecl(node)) {
        if(Interpreter::identMap.find(node->token.text) != Interpreter::identMap.end()) {
            throw RuntimeError(node->token, "variable already declared");
        }
        if(node->childeren.empty()) {
            Interpreter::identMap[node->token.text] = new Obj<double>(NUMBER, 0);
        } else {
            Interpreter::identMap[node->token.text] = Interpreter::interpretNode(node->childeren[0]);
        }
        return Interpreter::identMap[node->token.text];
    }

    //if not, check if it is an assignment

    if(!node->childeren.empty()) {
        Object* value = Interpreter::interpretNode(node->childeren[0]);

        Interpreter::identMap[node->token.text] = value;
        return value;
    }

    //finally, it must be just a table lookup

    return Interpreter::identMap[node->token.text];
}

Object* Interpreter::addition(ASTnode* node) {
    Object* left = interpretNode(node->childeren[0]);
    Object* right = interpretNode(node->childeren[1]);

    if(left->instanceof(NUMBER)) {
        if(right->instanceof(STRING)) {
            std::string result = std::to_string(getValue<double>(left)) + getValue<std::string>(right);
            return new Obj<std::string>(STRING, result);
        } else {
            double result = getValue<double>(left) + getValue<double>(right);
            return new Obj<double>(NUMBER, result);
        }
    } else {
        if(right->instanceof(STRING)) {
            std::string result = getValue<std::string>(left) + getValue<std::string>(right);
            return new Obj<std::string>(STRING, result);
        } else {
            std::string result = getValue<std::string>(left) + std::to_string(getValue<double>(right));
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
    return new Obj<double>(NUMBER, result);
}

Object* Interpreter::multiplication(ASTnode* node) {
    Object* left = interpretNode(node->childeren[0]);
    Object* right = interpretNode(node->childeren[1]);
    if(!isNumber(left) || !isNumber(right)) {
        throw RuntimeError(node->token, "multiplication can only be performed between numbers");
    }
    int result = getValue<double>(left) * getValue<double>(right);
    return new Obj<double>(NUMBER, result);
}

Object* Interpreter::division(ASTnode* node) {
    Object* left = interpretNode(node->childeren[0]);
    Object* right = interpretNode(node->childeren[1]);
    if(!isNumber(left) || !isNumber(right)) {
        throw RuntimeError(node->token, "division can only be performed between numbers");
    }
    double result = getValue<double>(left) / getValue<double>(right);
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
    return new Obj<double>(NUMBER, result);
}

Object* Interpreter::ternary(ASTnode*node ) {
    Object* left = interpretNode(node->childeren[0]);
    Object* middle = interpretNode(node->childeren[1]);
    Object* right = interpretNode(node->childeren[2]);
    if(!isNumber(left)) {
        throw RuntimeError(node->token, "ternary '?' operator can only be used on numbers");
    }
    if(getValue<double>(left)) {
        return middle;
    } else {
        return right;
    }
}

Object* Interpreter::interpretNode(ASTnode* node) {
    switch(node->token.type) {
    case INTLIT:
    case FLOATLIT:
    case STRINGLIT:
        return Interpreter::primary(node);
    case PLUS:
        return Interpreter::addition(node);
    case MINUS:
        if(node->childeren.size() == 1) {
            return Interpreter::negation(node);
        }
        return Interpreter::subtraction(node);
    case STAR:
        return Interpreter::multiplication(node);
    case SLASH:
        return Interpreter::division(node);
    case NOT:
        return Interpreter::negation(node);
    case GT:
    case LT:
    case LTEQ:
    case GTEQ:
    case EQEQ:
    case NOTEQ:
        return Interpreter::comparison(node);
    case QMARK:
        return ternary(node);
    case PRINT:
        Interpreter::print(node);
        return nullptr;
    case IDENT:
        return Interpreter::identifier(node);
    case LET:
        return Interpreter::interpretNode(node->childeren[0]);
    default:
        return nullptr;
    }
}

void Interpreter::interpret() {
    try {
        for(size_t i = 0; i < Interpreter::stmtList.size(); ++i) {
            Interpreter::interpretNode(Interpreter::stmtList[i]);
        }
    } catch(const RuntimeError& error) {
        Interpreter::reportRuntimeError(error);
    }
}
