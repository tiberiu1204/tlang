#include <iostream>
#include <fstream>
#include<string.h>
#include<tinterpreter.h>

void displayStatement(ASTnode* root) {
    std::cout<<root->token.text<<' ';
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
    std::ofstream out("./output");
    char* input = getInputFromFile("./input");
    Lexer* l = new Lexer(input);
    l->lex();
    std::vector<Token> tokens = l->getTokenList();
    Parser* parser = new Parser(tokens);
    std::vector<ASTnode*> root = parser->parse();
    if(!root.empty()) {
        //displayAST(root);
        Interpreter* interpreter = new Interpreter(root);
        interpreter->interpret();
    }
    std::cin.get();
    //std::cout<<root->token.text;g
    /*for(size_t i = 0; i < tokens.size(); ++i) {
        out<<stateMap.at(tokens[i].type)<<" at line "<<tokens[i].line<<" and collumn "<<tokens[i].collumn<<"\n";
    }*/
}
