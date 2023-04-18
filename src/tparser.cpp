#include<tparser.h>
#include<iostream>

ParseError::ParseError() {}

void Parser::printErrorMsg(const Token& token, const std::string& message) {
    if(token.type == END) {
        std::cout<<"[ERROR] at end: "<<message<<"\n";
    } else {
        std::cout<<"[ERROR] line "<<token.line<<" collumn "<<token.collumn<<" at '"<<token.text<<"'"<<": Parse Error: "<<message<<"\n";
    }
}

ParseError Parser::error(const Token& token, const std::string& message) {
    Parser::printErrorMsg(token, message);
    return ParseError();
}

void Parser::synchronize() {
    Parser::advance();
    while(!Parser::isAtEnd()) {
        if(Parser::prev().type == SEMICOLIN) return;

        switch(Parser::peek().type) {
        case WHILE:
        case FOR:
        case IF:
        case LET:
        case CLASS:
        case RETURN:
        case PRINT:
            return;
        default:
            advance();
        }
    }
}

ASTnode::ASTnode(Token token) {
    ASTnode::token = token;
}

ASTnode::~ASTnode() {
    delete this->token.value;
}

Token Parser::peek() {
    return Parser::tokens[Parser::curPos];
}

Token Parser::prev() {
    return Parser::tokens[Parser::curPos - 1];
}

Token Parser::advance() {
    if(!isAtEnd()) Parser::curPos++;
    return Parser::prev();
}

bool Parser::isAtEnd() {
    if(Parser::peek().type == END) return true;
    return false;
}

bool Parser::check(State type) {
    if(isAtEnd()) return false;
    return type == Parser::peek().type;
}

void Parser::consume(State type, const char* errorMsg) {
    if(check(type)) {
        advance();
        return;
    }
    throw error(prev(), errorMsg);
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
    return Parser::statement();
}

ASTnode* Parser::varDeclStmt() {
    ASTnode* identfierType = new ASTnode(prev());
    identfierType->addChild(declExpr());
    while(match(std::vector<State>({COMA}))) {
        identfierType->addChild(declExpr());
    }
    consume(SEMICOLIN, "expected ';' after variable declaration");
    return identfierType;
}

ASTnode* Parser::varDecl() {
    ASTnode* identifierType = new ASTnode(prev());
    identifierType->addChild(declExpr());
    return identifierType;
}

ASTnode* Parser::declExpr() {
    consume(IDENT, "expected identifier");
    ASTnode* identifier = new ASTnode(prev());
    if(match(std::vector<State>({EQ}))) {
        identifier->addChild(expression());
    }
    return identifier;
}

ASTnode* Parser::statement() {
    if(match(std::vector<State>({SEMICOLIN}))) {
        return nullptr;
    }
    if(match(std::vector<State>({PRINT}))) {
        return printStmt();
    }

    if(match(std::vector<State>({LET}))) {
        return varDeclStmt();
    }

    if(match(std::vector<State>({LBRACE}))) {
        return block();
    }

    if(match(std::vector<State>({IF}))) {
        return ifStmt();
    }

    if(match(std::vector<State>({WHILE}))) {
        return whileStmt();
    }

    if(match(std::vector<State>({FOR}))) {
        return forStmt();
    }

    return exprStmt();
}

ASTnode* Parser::block() {
    ASTnode* node = new ASTnode(prev());

    while(!check(RBRACE) && !isAtEnd()) {
        node->addChild(declaration());
    }
    consume(RBRACE, "expected '}' after block");
    return node;
}

ASTnode* Parser::ifStmt() {
    ASTnode* node = new ASTnode(prev());

    consume(LPAREN, "expected '(' after 'if' keyword");
    node->addChild(expression());
    consume(RPAREN, "expected ')'");
    node->addChild(statement());
    if(match(std::vector<State>({ELSE}))) {
        node->addChild(statement());
    }
    return node;
}

ASTnode* Parser::whileStmt() {
    ASTnode* node = new ASTnode(prev());
    consume(LPAREN, "expected '(' after 'while' keyword");
    node->addChild(expression());
    consume(RPAREN, "expected ')'");
    node->addChild(statement());
    return node;
}

