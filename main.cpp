#include <iostream>
#include <fstream>
#include<string.h>
#include<tinterpreter.h>

void displayAST(ASTnode* root) {
    std::cout<<root->token.text<<'\n';
    for(size_t i = 0; i < root->childeren.size(); ++i) {
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
    std::vector<ASTnode*> root = parser->parse();
    if(!root.empty()) {
        //displayAST(root);
        Interpreter* interpreter = new Interpreter(root);
        interpreter->interpret();
        //std::cout<<root->token.text;
    }
    //std::cout<<root->token.text;
    /*for(size_t i = 0; i < tokens.size(); ++i) {
        out<<stateMap.at(tokens[i].type)<<" at line "<<tokens[i].line<<" and collumn "<<tokens[i].collumn<<"\n";
    }*/
}
