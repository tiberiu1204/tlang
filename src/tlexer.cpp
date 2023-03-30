#include<tlexer.h>
#include<string.h>
#include<stdlib.h>

DFAnode* transition(char input, DFAnode* currNode) {
    for(long long unsigned int i = 0; i < currNode->adjArcs.size(); ++i) {
        if(strchr(currNode->adjArcs[i].chars, input) != nullptr) {
            return currNode->adjArcs[i].destNode;
        }
    }
    return nullptr;
}

std::vector<DFAnode*> defineDFA() {
    std::vector<DFAnode*> DFA;

    const char* numbers = "0123456789";
    const char* identStart = "qwertyuiopasdfghjklzxcvbnm_ABCDEFGHIJKLMNOPQRSTUVWXYZ";
    const char* identContinue = "qwertyuiopasdfghjklzxcvbnm_0123456789ABCDEFGHIJKLMNOPQRSTUVWXYZ";
    const char* commentContents = " !\"#$%&\'()*+,-./0123456789:;<=>?@ABCDEFGHIJKLMNOPQRSTUVWXYZ[\\]^_`abcdefghijklmnopqrstuvwxyz{|}~\t";
    const char* stringContent = " !\'#$%&()*+,-./0123456789:;<=>?@ABCDEFGHIJKLMNOPQRSTUVWXYZ[\\]^_`abcdefghijklmnopqrstuvwxyz{|}~\t\n";

    DFA.push_back(new DFAnode(START));
    DFA.push_back(new DFAnode(END));
    DFA.push_back(new DFAnode(PLUS));
    DFA.push_back(new DFAnode(EQ));
    DFA.push_back(new DFAnode(PLUSPLUS));
    DFA.push_back(new DFAnode(PLUSEQ));
    DFA.push_back(new DFAnode(MINUS));
    DFA.push_back(new DFAnode(MINUSMINUS));
    DFA.push_back(new DFAnode(MINUSEQ));
    DFA.push_back(new DFAnode(STAR));
    DFA.push_back(new DFAnode(STAREQ));
    DFA.push_back(new DFAnode(SLASH));
    DFA.push_back(new DFAnode(COMMENT));
    DFA.push_back(new DFAnode(SLASHEQ));
    DFA.push_back(new DFAnode(BACKSLASH));
    DFA.push_back(new DFAnode(COMA));
    DFA.push_back(new DFAnode(POINT));
    DFA.push_back(new DFAnode(LBRACE));
    DFA.push_back(new DFAnode(RBRACE));
    DFA.push_back(new DFAnode(LPAREN));
    DFA.push_back(new DFAnode(RPAREN));
    DFA.push_back(new DFAnode(SEMICOLIN));
    DFA.push_back(new DFAnode(INT));
    DFA.push_back(new DFAnode(FLOAT));
    DFA.push_back(new DFAnode(BIGINT));
    DFA.push_back(new DFAnode(STRING));
    DFA.push_back(new DFAnode(BOOL));
    DFA.push_back(new DFAnode(CHAR));
    DFA.push_back(new DFAnode(INTLIT));
    DFA.push_back(new DFAnode(FLOATLIT));
    DFA.push_back(new DFAnode(BIGINTLIT));
    DFA.push_back(new DFAnode(STRINGLIT));
    DFA.push_back(new DFAnode(STRLITERR));
    DFA.push_back(new DFAnode(CHARLIT));
    DFA.push_back(new DFAnode(LT));
    DFA.push_back(new DFAnode(LTEQ));
    DFA.push_back(new DFAnode(GT));
    DFA.push_back(new DFAnode(GTEQ));
    DFA.push_back(new DFAnode(EQEQ));
    DFA.push_back(new DFAnode(IDENT));
    DFA.push_back(new DFAnode(IF));
    DFA.push_back(new DFAnode(ELSE));
    DFA.push_back(new DFAnode(WHILE));
    DFA.push_back(new DFAnode(FOR));
    DFA.push_back(new DFAnode(OR));
    DFA.push_back(new DFAnode(OROR));
    DFA.push_back(new DFAnode(AND));
    DFA.push_back(new DFAnode(ANDAND));
    DFA.push_back(new DFAnode(NOT));
    DFA.push_back(new DFAnode(NOTEQ));
    DFA.push_back(new DFAnode(MOD));
    DFA.push_back(new DFAnode(MODEQ));
    DFA.push_back(new DFAnode(ERROR));
    DFA.push_back(new DFAnode(DISCARD));
    DFA.push_back(new DFAnode(ID_OR_KW));

    // =

    DFA[START]->createArcTo("=", DFA[EQ]);

    // + ++ +=

    DFA[START]->createArcTo("+", DFA[PLUS]);
    DFA[PLUS]->createArcTo("+", DFA[PLUSPLUS]);
    DFA[PLUS]->createArcTo("=", DFA[PLUSEQ]);

    // - -- -=

    DFA[START]->createArcTo("-", DFA[MINUS]);
    DFA[MINUS]->createArcTo("-", DFA[MINUSMINUS]);
    DFA[MINUS]->createArcTo("=", DFA[MINUSEQ]);

    // / /=

    DFA[START]->createArcTo("/", DFA[SLASH]);
    DFA[SLASH]->createArcTo("=", DFA[SLASHEQ]);

    // * *=

    DFA[START]->createArcTo("*", DFA[STAR]);
    DFA[STAR]->createArcTo("=", DFA[STAREQ]);

    // % %=

    DFA[START]->createArcTo("%", DFA[MOD]);
    DFA[MOD]->createArcTo("=", DFA[MODEQ]);

    //{} () TODO []

    DFA[START]->createArcTo("{", DFA[LBRACE]);
    DFA[START]->createArcTo("}", DFA[RBRACE]);
    DFA[START]->createArcTo("(", DFA[LPAREN]);
    DFA[START]->createArcTo(")", DFA[RPAREN]);

    // | || & && == < > <= >= ! !=

    DFA[START]->createArcTo("|", DFA[OR]); // |
    DFA[OR]->createArcTo("|", DFA[OROR]); // ||
    DFA[START]->createArcTo("&", DFA[AND]); // &
    DFA[AND]->createArcTo("&", DFA[ANDAND]); // &&
    DFA[EQ]->createArcTo("=", DFA[EQEQ]); // ==
    DFA[START]->createArcTo("<", DFA[LT]); // <
    DFA[START]->createArcTo(">", DFA[GT]); // >
    DFA[LT]->createArcTo("=", DFA[LTEQ]); // <=
    DFA[GT]->createArcTo("=", DFA[GTEQ]); // >=
    DFA[START]->createArcTo("!", DFA[NOT]); // !
    DFA[NOT]->createArcTo("=", DFA[NOTEQ]); // !=

    // ; . ,

    DFA[START]->createArcTo(";", DFA[SEMICOLIN]);
    DFA[START]->createArcTo(".", DFA[POINT]);
    DFA[START]->createArcTo(",", DFA[COMA]);

    //coments TODO implement /*

    DFA[SLASH]->createArcTo("/", DFA[COMMENT]);
    DFA[COMMENT]->createArcTo(commentContents, DFA[COMMENT]);

    //INT

    DFA[START]->createArcTo(numbers, DFA[INTLIT]);
    DFA[INTLIT]->createArcTo(numbers, DFA[INTLIT]);

    //FLOAT

    DFA[INTLIT]->createArcTo(".", DFA[ERROR]);
    DFA[ERROR]->createArcTo(numbers, DFA[FLOATLIT]);
    DFA[FLOATLIT]->createArcTo(numbers, DFA[FLOATLIT]);

    //STRING
    DFA[START]->createArcTo("\"", DFA[STRLITERR]);
    DFA[STRLITERR]->createArcTo(stringContent, DFA[STRLITERR]);
    DFA[STRLITERR]->createArcTo("\"", DFA[STRINGLIT]);

    //keywords and identifiers

    DFA[START]->createArcTo(identStart, DFA[ID_OR_KW]);
    DFA[ID_OR_KW]->createArcTo(identContinue, DFA[ID_OR_KW]);

    //white space

    DFA[START]->createArcTo("\t\n ", DFA[DISCARD]);
    DFA[DISCARD]->createArcTo("\t\n ", DFA[DISCARD]);

    return DFA;
}

