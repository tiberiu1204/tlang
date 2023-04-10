#include<tparser.h>
#include<iostream>

ParseError::ParseError() {}

void Parser::printErrorMsg(Token* token, std::string message) {
    if(token->type == END) {
        std::cout<<"[ERROR] at end: "<<message<<"\n";
    } else {
        std::cout<<"[ERROR] line "<<token->line<<" collumn "<<token->collumn<<" at '"<<token->text<<"'"<<": Parse Error: "<<message<<"\n";
    }
}

ParseError Parser::error(Token* token, std::string message) {
    Parser::printErrorMsg(token, message);
    return ParseError();
}

void Parser::synchronize() {
    Parser::advance();
    while(!Parser::isAtEnd()) {
        if(Parser::prev()->type == SEMICOLIN) return;

        switch(Parser::peek()->type) {
        case WHILE:
        case FOR:
        case IF:
        case INT:
        case STRING:
        case FLOAT:
        case CLASS:
        case RETURN:
            return;
        default:
            advance();
        }
    }
}

ASTnode::ASTnode(Token* token) {
    ASTnode::token = token;
}

Token* Parser::peek() {
    return Parser::tokens[Parser::curPos];
}

Token* Parser::prev() {
    return Parser::tokens[Parser::curPos - 1];
}

Token* Parser::advance() {
    if(!isAtEnd()) Parser::curPos++;
    return Parser::prev();
}

bool Parser::isAtEnd() {
    if(Parser::peek()->type == END) return true;
    return false;
}

bool Parser::check(State type) {
    if(isAtEnd()) return false;
    return type == Parser::peek()->type;
}

void Parser::consume(State type, const char* errorMsg) {
    if(check(type)) {
        advance();
        return;
    }
    throw Parser::error(Parser::prev(), errorMsg);
}

bool Parser::match(std::vector<State> acceptedTokens) {
    for(size_t i = 0; i < acceptedTokens.size(); ++i) {
        if(Parser::check(acceptedTokens[i])) {
            advance();
            return true;
        }
    }
    return false;
}

ASTnode* Parser::declaration() {
    if(Parser::match(std::vector<State>({INT, FLOAT, BOOL, STRING, CHAR}))) {
        return Parser::varDecl();
    }
    return Parser::statement();
}
ASTnode* Parser::varDecl() {
    ASTnode* identifierType = new ASTnode(Parser::prev());
    Parser::consume(IDENT, "expected identifier");
    ASTnode* identifier = new ASTnode(Parser::prev());
    identifierType->addChild(identifier);
    if(Parser::match(std::vector<State>({EQ}))) {
        ASTnode* expr = Parser::expression();
        identifier->addChild(expr);
    }
    Parser::consume(SEMICOLIN, "expected ';' after variable declaration");
    return identifierType;
}

ASTnode* Parser::statement() {
    if(Parser::match(std::vector<State>({PRINT}))) {
        return Parser::printStmt();
    }

    return Parser::exprStmt();
}

ASTnode* Parser::exprStmt() {
    ASTnode* expression = Parser::expression();
    Parser::consume(SEMICOLIN, "expected ';' after expression");
    return expression;
}

ASTnode* Parser::printStmt() {
    ASTnode* print = new ASTnode(Parser::prev());
    ASTnode* expr = Parser::expression();
    Parser::consume(SEMICOLIN, "expected ';' after expression");
    print->addChild(expr);
    return print;
}

ASTnode* Parser::expression() {
    ASTnode* expr = Parser::equality();

    if(Parser::match(std::vector<State>({QMARK}))) {
        ASTnode* father = new ASTnode(Parser::prev());
        father->addChild(expr);
        father->addChild(Parser::expression());
        Parser::consume(COLON, "expected ':'");
        father->addChild(Parser::expression());
        expr = father;
    }

    return expr;
}

ASTnode* Parser::equality() {
    ASTnode* left = Parser::comparison();

    while(Parser::match(std::vector<State>({EQEQ, NOTEQ}))) {
        Token* oper = Parser::prev();
        ASTnode* right = Parser::comparison();
        ASTnode* father = new ASTnode(oper);
        father->addChild(left);
        father->addChild(right);
        left = father;
    }

    return left;
}

ASTnode* Parser::comparison() {
    ASTnode* left = Parser::term();

    while(Parser::match(std::vector<State>({LT, GT, LTEQ, GTEQ}))) {
        Token* oper = Parser::prev();
        ASTnode* right = Parser::term();
        ASTnode* father = new ASTnode(oper);
        father->addChild(left);
        father->addChild(right);
        left = father;
    }

    return left;
}

ASTnode* Parser::term() {
    ASTnode* left = Parser::factor();

    while(Parser::match(std::vector<State>({MINUS, PLUS}))) {
        Token* oper = Parser::prev();
        ASTnode* right = Parser::factor();
        ASTnode* father = new ASTnode(oper);
        father->addChild(left);
        father->addChild(right);
        left = father;
    }

    return left;
}

ASTnode* Parser::factor() {
    ASTnode* left = Parser::unary();

    while(Parser::match(std::vector<State>({SLASH, STAR}))) {
        Token* oper = Parser::prev();
        ASTnode* right = Parser::unary();
        ASTnode* father = new ASTnode(oper);
        father->addChild(left);
        father->addChild(right);
        left = father;
    }

    return left;
}

ASTnode* Parser::unary() {
    if(Parser::match(std::vector<State>({NOT, MINUS}))) {
        Token* oper = Parser::prev();
        ASTnode* node = Parser::unary();
        ASTnode* father = new ASTnode(oper);
        father->addChild(node);
        return father;
    }

    return Parser::primary();
}

ASTnode* Parser::primary() {
    Token* current = Parser::peek();
    if(Parser::match(std::vector<State>({INTLIT}))) return new ASTnode(current);
    if(Parser::match(std::vector<State>({FLOATLIT}))) return new ASTnode(current);
    if(Parser::match(std::vector<State>({STRINGLIT}))) return new ASTnode(current);
    if(Parser::match(std::vector<State>({IDENT}))) return new ASTnode(current);
    if(Parser::match(std::vector<State>({LPAREN}))) {
        ASTnode* node = Parser::expression();
        Parser::consume(RPAREN, "expected ')'");
        return node;
    }
    throw Parser::error(Parser::peek(), "expected expression");
    return nullptr;
}

void ASTnode::addChild(ASTnode* child) {
    ASTnode::childeren.push_back(child);
    ASTnode::childeren.back()->father = this;
}

Parser::Parser(std::vector<Token*> tokens) {
    Parser::tokens = tokens;
}

std::vector<ASTnode*> Parser::parse() {
    std::vector<ASTnode*> stmtList;
    while(!Parser::isAtEnd()) {
        stmtList.push_back(Parser::declaration());
    }

    return stmtList;
}
