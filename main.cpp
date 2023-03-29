#include <iostream>
#include <fstream>
#include<string.h>
#include<tlexer.h>

int main()
{
    char inp[] = "a = 3\nb = 4\nc = a + b\na = a / b\nmisto = false\nmisto = true";
    Lexer* l = new Lexer(inp);
    std::vector<Token*> v;
    v = l->lex();
    for(int i = 0; i < v.size(); ++i) {
        std::cout<<v[i]->type<<" "<<v[i]->text<<" "<<v[i]->startPos<<"\n";
    }
}
