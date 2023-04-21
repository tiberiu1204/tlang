#include<tinterpreter.h>
#include<sstream>
#include<iostream>

size_t minValidArgumentCount(ASTnode* func) {
    size_t result = func->childeren.size() - 1;
    for(size_t i = func->childeren.size() - 2; i >= 0; --i) {
        if(func->childeren[i]->token.value != nullptr) {
            result--;
        } else {
            break;
        }
    }
    return result;
}

bool isVaidCall(ASTnode* node) {
    switch(node->token.type) {
    case IDENT:
        return true;
    default:
        return false;
    }
}

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
    case FUNC:
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
    scopes->pop_back();
}

void Interpreter::clearScope(std::unordered_set<std::string> omit) {
    for(const auto& variable : scopes->back()) {
        if(omit.find(variable.first) == omit.end()) {
            scopes->back().erase(variable.first);
        }
    }
}

std::unique_ptr<Object>* Interpreter::getVariable(const std::string& name) {
    for(size_t i = scopes->size() - 1; i >= 0; --i) {
        if(scopes->at(i).find(name) != scopes->at(i).end()) {
            return &scopes->at(i)[name];
        }
    }
    return nullptr;
}

Interpreter::Interpreter(std::vector<ASTnode*> stmtList) {
    Interpreter::stmtList = stmtList;
}

void Interpreter::print(ASTnode* node) {
    std::unique_ptr<Object> value = interpretNode(node->childeren[0]);
    if(value->instanceof(STRING)) {
        std::cout<<getValue<std::string>(value.get())<<"\n";
        return;
    }
    if(value->instanceof(NUMBER)) {
        std::cout<<getValue<double>(value.get())<<"\n";
        return;
    }
    std::cout<<"[DEBUG] trying to print object of unknown type";
}

void Interpreter::executeBlock(ASTnode* block) {
    if(isLoneBlock(block)) {
        scopes->push_back(Scope());
    }
    for(size_t i = 0; i < block->childeren.size(); ++i) {
        interpretNode(block->childeren[i]);
    }
    if(isLoneBlock(block)) {
        popScope();
    }
}

void Interpreter::exprStmt(ASTnode* node) {
    for(size_t i = 0; i < node->childeren.size(); ++i) {
         interpretNode(node->childeren[i]);
    }
}

void Interpreter::ifStmt(ASTnode* node) {
    scopes->push_back(Scope());
    std::unique_ptr<Object> condition = interpretNode(node->childeren[0]);
    ASTnode* nodeToExecute;
    if(isTruthy(condition.get())) {
        nodeToExecute = node->childeren[1];
    } else {
        if(node->childeren.size() == 2) return;
        nodeToExecute = node->childeren[2];
    }
    interpretNode(nodeToExecute);
    popScope();
}

void Interpreter::whileStmt(ASTnode* node) {
    scopes->push_back(Scope());
    std::unique_ptr<Object> condition = interpretNode(node->childeren[0]);
    try {
        while(isTruthy(condition.get())) {
            try {
                interpretNode(node->childeren[1]);
            } catch(const ContinueStmt& stmt) {}
            popScope();
            scopes->push_back(Scope());
            condition = interpretNode(node->childeren[0]);
        }
    } catch(const BreakStmt& stmt) {
        popScope();
        return;
    }
    popScope();
}

void Interpreter::forStmt(ASTnode* node) {
    scopes->push_back(Scope());

    //execute initial statement

    interpretNode(node->childeren[0]);
    std::unique_ptr<Object> condition = interpretNode(node->childeren[1]);

    //store initial variable declarations in a set to be omitted when scope is cleared

    std::unordered_set<std::string> omit;
    for(const auto& element : scopes->back()) {
        omit.insert(element.first);
    }

    try {
        while(isTruthy(condition.get())) {
            //execute body

            try {
                interpretNode(node->childeren[3]);
            } catch(const ContinueStmt& stmt) {}

            //execute increment statement

            interpretNode(node->childeren[2]);

            //clear scope except variables declared in initial statement

            clearScope(omit);

            //re-interpret condition

            condition = interpretNode(node->childeren[1]);
        }
    } catch(const BreakStmt& stmt) {
        popScope();
        return;
    }
    popScope();
}

