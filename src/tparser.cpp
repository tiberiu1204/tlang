#include<tparser.h>

ASTnode::ASTnode(Token* token) {
    ASTnode::token = token;
}

ASTnode* Parser::expression() {
    return Parser::equality();
}

ASTnode* Parser::equality() {

}

ASTnode* Parser::comparison() {

}

ASTnode* Parser::term() {

}

ASTnode* Parser::factor() {

}

ASTnode* Parser::unary() {

}

ASTnode* Parser::primary() {

}

void ASTnode::addChild(ASTnode* child) {
    ASTnode::childeren.push_back(child);
    ASTnode::childeren.back()->father = this;
}

Parser::Parser(std::vector<Token> tokens) {
    Parser::tokens = tokens;
}

ASTnode* Parser::parse() {
    /*ASTnode* root = new ASTnode("ROOT");
    return root;*/
}
