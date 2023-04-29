#include<tresolver.h>

Resolver::Resolver(const std::vector<ASTnode*>& stmtList, Interpreter* interpreter) {
    m_StmtList = stmtList;
    m_pInterpreter = interpreter;
}

void Resolver::pushScope() {
    m_Scopes.push_back(std::unordered_set<std::string>());
}

void Resolver::popScope() {
    m_Scopes.pop_back();
}

void Resolver::block(ASTnode* node) {
    if(node->father != nullptr) {
        switch(node->father->token.type) {
        case IF:
        case WHILE:
        case FOR:
        case FUNC:
            visitNodeChildren(node);
            return;
        default:
            break;
        }
    }
    pushScope();
    visitNodeChildren(node);
    popScope();
}

void Resolver::ifStmt(ASTnode* node) {
    ASTnode* condition = node->childeren[0];
    ASTnode* ifBody = node->childeren[1];
    ASTnode* elseBody = node->childeren[2];

    pushScope();
    visitNode(condition);
    visitNode(ifBody);
    popScope();

    pushScope();
    visitNode(elseBody);
    popScope();
}

void Resolver::whileStmt(ASTnode* node) {
    pushScope();
    visitNodeChildren(node);
    popScope();
}

void Resolver::forStmt(ASTnode* node) {
    pushScope();
    visitNodeChildren(node);
    popScope();
}

void Resolver::funcDecl(ASTnode* node) {
    ASTnode* func = node->childeren[0];
    ASTnode* params = func->childeren[0];
    ASTnode* functionBody = func->childeren[1];

    m_Scopes.back().insert(func->token.text);
    pushScope();
    if(params != nullptr) {
        for(ASTnode* param : params->childeren) {
            varDecl(param);
        }
    }
    visitNodeChildren(functionBody);
    popScope();
}

void Resolver::varDecl(ASTnode* node) {
    if(m_Scopes.back().find(node->token.text) != m_Scopes.back().end()) {
        throw error(node->token, "identifier already declared in this scope");
    }
    m_Scopes.back().insert(node->token.text);
    visitNodeChildren(node);
}

void Resolver::identifier(ASTnode* node) {
    if(nativeFunctions.find(node->token.text) != nativeFunctions.end()) {
        visitNodeChildren(node);
        return;
    }
    for(size_t i = m_Scopes.size() - 1; ; --i) {
        std::unordered_set<std::string>& scope = m_Scopes[i];
        if(scope.find(node->token.text) != scope.end()) {
            size_t depth = m_Scopes.size() - i - 1;
            m_pInterpreter->resolve(node, depth);
            break;
        }
        if(i == 0) {
            throw error(node->token, "could not resolve identifier to any declaration");
            break;
        }
    }
    visitNodeChildren(node);
}

void Resolver::visitNodeChildren(ASTnode* node) {
    for(ASTnode* child : node->childeren) {
        visitNode(child);
    }
}

void Resolver::visitNode(ASTnode* node) {
    if(node == nullptr) {
        return;
    }
    switch(node->token.type) {
    case IDENT:
        identifier(node);
        break;
    case LET:
        varDecl(node->childeren[0]);
        break;
    case LBRACE:
        block(node);
        break;
    case IF:
        ifStmt(node);
        break;
    case WHILE:
        whileStmt(node);
        break;
    case FOR:
        forStmt(node);
        break;
    case FUNC:
        funcDecl(node);
        break;
    default:
        visitNodeChildren(node);
        break;
    }
}

bool Resolver::run() {
    bool hasError = false;
    pushScope();
    for(ASTnode* stmt : m_StmtList) {
        try {
            visitNode(stmt);
        } catch(const ParseError& err) {
            hasError = true;
        }
    }
    return !hasError;
}
