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
    case IDENT:
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
        Object* value = arguments[argumentIndex++]->clone();
        interpreter->callStack.top().insertObject(paramName, value);
    }
    return interpreter->interpretNode(m_Body);
}

void Interpreter::reportRuntimeError(const RuntimeError& error) {
    std::cout<<"[ERROR] at line "<<error.token.line<<" collumn "<<error.token.collumn<<" at '"<<error.token.text<<"': Runtime error: "<<error.message<<"\n";
}

void Interpreter::popScope(const size_t& position) {
    while(callStack.top().size() > position) {
        callStack.top().popScope();
    }
}

void Interpreter::pushScope() {
    callStack.top().pushScope();
}

void Interpreter::clearScope(std::unordered_set<std::string> omit) {
    for(const auto& variable : callStack.top().back()) {
        if(omit.find(variable.first) == omit.end()) {
            callStack.top().back().erase(variable.first);
        }
    }
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
}

void Interpreter::executeBlock(ASTnode* block) {
    size_t before = callStack.top().size();
    if(isLoneBlock(block)) {
        pushScope();
    }
    for(size_t i = 0; i < block->childeren.size(); ++i) {
        interpretNode(block->childeren[i]);
    }
    if(isLoneBlock(block)) {
        popScope(before);
    }
}

void Interpreter::exprStmt(ASTnode* node) {
    for(size_t i = 0; i < node->childeren.size(); ++i) {
         interpretNode(node->childeren[i]);
    }
}

void Interpreter::ifStmt(ASTnode* node) {
    size_t before = callStack.top().size();
    pushScope();
    std::unique_ptr<Object> condition = interpretNode(node->childeren[0]);
    ASTnode* nodeToExecute;
    if(isTruthy(condition.get())) {
        nodeToExecute = node->childeren[1];
    } else {
        nodeToExecute = node->childeren[2];
    }
    interpretNode(nodeToExecute);
    popScope(before);
}

void Interpreter::whileStmt(ASTnode* node) {
    size_t before = callStack.top().size();
    pushScope();
    std::unique_ptr<Object> condition = interpretNode(node->childeren[0]);
    try {
        while(isTruthy(condition.get())) {
            try {
                interpretNode(node->childeren[1]);
            } catch(const ContinueStmt& stmt) {}
            popScope(before);
            pushScope();
            condition = interpretNode(node->childeren[0]);
        }
    } catch(const BreakStmt& stmt) {
        popScope(before);
        return;
    }
    popScope(before);
}

void Interpreter::forStmt(ASTnode* node) {
    size_t before = callStack.top().size();
    pushScope();

    //execute initial statement

    interpretNode(node->childeren[0]);
    std::unique_ptr<Object> condition = interpretNode(node->childeren[1]);

    //store initial variable declarations in a set to be omitted when scope is cleared

    std::unordered_set<std::string> omit;
    for(const auto& element : callStack.top().back()) {
        omit.insert(element.first);
    }

    try {
        while(isTruthy(condition.get())) {
            //execute body
            try {
                interpretNode(node->childeren[3]);
            } catch(const ContinueStmt& stmt) {}
            popScope(before + 1);

            //execute increment statement

            interpretNode(node->childeren[2]);

            //clear scope except variables declared in initial statement

            clearScope(omit);

            //re-interpret condition

            condition = interpretNode(node->childeren[1]);
        }
    } catch(const BreakStmt& stmt) {
        popScope(before);
        return;
    }
    popScope(before);
}

