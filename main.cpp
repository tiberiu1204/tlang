#include <iostream>
#include <fstream>
#include<string.h>
#include<tparser.h>

void displayAST(ASTnode* root) {
    std::cout<<root->token.text<<'\n';
    for(int i = 0; i < root->childeren.size(); ++i) {
        displayAST(root->childeren[i]);
    }
}

int main()
{
    std::ofstream out("../output");
    char* input = getInputFromFile("../dumyfile");
    Lexer* l = new Lexer(input);
    l->lex();
    std::vector<Token> tokens = l->getTokenList();
    Parser* parser = new Parser(tokens);
    ASTnode* root = parser->parse();
    displayAST(root);
    //std::cout<<root->token.text;
}