ASTnode* Parser::forStmt() {
    ASTnode* node = new ASTnode(prev());
    consume(LPAREN, "expected '(' after 'for' keyword");

    //initial statement

    if(match(std::vector<State>({LET}))) {
        node->addChild(varDeclStmt());
    } else if(match(std::vector<State>({SEMICOLIN}))) {
        node->addChild(nullptr);
    } else {
        node->addChild(exprStmt());
    }

    //condition

    if(!check(SEMICOLIN)) {
        node->addChild(expression());
    } else {
        node->addChild(nullptr);
    }
    consume(SEMICOLIN, "expected ';' after condition");

    //final statement

    if(!check(RPAREN)) {
        node->addChild(exprBlock());
    } else {
        node->addChild(nullptr);
    }
    consume(RPAREN, "expected ')'");

    //body

    node->addChild(statement());

    return node;
}

ASTnode* Parser::exprStmt() {
    ASTnode* node = exprBlock();
    consume(SEMICOLIN, "expected ';' after statement");
    return node;
}

ASTnode* Parser::exprBlock() {
    ASTnode* node = new ASTnode(Token(COMA, "", 0, 0, nullptr));
    node->addChild(expression());
    while(match(std::vector<State>({COMA}))) {
        node->addChild(expression());
    }
    return node;
}

ASTnode* Parser::printStmt() {
    ASTnode* print = new ASTnode(Parser::prev());
    ASTnode* expr = Parser::expression();
    Parser::consume(SEMICOLIN, "expected ';' after print statement");
    print->addChild(expr);
    return print;
}

ASTnode* Parser::expression() {
    if(match(std::vector<State>({LET}))) {
        return varDecl();
    }
    return assignment();
}

ASTnode* Parser::assignment() {
    ASTnode* expr = Parser::logic_or();

    if(Parser::match(std::vector<State>({EQ}))) {
        expr->addChild(Parser::assignment());
    }

    return expr;
}

ASTnode* Parser::logic_or() {
    ASTnode* node = logic_and();
    while(match(std::vector<State>({OROR}))) {
        ASTnode* father = new ASTnode(prev());
        father->addChild(node);
        father->addChild(logic_and());
        node = father;
    }
    return node;
}

ASTnode* Parser::logic_and() {
    ASTnode* node = ternary();
    while(match(std::vector<State>({ANDAND}))) {
        ASTnode* father = new ASTnode(prev());
        father->addChild(node);
        father->addChild(ternary());
        node = father;
    }
    return node;
}

ASTnode* Parser::ternary() {
    ASTnode* expr = Parser::equality();

    if(Parser::match(std::vector<State>({QMARK}))) {
        ASTnode* father = new ASTnode(Parser::prev());
        father->addChild(expr);
        father->addChild(Parser::ternary());
        Parser::consume(COLON, "expected ':'");
        father->addChild(Parser::ternary());
        expr = father;
    }

    return expr;
}

ASTnode* Parser::equality() {
    ASTnode* left = Parser::comparison();

    while(Parser::match(std::vector<State>({EQEQ, NOTEQ}))) {
        Token oper = Parser::prev();
        ASTnode* right = comparison();
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
        Token oper = Parser::prev();
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
        Token oper = Parser::prev();
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
        Token oper = Parser::prev();
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
        Token oper = Parser::prev();
        ASTnode* node = Parser::unary();
        ASTnode* father = new ASTnode(oper);
        father->addChild(node);
        return father;
    }

    return Parser::primary();
}

ASTnode* Parser::primary() {
    Token current = peek();
    std::vector<State> terminals = { FLOATLIT, STRINGLIT, IDENT };
    if(Parser::match(terminals)) {
        return new ASTnode(current);
    }
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
    if(child != nullptr) {
        ASTnode::childeren.back()->father = this;
    }
}

Parser::Parser(std::vector<Token> tokens) {
    Parser::tokens = tokens;
}

std::vector<ASTnode*> Parser::parse() {
    std::vector<ASTnode*> stmtList;
    while(!Parser::isAtEnd()) {
        try {
            stmtList.push_back(Parser::declaration());
        } catch(const ParseError& error) {
            stmtList[0] = nullptr;
            Parser::synchronize();
        }
    }

    return stmtList;
}