void Interpreter::funcDecl(ASTnode* node) {
    ASTnode* params = node->childeren[0]->childeren[0];
    ASTnode* body = node->childeren[0]->childeren[1];
    std::string& funcName = node->childeren[0]->token.text;
    std::vector<std::string> parameterNames;
    if(params != nullptr) {
        for(ASTnode* param : params->childeren) {
            parameterNames.push_back(param->token.text);
        }
    }

    Function* func = new UserFunction(funcName, parameterNames, body);
    callStack.top().insertObject(funcName, new Obj<Function*>(FUNCTION, func));
    func->setStackFrame(callStack.top());
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
    Object* funcObject;

    if(nativeFunctions.find(callee->token.text) != nativeFunctions.end()) {
        funcObject = nativeFunctionsMap[callee->token.text];
    } else {
        funcObject = callStack.top().getObject(callee->token.text, resolverMap[callee]);
    }

    if(!funcObject->instanceof(FUNCTION)) {
        throw RuntimeError(callee->token, "can only call functions and classes");
    }
    Function* func = getValue<Function*>(funcObject);

    std::vector<std::unique_ptr<Object> > arguments;
    if(argumentBlock != nullptr) {
        for(ASTnode* argument : argumentBlock->childeren) {
            arguments.push_back(interpretNode(argument));
        }
    }

    std::unique_ptr<Object> result;
    callStack.push(func->getStackFrame());
    try {
        result = func->call(arguments, callee->token, this);
    } catch(const ReturnStmt& stmt) {
        result = std::unique_ptr<Object>(stmt.value);
    }
    callStack.pop();
    return result;
}

std::unique_ptr<Object> Interpreter::primary(ASTnode* node) {
    return std::unique_ptr<Object>(node->token.value->clone());
}

std::unique_ptr<Object> Interpreter::varDecl(ASTnode* node) {
    for(ASTnode* ident : node->childeren) {
        if(ident->childeren.empty()) {
            callStack.top().insertObject(ident->token.text, new Obj<double>(NUMBER, 0));
        } else {
            callStack.top().insertObject(ident->token.text, interpretNode(ident->childeren[0])->clone());
        }
    }
    Object* lastStoredVariable = callStack.top().getObject(node->childeren.back()->token.text, 0);
    return std::unique_ptr<Object>(lastStoredVariable->clone());
}

std::unique_ptr<Object> Interpreter::identifier(ASTnode* node) {

    //check if it is an assignment

    if(!node->childeren.empty()) {
        std::unique_ptr<Object> result = interpretNode(node->childeren[0]);
        callStack.top().replaceObject(node->token.text, resolverMap[node], result.get(), node->token);
        return result;
    }

    //finally, it must be just a table lookup
    Object* variable = callStack.top().getObject(node->token.text, resolverMap[node]);
    return std::unique_ptr<Object>(variable->clone());
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

std::unique_ptr<Object> Interpreter::modulus(ASTnode* node) {
    std::unique_ptr<Object> left = interpretNode(node->childeren[0]);
    std::unique_ptr<Object> right = interpretNode(node->childeren[1]);
    if(!isNumber(left.get()) || !isNumber(right.get())) {
        throw RuntimeError(node->token, "modulus can only be performed between numbers");
    }
    double leftNumber = getValue<double>(left.get());
    double rightNumber = getValue<double>(right.get());
    if((double)((int)leftNumber) != leftNumber || (double)((int)rightNumber) != rightNumber) {
        throw RuntimeError(node->token, "modulus can only be performed on integer numbers");
    }
    double result = (double)((int)leftNumber % (int)rightNumber);
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
    case MOD:
        return modulus(node);
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

void Interpreter::resolve(ASTnode* node, size_t depth) {
    resolverMap[node] = depth;
}

void Interpreter::interpret() {
    callStack.push(StackFrame());
    nativeFunctionsMap = defineNativeFunctions();
    if(stmtList[0] == nullptr) {
        return;
    }

    try {
        for(size_t i = 0; i < Interpreter::stmtList.size(); ++i) {
            interpretNode(stmtList[i]);
        }
    } catch(const RuntimeError& error) {
        reportRuntimeError(error);
    }
    //[DEBUG]
    /*for(auto element : resolverMap) {
        std::cout<<element.first->token.text << " " <<element.second<<'\n';
    }*/
}