std::vector<Token> Lexer::getTokenList() {
    return Lexer::tokens;
}

State idOrKw(std::string text) {
    if(text == "if") return IF;
    if(text == "else") return ELSE;
    if(text == "while") return WHILE;
    if(text == "for") return FOR;
    if(text == "int") return INT;
    if(text == "char") return CHAR;
    if(text == "string") return STRING;
    if(text == "float") return FLOAT;
    if(text == "bool") return BOOL;
    if(text == "bigint") return BIGINT;
    return IDENT;
}

char* getInputFromFile(const char* fileName) {
    FILE* file = fopen(fileName, "r");

    if(file == NULL) {
        return NULL;
    }

    fseek(file, 0, SEEK_END);
    int length = ftell(file);
    fseek(file, 0, SEEK_SET);

    char* input = (char*)malloc(sizeof(char) * (length + 1));

    char c;
    int i = 0;
    while((c = fgetc(file)) != EOF) {
        input[i++] = c;
    }

    input[i] = '\0';
    fclose(file);

    return input;
}

Token::Token(State type, std::string text) {
    Token::type = type;
    Token::text = text;
}

DFAarc::DFAarc(const char* chars, DFAnode* destNode) {
    DFAarc::chars = chars;
    DFAarc::destNode = destNode;
}

void DFAnode::createArcTo(const char* chars, DFAnode* node) {
    DFAnode::adjArcs.push_back(DFAarc(chars, node));
}

DFAnode::DFAnode(State s) {
    DFAnode::state = s;
}

Lexer::Lexer(const char* input) {
    Lexer::input = input;
    Lexer::DFA = defineDFA();
}

void Lexer::lex() {
    DFAnode* currNode = Lexer::DFA[START];
    DFAnode* nextNode;
    std::string text = "";
    for(size_t i = 0; i < strlen(Lexer::input); ++i) {
        nextNode = transition(Lexer::input[i], currNode);
        if(nextNode != nullptr) {
            if(currNode->state != DISCARD) {
                text += Lexer::input[i];
            }
            currNode = nextNode;
        } else {
            switch(currNode->state) {
            case DISCARD:
                break;
            case COMMENT:
                break;
            case ERROR:
                throw;
            case ID_OR_KW:
                Lexer::tokens.push_back(Token(idOrKw(text), text));
                break;
            default:
                Lexer::tokens.push_back(Token(currNode->state, text));
                break;
            }
            currNode = DFA[START];
            i--;
            text = "";
        }
    }
    switch(currNode->state) {
    case DISCARD:
        break;
    case ERROR:
        throw;
    case ID_OR_KW:
        Lexer::tokens.push_back(Token(currNode->state, text));
        break;
    default:
        Lexer::tokens.push_back(Token(currNode->state, text));
        break;
    }
}