std::unique_ptr<Object> Interpreter::call(ASTnode* node) {

    //node is CALL, child of node is callee, child of callee(if any) is COMA, which means expression block
    //in this case, expression block is the argument block

    ASTnode* callee = node->childeren[0];
    ASTnode* argumentBlock = nullptr;
    if(!node->childeren[0]->childeren.empty()) {
        argumentBlock = node->childeren[0]->childeren[0];
    }
    if(!isVaidCall(callee)) {
        throw RuntimeError(callee->token, "can only call functions and classes");
    }
    size_t argumentCount = argumentBlock ? argumentBlock->childeren.size() : 0;
    Object* argumentList[argumentCount];
    for(size_t i = 0; i < argumentCount; ++i) {
        ASTnode* expr = argumentBlock->childeren[i];
        argumentList[i] = copyObject(interpretNode(expr).get());
    }

    return callFunciton(callee, argumentList, argumentCount);
}

std::unique_ptr<Object> Interpreter::callFunciton(ASTnode* callee, Object* arguments[], const size_t& argumentCount) {
    ASTnode* func = functions[callee->token.text];
    if(argumentCount < minValidArgumentCount(func)) {
        throw RuntimeError(callee->token, "no function of this name matches number of arguments introduced");
    }

    //creating new scope list for call, saving global variables to new scope

    std::vector<Scope>* prevSopeList = scopes;
    std::vector<Scope> newScopeList;
    newScopeList.push_back(Scope());
    for(const auto& element : scopes->front()) {
        //element.first = identifier name, element.second = unique_ptr<Object>, which means identifier's stored value
        newScopeList[0][element.first] = std::unique_ptr<Object>(copyObject(element.second.get()));
    }
    newScopeList.push_back(Scope());

    //changing where main scopes list points to

    scopes = &newScopeList;

    //adding arguments to scope

    for(size_t i = 0; i < func->childeren.size() - 2; ++i) {
        ASTnode* argument = func->childeren[i];
        if(i < argumentCount) {
            scopes->back()[argument->token.text] = std::unique_ptr<Object>(copyObject(arguments[i]));
        } else {
            Object* defaultArgument = copyObject(func->childeren[i]->token.value);
            scopes->back()[argument->token.text] = std::unique_ptr<Object>(defaultArgument);
        }
    }

    std::unique_ptr<Object> result = interpretNode(func->childeren.back());

    //reversing where scopes list and cleaning up

    scopes = prevSopeList;
    delete[] arguments;
    return result;
}

std::unique_ptr<Object> Interpreter::primary(ASTnode* node) {
    return std::unique_ptr<Object>(copyObject(node->token.value));
}

std::unique_ptr<Object> Interpreter::varDecl(ASTnode* node) {
    for(size_t i = 0; i < node->childeren.size(); ++i) {
        ASTnode* ident = node->childeren[i];
        if(scopes->back().find(ident->token.text) != scopes->back().end()) {
            throw RuntimeError(ident->token, "variable already declared in this scope");
        }
        if(ident->childeren.empty()) {
            scopes->back()[ident->token.text] = std::unique_ptr<Object>(new Obj<double>(NUMBER, 0));
        } else {
            scopes->back()[ident->token.text] = interpretNode(ident->childeren[0]);
        }
    }
    return std::unique_ptr<Object>(copyObject(scopes->back()[node->childeren.back()->token.text].get()));
}

std::unique_ptr<Object> Interpreter::identifier(ASTnode* node) {

    //check if variable declared at all
    std::unique_ptr<Object>* storedVariable = getVariable(node->token.text);
    if(storedVariable == nullptr) {
        throw RuntimeError(node->token, "unknown identifier (variable not declared)");
    }

    //if no error, check if it is an assignment

    if(!node->childeren.empty()) {
        *storedVariable = std::unique_ptr<Object>(interpretNode(node->childeren[0]));
        return std::unique_ptr<Object>(copyObject(storedVariable->get()));
    }

    //finally, it must be just a table lookup

    return std::unique_ptr<Object>(copyObject(storedVariable->get()));
}

