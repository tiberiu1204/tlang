#include<tparser.h>
#include<iostream>

void Parser::printErrorMsg(const Token& token, const std::string& message) {
    if(token.type == END) {
        std::cout<<"[ERROR] at end: "<<message<<"\n";
    } else {
        std::cout<<"[ERROR] line "<<token.line<<" collumn "<<token.collumn<<" at '"<<token.text<<"'"<<": Parse Error: "<<message<<"\n";
    }
}

ParseError Parser::error(const Token& token, const std::string& message) {
    printErrorMsg(token, message);
    return ParseError();
}

void Parser::synchronize() {
    advance();
    while(!isAtEnd()) {
        if(prev().type == SEMICOLIN) return;

        switch(peek().type) {
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
        if(check(acceptedTokens[i])) {
            advance();
            return true;
        }
    }
    return false;
}

ASTnode* Parser::declaration() {
    if(match(std::vector<State>({FUNC}))) {
        return funcDecl();
    }
    return statement();
}

ASTnode* Parser::funcDecl() {
    inFunction = true;
    ASTnode* node = new ASTnode(prev());
    node->addChild(functionProduction());
    return node;
}

ASTnode* Parser::functionProduction() {

    //root node is function name, first child is parameter block(or nullptr), second is function body

    consume(IDENT, "expected function name");
    ASTnode* node = new ASTnode(prev()); // created root(function name)
    consume(LPAREN, "expected '(' after function declaration");
    if(match(std::vector<State>({IDENT}))) {
        node->addChild(parameters()); //added parameters block
    } else {
        node->addChild(nullptr); //or nullptr
    }
    consume(RPAREN, "expected ')' after function parameters");
    consume(LBRACE, "expected '{' after function declaration");
    node->addChild(block()); //added function body
    return node;
}

ASTnode* Parser::parameters() {
    //root node is COMA, children are parameters' names
    ASTnode* node = new ASTnode(Token(COMA, ",", 0, 0, nullptr));
    node->addChild(new ASTnode(prev()));
    while(match(std::vector({COMA}))) {
        consume(IDENT, "expected parameter name");
        node->addChild(new ASTnode(prev()));
    }
    return node;
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

    if(match(std::vector<State>({CONTINUE}))) {
        if(!inLoop) {
            throw error(prev(), "'continue' keyword can only be used inside a loop");
        }
        ASTnode* node = new ASTnode(prev());
        consume(SEMICOLIN, "expected ';' after 'continue' keyword");
        return node;
    }

    if(match(std::vector<State>({BREAK}))) {
        if(!inLoop) {
            throw error(prev(), "'break' keyword can only be used inside a loop");
        }
        ASTnode* node = new ASTnode(prev());
        consume(SEMICOLIN, "expected ';' after 'break' keyword");
        return node;
    }

    if(match(std::vector<State>({RETURN}))) {
        if(!inFunction) {
            throw error(prev(), "'return' keyword can only be used inside a function definition");
        }
        return returnStmt();
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

ASTnode* Parser::returnStmt() {
    ASTnode* node = new ASTnode(prev());
    if(!check(SEMICOLIN)) {
        node->addChild(expression());
    } else {
        node->addChild(nullptr);
    }
    consume(SEMICOLIN, "expected ';' after return statement");
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
    inLoop = true;
    ASTnode* node = new ASTnode(prev());
    consume(LPAREN, "expected '(' after 'while' keyword");
    node->addChild(expression());
    consume(RPAREN, "expected ')'");
    node->addChild(statement());
    return node;
}

ASTnode* Parser::forStmt() {
    inLoop = true;
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
    ASTnode* node = new ASTnode(Token(COMA, ",", 0, 0, nullptr));
    node->addChild(expression());
    while(match(std::vector<State>({COMA}))) {
        node->addChild(expression());
    }
    return node;
}

ASTnode* Parser::printStmt() {
    ASTnode* print = new ASTnode(prev());
    ASTnode* expr = expression();
    consume(SEMICOLIN, "expected ';' after print statement");
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
    ASTnode* expr = logic_or();

    if(match(std::vector<State>({EQ, PLUSPLUS, MINUSMINUS, PLUSEQ, MINUSEQ, STAREQ, SLASHEQ}))) {
        ASTnode* childNode;
        switch(prev().type) {
        case EQ:
            expr->addChild(assignment());
            break;
        case PLUSPLUS:
            childNode = new ASTnode(Token(PLUS));
            childNode->addChild(new ASTnode(expr->token));
            childNode->addChild(new ASTnode(Token(FLOATLIT, "", 0, 0, new Obj<double>(NUMBER, 1))));
            expr->addChild(childNode);
            break;
        case MINUSMINUS:
            childNode = new ASTnode(Token(MINUS));
            childNode->addChild(new ASTnode(expr->token));
            childNode->addChild(new ASTnode(Token(FLOATLIT, "", 0, 0, new Obj<double>(NUMBER, 1))));
            expr->addChild(childNode);
            break;
        case PLUSEQ:
            childNode = new ASTnode(Token(PLUS));
            childNode->addChild(new ASTnode(expr->token));
            childNode->addChild(assignment());
            expr->addChild(childNode);
            break;
        case MINUSEQ:
            childNode = new ASTnode(Token(MINUS));
            childNode->addChild(new ASTnode(expr->token));
            childNode->addChild(assignment());
            expr->addChild(childNode);
            break;
        case STAREQ:
            childNode = new ASTnode(Token(STAR));
            childNode->addChild(new ASTnode(expr->token));
            childNode->addChild(assignment());
            expr->addChild(childNode);
            break;
        case SLASHEQ:
            childNode = new ASTnode(Token(SLASH));
            childNode->addChild(new ASTnode(expr->token));
            childNode->addChild(assignment());
            expr->addChild(childNode);
            break;
        default:
            break;
        }
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
    ASTnode* expr = equality();

    if(match(std::vector<State>({QMARK}))) {
        ASTnode* father = new ASTnode(prev());
        father->addChild(expr);
        father->addChild(ternary());
        consume(COLON, "expected ':'");
        father->addChild(ternary());
        expr = father;
    }

    return expr;
}

ASTnode* Parser::equality() {
    ASTnode* left = comparison();

    while(match(std::vector<State>({EQEQ, NOTEQ}))) {
        Token oper = prev();
        ASTnode* right = comparison();
        ASTnode* father = new ASTnode(oper);
        father->addChild(left);
        father->addChild(right);
        left = father;
    }

    return left;
}

ASTnode* Parser::comparison() {
    ASTnode* left = term();

    while(match(std::vector<State>({LT, GT, LTEQ, GTEQ}))) {
        Token oper = prev();
        ASTnode* right = term();
        ASTnode* father = new ASTnode(oper);
        father->addChild(left);
        father->addChild(right);
        left = father;
    }

    return left;
}

ASTnode* Parser::term() {
    ASTnode* left = factor();

    while(match(std::vector<State>({MINUS, PLUS}))) {
        Token oper = prev();
        ASTnode* right = factor();
        ASTnode* father = new ASTnode(oper);
        father->addChild(left);
        father->addChild(right);
        left = father;
    }

    return left;
}

ASTnode* Parser::factor() {
    ASTnode* left = unary();

    while(match(std::vector<State>({SLASH, STAR}))) {
        Token oper = prev();
        ASTnode* right = unary();
        ASTnode* father = new ASTnode(oper);
        father->addChild(left);
        father->addChild(right);
        left = father;
    }

    return left;
}

ASTnode* Parser::unary() {
    if(match(std::vector<State>({NOT, MINUS}))) {
        Token oper = prev();
        ASTnode* node = unary();
        ASTnode* father = new ASTnode(oper);
        father->addChild(node);
        return father;
    }

    return call();
}

ASTnode* Parser::call() {
    ASTnode* node = primary();
    if(match(std::vector<State>({LPAREN}))) {
        //father is CALL

        ASTnode* father = new ASTnode(Token(CALL, "call", 0, 0, nullptr));

        //first child is callee (function name)

        father->addChild(node);

        //second child is exprBlock(arguments) or nullptr if no arguments are present

        if(!check(RPAREN)){
            node->addChild(exprBlock());
        } else {
            node->addChild(nullptr);
        }
        consume(RPAREN, "expected ')'");
        node = father;
    }
    return node;
}

ASTnode* Parser::primary() {
    Token current = peek();
    std::vector<State> terminals = { FLOATLIT, STRINGLIT, IDENT };
    if(match(terminals)) {
        return new ASTnode(current);
    }
    if(match(std::vector<State>({LPAREN}))) {
        ASTnode* node = expression();
        consume(RPAREN, "expected ')'");
        return node;
    }
    throw error(Parser::peek(), "expected expression");
    return nullptr;
}

void ASTnode::addChild(ASTnode* child) {
    childeren.push_back(child);
    if(child != nullptr) {
        childeren.back()->father = this;
    }
}

Parser::Parser(std::vector<Token> tokens) {
    Parser::tokens = tokens;
}

std::vector<ASTnode*> Parser::parse() {
    std::vector<ASTnode*> stmtList;
    while(!isAtEnd()) {
        try {
            inLoop = false;
            inFunction = false;
            stmtList.push_back(declaration());
        } catch(const ParseError& error) {
            if(stmtList.empty()) {
                stmtList.push_back(nullptr);
            } else {
                stmtList[0] = nullptr;
            }
            synchronize();
        }
    }

    return stmtList;
}
