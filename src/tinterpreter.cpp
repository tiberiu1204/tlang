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

UserFunction::UserFunction(const std::string& name, const std::vector<std::string>& parameters, ASTnode* body)
    : Function(name, parameters, body) {}

UserFunction* UserFunction::clone() {
    return new UserFunction(*this);
}

std::unique_ptr<Object> UserFunction::call(const std::vector<std::unique_ptr<Object> >& arguments, const Token& token, Interpreter* interpreter) {
    if(arguments.size() != this->arity()) {
        throw RuntimeError(token, "expected " + std::to_string((int)this->arity()) + " arguments, got " + std::to_string(arguments.size()));
    }
    size_t argumentIndex = 0;
    for(const std::string& paramName : m_Parameters) {
        Object* copiedObject = arguments[argumentIndex++].get()->clone();
        interpreter->scopes->back()[paramName] = std::unique_ptr<Object>(copiedObject);
    }
    return interpreter->interpretNode(m_Body);
}

void Interpreter::reportRuntimeError(const RuntimeError& error) {
    std::cout<<"[ERROR] at line "<<error.token.line<<" collumn "<<error.token.collumn<<" at '"<<error.token.text<<"': Runtime error: "<<error.message<<"\n";
}

void Interpreter::popScope() {
    scopes->pop_back();
}

void Interpreter::pushScope() {
    scopes->push_back(Scope());
}

void Interpreter::clearScope(std::unordered_set<std::string> omit) {
    for(const auto& variable : scopes->back()) {
        if(omit.find(variable.first) == omit.end()) {
            scopes->back().erase(variable.first);
        }
    }
}

std::unique_ptr<Object>* Interpreter::getObject(const std::string& name) {
    for(size_t i = scopes->size() - 1; ; --i) {
        if(scopes->at(i).find(name) != scopes->at(i).end()) {
            return &scopes->at(i)[name];
        }
        if(i == 0) break;
    }
    return nullptr;
}

Interpreter::Interpreter(std::vector<ASTnode*> stmtList) {
    Interpreter::stmtList = stmtList;
}

void Interpreter::print(ASTnode* node) {
    std::unique_ptr<Object> value = interpretNode(node->childeren[0]);
    if(value == nullptr) {
        std::cout<<"void\n";
        return;
    }
    if(value->instanceof(STRING)) {
        std::cout<<getValue<std::string>(value.get())<<"\n";
        return;
    }
    if(value->instanceof(NUMBER)) {
        std::cout<<getValue<double>(value.get())<<"\n";
        return;
    }
    std::cout<<"Unknown\n";
}

void Interpreter::executeBlock(ASTnode* block) {
    if(isLoneBlock(block)) {
        pushScope();
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
    pushScope();
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
    pushScope();
    std::unique_ptr<Object> condition = interpretNode(node->childeren[0]);
    try {
        while(isTruthy(condition.get())) {
            try {
                interpretNode(node->childeren[1]);
            } catch(const ContinueStmt& stmt) {}
            popScope();
            pushScope();
            condition = interpretNode(node->childeren[0]);
        }
    } catch(const BreakStmt& stmt) {
        popScope();
        return;
    }
    popScope();
}

void Interpreter::forStmt(ASTnode* node) {
    pushScope();

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

void Interpreter::funcDecl(ASTnode* node) {
    ASTnode* params = node->childeren[0]->childeren[0];
    ASTnode* body = node->childeren[0]->childeren[1];
    std::string funcName = node->childeren[0]->token.text;
    std::vector<std::string> parameterNames;
    if(params != nullptr) {
        for(ASTnode* param : params->childeren) {
            parameterNames.push_back(param->token.text);
        }
    }
    if(scopes->back().find(funcName) != scopes->back().end()) {
        throw RuntimeError(node->childeren[0]->token, "identifier already declared in this scope");
    }

    scopes->back()[funcName] = std::unique_ptr<Object>(new Obj<Function*>(FUNCTION, new UserFunction(funcName, parameterNames, body)));
}

void Interpreter::returnStmt(ASTnode* node) {
    ASTnode* returnBody = node->childeren[0];
    if(returnBody == nullptr) {
        throw ReturnStmt(nullptr);
    }
    std::unique_ptr<Object> returnObject = interpretNode(returnBody);
    throw ReturnStmt(returnObject.get()->clone());
}

std::unique_ptr<Object> Interpreter::callFunction(ASTnode* node) {
    ASTnode* callee = node->childeren[0];
    ASTnode* argumentBlock = callee->childeren[0];
    std::unique_ptr<Object>* funcPointer = getObject(callee->token.text);
    if(funcPointer == nullptr) {
        throw RuntimeError(callee->token, "'" + callee->token.text + "'" + " is not declared");
    }
    if(!(*funcPointer)->instanceof(FUNCTION)) {
        throw RuntimeError(callee->token, "can only call functions and classes");
    }
    Function* func = getValue<Function*>(funcPointer->get());

    std::vector<std::unique_ptr<Object> > arguments;
    if(argumentBlock != nullptr) {
        for(ASTnode* argument : argumentBlock->childeren) {
            arguments.push_back(interpretNode(argument));
        }
    }


    pushScope();

    std::unique_ptr<Object> result;
    try {
        result = func->call(arguments, callee->token, this);
    } catch(const ReturnStmt& stmt) {
        result = std::unique_ptr<Object>(stmt.value);
    }
    popScope();
    return result;
}

std::unique_ptr<Object> Interpreter::primary(ASTnode* node) {
    return std::unique_ptr<Object>(node->token.value->clone());
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
    Object* lastStoredVariable = scopes->back()[node->childeren.back()->token.text].get();
    return std::unique_ptr<Object>(lastStoredVariable->clone());
}

std::unique_ptr<Object> Interpreter::identifier(ASTnode* node) {

    //check if variable declared at all
    std::unique_ptr<Object>* storedVariable = getObject(node->token.text);
    if(storedVariable == nullptr) {
        throw RuntimeError(node->token, "unknown identifier (variable not declared)");
    }

    //if no error, check if it is an assignment

    if(!node->childeren.empty()) {
        *storedVariable = std::unique_ptr<Object>(interpretNode(node->childeren[0]));
        return std::unique_ptr<Object>(storedVariable->get()->clone());
    }

    //finally, it must be just a table lookup

    return std::unique_ptr<Object>(storedVariable->get()->clone());
}

std::unique_ptr<Object> Interpreter::addition(ASTnode* node) {
    std::unique_ptr<Object> left = interpretNode(node->childeren[0]);
    std::unique_ptr<Object> right = interpretNode(node->childeren[1]);

    //TODO: cover all cases

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
        return std::unique_ptr<Object>(middle.get()->clone());
    }
    return std::unique_ptr<Object>(right.get()->clone());
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
        return callFunction(node);
    case FUNC:
        funcDecl(node);
        return nullptr;
    case RETURN:
        returnStmt(node);
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
    defineNativeFunctions(mainScopeList[0]);
    scopes = &mainScopeList;

    try {
        for(size_t i = 0; i < Interpreter::stmtList.size(); ++i) {
            interpretNode(stmtList[i]);
        }
    } catch(const RuntimeError& error) {
        reportRuntimeError(error);
    }
}
