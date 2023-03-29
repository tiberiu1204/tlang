#include<tlexer.h>
#include<string.h>

bool isNumber(char* s) {
    if(s == nullptr) return false;
    for(int i = 0; i < strlen(s); ++i) {
        if(!(s[i] > '0' && s[i] < '9')) {
            return false;
        }
    }
    return true;
}

Token::Token(Type type, char* text, long long startPos) {
    Token::type = type;
    Token::text = text;
    Token::startPos = startPos;
}

Lexer::Lexer() {
    input = "";
    delim = " ;\n";
}

Lexer::Lexer(char* input) {
    Lexer::input = input;
    delim = " ;\n";
}

std::vector<Token*> Lexer::lex() {
    long long inputSize = strlen(input);
    char* token = strtok(input, delim);
    std::vector<Token*> tokenList;
    while(true) {
        if(token == nullptr) {
            tokenList.push_back(new Token(eof, "<EOF>", inputSize));
            break;
        }
        else if(isNumber(token)) {
            tokenList.push_back(new Token(Num, token, token - input));
        }
        else if(!strcmp("+", token)) {
            tokenList.push_back(new Token(Plus, token, token - input));
        }
        else if(!strcmp("-", token)) {
            tokenList.push_back(new Token(Minus, token, token - input));
        }
        else if(!strcmp("*", token)) {
            tokenList.push_back(new Token(Times, token, token - input));
        }
        else if(!strcmp("/", token)) {
            tokenList.push_back(new Token(Divided, token, token - input));
        }
        else if(!strcmp("=", token)) {
            tokenList.push_back(new Token(Equal, token, token - input));
        }
        else if(!strcmp("true", token)) {
            tokenList.push_back(new Token(True, token, token - input));
        }
        else if(!strcmp("false", token)) {
            tokenList.push_back(new Token(False, token, token - input));
        }
        else {
            tokenList.push_back(new Token(Identifier, token, token - input));
        }
        token = strtok(NULL, delim);
    }
    return tokenList;
}
