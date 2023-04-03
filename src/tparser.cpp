#include<tparser.h>

ASTnode::ASTnode(Token* token) {
    ASTnode::token = token;
}

ASTnode* Parser::expression() {
    return Parser::equality();
}

State Parser::peek() {
    return Parser::tokens[Parser::curPos].type;
}

Token Parser::prev() {
    return Parser::tokens[Parser::curPos - 1];
}

Token Parser::advance() {
    if(!isAtEnd()) Parser::curPos++;
    return Parser::prev();
}

bool Parser::isAtEnd() {
    if(Parser::peek() == END) return true;
    return false;
}

bool Parser::check(State type) {
    if(isAtEnd()) return false;
    return type == Parser::peek();
}

void Parser::consume(State type, const char* errorMsg) {
    if(check(type)) {
        advance();
        return;
    }
    throw std::runtime_error(errorMsg);
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

ASTnode* Parser::equality() {
    ASTnode* left = Parser::comparison();

    while(Parser::match(std::vector<State>({EQEQ, NOTEQ}))) {
        Token oper = Parser::prev();
        ASTnode* right = Parser::comparison();
        ASTnode* father = new ASTnode(&oper);
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
        ASTnode* father = new ASTnode(&oper);
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
        ASTnode* father = new ASTnode(&oper);
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
        ASTnode* father = new ASTnode(&oper);
        father->addChild(left);
        father->addChild(right);
        left = father;
    }

    return left;
}

ASTnode* Parser::unary() {
    if(Parser::match(std::vector<State>({NOT, MINUS}))) {
        Token opr = Parser::prev();
        ASTnode* node = Parser::unary();
        ASTnode* father = new ASTnode(&opr);
        father->addChild(node);
        return father;
    }
    return Parser::primary();
}

ASTnode* Parser::primary() {
    Token current = Parser::tokens[Parser::curPos];
    if(Parser::match(std::vector<State>({INTLIT}))) return new ASTnode(&current);
    if(Parser::match(std::vector<State>({STRINGLIT}))) return new ASTnode(&current);
    if(Parser::match(std::vector<State>({CHARLIT}))) return new ASTnode(&current);
    if(Parser::match(std::vector<State>({TRUE}))) return new ASTnode(&current);
    if(Parser::match(std::vector<State>({FALSE}))) return new ASTnode(&current);
    if(Parser::match(std::vector<State>({LPAREN}))) {
        ASTnode* node = Parser::expression();
        Parser::consume(RPAREN, "Syntax Error: expected ')'");
        return node;
    }
    throw std::runtime_error("Syntax Error");
}

void ASTnode::addChild(ASTnode* child) {
    ASTnode::childeren.push_back(child);
    ASTnode::childeren.back()->father = this;
}

Parser::Parser(std::vector<Token> tokens) {
    Parser::tokens = tokens;
}

/*ASTnode* Parser::parse() {
    ASTnode* root = new ASTnode("ROOT");
    return root;
}*/
