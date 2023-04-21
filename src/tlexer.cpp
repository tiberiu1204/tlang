#include<tlexer.h>
#include<string.h>
#include<stdlib.h>
#include<sstream>

DFAnode* transition(char input, DFAnode* currNode) {
    for(long long unsigned int i = 0; i < currNode->adjArcs.size(); ++i) {
        DFAarc arc = currNode->adjArcs[i];
        if(strchr(arc.chars, input) != nullptr && !arc.allBut) {
            return arc.destNode;
        } else if(strchr(arc.chars, input) == nullptr && arc.allBut) {
            return arc.destNode;
        }
    }
    return nullptr;
}

std::vector<DFAnode*> defineDFA() {
    std::vector<DFAnode*> DFA;

    const char* numbers = "0123456789";
    const char* identStart = "abcdefghijklmnopqrstuvwxyz_ABCDEFGHIJKLMNOPQRSTUVWXYZ";
    const char* identContinue = "abcdefghijklmnopqrstuvwxyz_0123456789ABCDEFGHIJKLMNOPQRSTUVWXYZ";

    for(auto element : stateMap) {
        DFA.push_back(new DFAnode(element.first));
    }

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

    // ; . , : ?

    DFA[START]->createArcTo(";", DFA[SEMICOLIN]);
    DFA[START]->createArcTo(".", DFA[POINT]);
    DFA[START]->createArcTo(",", DFA[COMA]);
    DFA[START]->createArcTo(":", DFA[COLON]);
    DFA[START]->createArcTo("?", DFA[QMARK]);

    //coments

    DFA[SLASH]->createArcTo("/", DFA[COMMENT]);
    DFA[COMMENT]->createArcTo("\n", DFA[COMMENT], true);

    DFA[SLASH]->createArcTo("*", DFA[MLCOMM]);
    DFA[MLCOMM]->createArcTo("*", DFA[MLCOMM], true);
    DFA[MLCOMM]->createArcTo("*", DFA[MLCOMMST]);
    DFA[MLCOMMST]->createArcTo("/", DFA[MLCOMM], true);
    DFA[MLCOMMST]->createArcTo("/", DFA[MLCOMMEND]);

    //INT

    DFA[START]->createArcTo(numbers, DFA[INTLIT]);
    DFA[INTLIT]->createArcTo(identStart, DFA[INTLITERR]);
    DFA[INTLIT]->createArcTo(numbers, DFA[INTLIT]);

    //FLOAT

    DFA[INTLIT]->createArcTo(".", DFA[FLOATLITERR]);
    DFA[FLOATLITERR]->createArcTo(numbers, DFA[FLOATLIT]);
    DFA[FLOATLIT]->createArcTo(numbers, DFA[FLOATLIT]);

    //STRING & CHAR
    DFA[START]->createArcTo("\"", DFA[STRLITERR]);
    DFA[STRLITERR]->createArcTo("\"\n", DFA[STRLITERR], true);
    DFA[STRLITERR]->createArcTo("\"", DFA[STRINGLIT]);

    DFA[START]->createArcTo("'", DFA[CHARLITERR]);
    DFA[CHARLITERR]->createArcTo("'\n", DFA[CHARLITERR], true);
    DFA[CHARLITERR]->createArcTo("'", DFA[CHARLIT]);

    //keywords & identifiers

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
    if(text == "let") return LET;
    if(text == "true") return TRUE;
    if(text == "false") return FALSE;
    if(text == "return") return RETURN;
    if(text == "print") return PRINT;
    if(text == "class") return CLASS;
    if(text == "continue") return CONTINUE;
    if(text == "break") return BREAK;
    if(text == "func") return FUNC;
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

Object::Object(const Type& t) : type(t) {}

bool Object::instanceof(const Type& type) {
    if(Object::type == type) {
        return true;
    }
    return false;
}

Token::Token() {
    Token::text = "undefined";
}

Token::Token(const State& type) {
    Token::type = type;
    value = nullptr;
    line = 0;
    collumn = 0;
    text = "";
}

Token::Token(const State& type, const std::string& text, const size_t& line, const size_t& collumn, Object* value) {
    Token::type = type;
    Token::text = text;
    Token::line = line;
    Token::collumn = collumn;
    Token::value = value;
}

DFAarc::DFAarc(const char* chars, DFAnode* destNode, bool allBut = false) {
    DFAarc::chars = chars;
    DFAarc::destNode = destNode;
    DFAarc::allBut = allBut;
}

void DFAnode::createArcTo(const char* chars, DFAnode* node, bool allBut) {
    DFAnode::adjArcs.push_back(DFAarc(chars, node, allBut));
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
            Lexer::updateLineAndCol(Lexer::input[i]);
            currNode = nextNode;
        } else {
            Lexer::handleFinalState(currNode->state, text);
            currNode = DFA[START];
            i--;
            text = "";
        }
    }
    Lexer::handleFinalState(currNode->state, text);
    Lexer::tokens.push_back(Token(END, "<EOF>", line, collumn, nullptr));
}

void Lexer::handleFinalState(State state, std::string text) {
    std::stringstream stream;
    if(state == ID_OR_KW) {
        state = idOrKw(text);
    }
    switch(state) {
    case DISCARD:
        break;
    case COMMENT:
        break;
    case MLCOMMEND:
        break;
    case MLCOMM:
        break;
    case MLCOMMST:
        break;
    case CHARLIT:
    case STRINGLIT:
        text.erase(text.begin());
        text.pop_back();
        Lexer::tokens.push_back(Token(STRINGLIT, text, Lexer::line, Lexer::collumn - text.length() - 2, new Obj<std::string>(STRING, text)));
        break;
    case INTLIT:
    case FLOATLIT:
        double num;
        stream<<text;
        stream>>num;
        Lexer::tokens.push_back(Token(FLOATLIT, text, Lexer::line, Lexer::collumn - text.length(), new Obj<double>(NUMBER, num)));
        break;
    case TRUE:
        Lexer::tokens.push_back(Token(FLOATLIT, text, Lexer::line, Lexer::collumn - text.length(), new Obj<double>(NUMBER, 1)));
        break;
    case FALSE:
        Lexer::tokens.push_back(Token(FLOATLIT, text, Lexer::line, Lexer::collumn - text.length(), new Obj<double>(NUMBER, 0)));
        break;

    //Errors handled after this point

    case FLOATLITERR:
        throw std::runtime_error("[ERROR] lexing float literal");
    case CHARLITERR:
        throw std::runtime_error("[ERROR] unclosed string");
    case STRLITERR:
        throw std::runtime_error("[ERROR] unclosed string");
    case START:
        throw std::runtime_error("[ERROR] forbidden characters used for identifiers");
    case INTLITERR:
        throw std::runtime_error("[ERROR] identifiers can't begin with a number");
    default:
        Lexer::tokens.push_back(Token(state, text, Lexer::line, Lexer::collumn - text.length(), nullptr));
        break;
    }
}

void Lexer::updateLineAndCol(char inputChar) {
    if(inputChar == '\n') {
        Lexer::line++;
        Lexer::collumn = 1;
    } else {
        Lexer::collumn++;
    }
}
