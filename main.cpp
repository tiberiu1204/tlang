#include <iostream>
#include <fstream>
#include<string.h>
#include<tlexer.h>

int main()
{
    std::ofstream out("../output");
    char* input = getInputFromFile("../dumyfile");
    Lexer* l = new Lexer(input);
    l->lex();
    std::vector<Token> tokens = l->getTokenList();
    for(size_t i = 0; i < tokens.size(); ++i) {
        out<<stateMap.at(tokens[i].type)<<" "<<tokens[i].text<<"\n";
    }
}
