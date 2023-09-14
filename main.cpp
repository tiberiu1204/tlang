#include <iostream>
#include <fstream>
#include<string.h>
#include<tresolver.h>
#include<cstdlib>

void displayStatement(ASTnode* root) {
    if(root != nullptr) {
        std::cout<<root->token.text<<' ';
    } else {
        std::cout<<"null ";
        return;
    }
    for(size_t i = 0; i < root->childeren.size(); ++i) {
        displayStatement(root->childeren[i]);
    }
}

void displayAST(std::vector<ASTnode*> v) {
    for(size_t i = 0; i < v.size(); ++i) {
        displayStatement(v[i]);
        std::cout<<'\n';
    }
}

int main(int argc, char *argv[])
{
    std::cout.precision(16);
    if(argc == 1) {
        std::cout<<"You should supply a file name\n";
        return 0;
    }
    if(argc > 2) {
        std::cout<<"Only one file name may be supplied\n";
        return 0;
    }
    char* input = getInputFromFile(argv[1]);
    if(input == NULL) {
        std::cout<<"No such file foud\n";
        return 0;
    }
    Lexer* l = new Lexer(input);
    l->lex();
    std::vector<Token> tokens = l->getTokenList();
    Parser* parser = new Parser(tokens);
    std::vector<ASTnode*> root = parser->parse();
    if(!root.empty()) {
        if(root[0] == nullptr) return -1;
        Interpreter* interpreter = new Interpreter(root);
        Resolver res = Resolver(root, interpreter);
        if(res.run()) {
            interpreter->interpret();
        }
    }
}