std::unique_ptr<Object> Interpreter::addition(ASTnode* node) {
    std::unique_ptr<Object> left = interpretNode(node->childeren[0]);
    std::unique_ptr<Object> right = interpretNode(node->childeren[1]);

    if(left->instanceof(NUMBER)) {
        if(right->instanceof(STRING)) {
            std::string result = std::to_string(getValue<double>(left.get())) + getValue<std::string>(right.get());
            return std::unique_ptr<Object>(new Obj<std::string>(STRING, result));
        } else {
            double result = getValue<double>(left.get()) + getValue<double>(right.get());
            return std::unique_ptr<Object>(new Obj<double>(NUMBER, result));
        }
    } else {
        if(right->instanceof(STRING)) {
            std::string result = getValue<std::string>(left.get()) + getValue<std::string>(right.get());
            return std::unique_ptr<Object>(new Obj<std::string>(STRING, result));
        } else {
            std::string result = getValue<std::string>(left.get()) + std::to_string(getValue<double>(right.get()));
            return std::unique_ptr<Object>(new Obj<std::string>(STRING, result));
        }
    }
}

std::unique_ptr<Object> Interpreter::subtraction(ASTnode* node) {
    std::unique_ptr<Object> left = interpretNode(node->childeren[0]);
    std::unique_ptr<Object> right = interpretNode(node->childeren[1]);
    if(!isNumber(left.get()) || !isNumber(right.get())) {
        throw RuntimeError(node->token, "subtraction can only be performed between numbers");
    }
    double result = getValue<double>(left.get()) - getValue<double>(right.get());
    return std::unique_ptr<Object>(new Obj<double>(NUMBER, result));
}

std::unique_ptr<Object> Interpreter::multiplication(ASTnode* node) {
    std::unique_ptr<Object> left = interpretNode(node->childeren[0]);
    std::unique_ptr<Object> right = interpretNode(node->childeren[1]);
    if(!isNumber(left.get()) || !isNumber(right.get())) {
        throw RuntimeError(node->token, "multiplication can only be performed between numbers");
    }
    double result = getValue<double>(left.get()) * getValue<double>(right.get());
    return std::unique_ptr<Object>(new Obj<double>(NUMBER, result));
}

std::unique_ptr<Object> Interpreter::division(ASTnode* node) {
    std::unique_ptr<Object> left = interpretNode(node->childeren[0]);
    std::unique_ptr<Object> right = interpretNode(node->childeren[1]);
    if(!isNumber(left.get()) || !isNumber(right.get())) {
        throw RuntimeError(node->token, "division can only be performed between numbers");
    }
    double result = getValue<double>(left.get()) / getValue<double>(right.get());
    return std::unique_ptr<Object>(new Obj<double>(NUMBER, result));
}

std::unique_ptr<Object> Interpreter::negation(ASTnode* node) {
    std::unique_ptr<Object> value = interpretNode(node->childeren[0]);
    if(!isNumber(value.get())) {
        throw RuntimeError(node->token, "cannot negate non-number");
    }
    double result;
    switch(node->token.type) {
    case NOT:
        result = !getValue<double>(value.get());
        break;
    case MINUS:
        result = -getValue<double>(value.get());
        break;
    default:
        break;
    }
    return std::unique_ptr<Object>(new Obj<double>(NUMBER, result));
}

std::unique_ptr<Object> Interpreter::comparison(ASTnode* node) {
    std::unique_ptr<Object> left = interpretNode(node->childeren[0]);
    std::unique_ptr<Object> right = interpretNode(node->childeren[1]);
    double result;
    switch(node->token.type) {
    case GT:
        if(!isNumber(left.get()) || !isNumber(right.get())) {
            throw RuntimeError(node->token, "can only perform '>' comparison between two numbers");
        }
        result = getValue<double>(left.get()) > getValue<double>(right.get());
        break;
    case LT:
        if(!isNumber(left.get()) || !isNumber(right.get())) {
            throw RuntimeError(node->token, "can only perform '<' comparison between two numbers");
        }
        result = getValue<double>(left.get()) < getValue<double>(right.get());
        break;
    case GTEQ:
        if(!isNumber(left.get()) || !isNumber(right.get())) {
            throw RuntimeError(node->token, "can only perform '>=' comparison between two numbers");
        }
        result = getValue<double>(left.get()) >= getValue<double>(right.get());
        break;
    case LTEQ:
        if(!isNumber(left.get()) || !isNumber(right.get())) {
            throw RuntimeError(node->token, "can only perform '<=' comparison between two numbers");
        }
        result = getValue<double>(left.get()) <= getValue<double>(right.get());
        break;
    case EQEQ:
        if(left->type != right->type) {
            result = 0;
        } else {
            if(left->instanceof(NUMBER)) {
                result = getValue<double>(left.get()) == getValue<double>(right.get());
            }
            if(left->instanceof(STRING)) {
                result = getValue<std::string>(left.get()) == getValue<std::string>(right.get());
            }
        }
        break;
    case NOTEQ:
        if(left->type != right->type) {
            result = 1;
        }
        else {
            if(left->instanceof(NUMBER)) {
                result = getValue<double>(left.get()) != getValue<double>(right.get());
            }
            if(left->instanceof(STRING)) {
                result = getValue<std::string>(left.get()) != getValue<std::string>(right.get());
            }
        }
        break;
    default:
        break;
    }
    return std::unique_ptr<Object>(new Obj<double>(NUMBER, result));
}

