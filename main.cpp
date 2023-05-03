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

int main()
{
    std::cout.precision(16);
    std::ofstream out("./output");
    char* input = getInputFromFile("./input");
    Lexer* l = new Lexer(input);
    l->lex();
    std::vector<Token> tokens = l->getTokenList();
    Parser* parser = new Parser(tokens);
    std::vector<ASTnode*> root = parser->parse();
    if(!root.empty()) {
        if(root[0] == nullptr) return -1;
        //displayAST(root);
        Interpreter* interpreter = new Interpreter(root);
        Resolver res = Resolver(root, interpreter);
        if(res.run()) {
            interpreter->interpret();
        }
    }
    /*for(size_t i = 0; i < tokens.size(); ++i) {
        out<<stateMap.at(tokens[i].type)<<" at line "<<tokens[i].line<<" and collumn "<<tokens[i].collumn<<"\n";
    }*/
}