std::unique_ptr<Object> Interpreter::ternary(ASTnode*node ) {
    std::unique_ptr<Object> left = interpretNode(node->childeren[0]);
    std::unique_ptr<Object> middle = interpretNode(node->childeren[1]);
    std::unique_ptr<Object> right = interpretNode(node->childeren[2]);

    if(!isNumber(left.get())) {
        throw RuntimeError(node->token, "ternary '?' operator can only be used on numbers");
    }
    if(getValue<double>(left.get())) {
        return std::unique_ptr<Object>(copyObject(middle.get()));
    }
    return std::unique_ptr<Object>(copyObject(right.get()));
}

std::unique_ptr<Object> Interpreter::logic_and(ASTnode* node) {
    std::unique_ptr<Object> left = interpretNode(node->childeren[0]);
    std::unique_ptr<Object> right = interpretNode(node->childeren[1]);

    if(isTruthy(left.get()) && isTruthy(right.get())) {
        return std::unique_ptr<Object>(new Obj<double>(NUMBER, 1));
    }
    return std::unique_ptr<Object>(new Obj<double>(NUMBER, 0));
}

std::unique_ptr<Object> Interpreter::logic_or(ASTnode* node) {
    std::unique_ptr<Object> left = interpretNode(node->childeren[0]);
    std::unique_ptr<Object> right = interpretNode(node->childeren[1]);

    if(isTruthy(left.get()) || isTruthy(right.get())) {
        return std::unique_ptr<Object>(new Obj<double>(NUMBER, 1));
    }
    return std::unique_ptr<Object>(new Obj<double>(NUMBER, 0));
}

std::unique_ptr<Object> Interpreter::interpretNode(ASTnode* node) {
    if(node == nullptr) {
        return nullptr;
    }
    std::unique_ptr<Object> o;
    switch(node->token.type) {
    case FLOATLIT:
    case STRINGLIT:
        return primary(node);
    case PLUS:
        return addition(node);
    case MINUS:
        if(node->childeren.size() == 1) {
            return negation(node);
        }
        return subtraction(node);
    case STAR:
        return multiplication(node);
    case SLASH:
        return division(node);
    case NOT:
        return negation(node);
    case GT:
    case LT:
    case LTEQ:
    case GTEQ:
    case EQEQ:
    case NOTEQ:
        return comparison(node);
    case QMARK:
        return ternary(node);
    case PRINT:
        print(node);
        return nullptr;
    case IDENT:
        return identifier(node);
    case LET:
        return varDecl(node);
    case LBRACE:
        executeBlock(node);
        return nullptr;
    case COMA:
        exprStmt(node);
        return nullptr;
    case ANDAND:
        return logic_and(node);
    case OROR:
        return logic_or(node);
    case IF:
        ifStmt(node);
        return nullptr;
    case WHILE:
        whileStmt(node);
        return nullptr;
    case FOR:
        forStmt(node);
        return nullptr;
    case CONTINUE:
        throw ContinueStmt();
    case BREAK:
        throw BreakStmt();
    case CALL:
        return call(node);
    default:
        return nullptr;
    }
}

void Interpreter::interpret() {
    if(stmtList[0] == nullptr) {
        return;
    }
    std::vector<Scope> mainScopeList;
    mainScopeList.push_back(Scope());
    scopes = &mainScopeList;

    try {
        for(size_t i = 0; i < Interpreter::stmtList.size(); ++i) {
            interpretNode(stmtList[i]);
        }
    } catch(const RuntimeError& error) {
        reportRuntimeError(error);
    }
}
