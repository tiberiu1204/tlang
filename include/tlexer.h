#ifndef TLEXER_H_INCLUDED
#define TLEXER_H_INCLUDED
#include<vector>
#include<map>
#include<string>

enum State {
    START,
    END,
    PLUS,
    EQ,
    PLUSPLUS,
    PLUSEQ,
    MINUS,
    MINUSMINUS,
    MINUSEQ,
    STAR,
    STAREQ,
    SLASH,
    COMMENT,
    MLCOMM,
    MLCOMMST,
    MLCOMMEND,
    SLASHEQ,
    BACKSLASH,
    COMA,
    POINT,
    LBRACE,
    RBRACE,
    LPAREN,
    RPAREN,
    SEMICOLIN,
    INT,
    FLOAT,
    BIGINT,
    STRING,
    BOOL,
    CHAR,
    INTLIT,
    TRUE,
    FALSE,
    FLOATLIT,
    BIGINTLIT,
    STRINGLIT,
    STRLITERR,
    CHARLIT,
    CHARLITERR,
    LT,
    LTEQ,
    GT,
    GTEQ,
    EQEQ,
    IDENT,
    IF,
    ELSE,
    WHILE,
    FOR,
    OR,
    OROR,
    AND,
    ANDAND,
    NOT,
    NOTEQ,
    MOD,
    MODEQ,
    FLOATLITERR,
    DISCARD,
    ID_OR_KW
};

const std::map<State, std::string> stateMap = {
    { START, "START" },
    { END, "END" },
    { PLUS, "PLUS" },
    { EQ, "EQ" },
    { PLUSPLUS, "PLUSPLUS" },
    { PLUSEQ, "PLUSEQ" },
    { MINUS, "MINUS" },
    { MINUSMINUS, "MINUSMINUS" },
    { MINUSEQ, "MINUSEQ" },
    { STAR, "STAR" },
    { STAREQ, "STAREQ" },
    { SLASH, "SLASH" },
    { COMMENT, "COMMENT" },
    { MLCOMM, "MLCOMM" },
    { MLCOMMST, "MLCOMMST" },
    { MLCOMMEND, "MLCOMMEND" },
    { SLASHEQ, "SLASHEQ" },
    { BACKSLASH, "BACKSLASH" },
    { COMA, "COMA" },
    { POINT, "POINT" },
    { LBRACE, "LBRACE" },
    { RBRACE, "RBRACE" },
    { LPAREN, "LPAREN" },
    { RPAREN, "RPAREN" },
    { SEMICOLIN, "SEMICOLIN" },
    { INT, "INT" },
    { FLOAT, "FLOAT" },
    { BIGINT, "BIGINT" },
    { STRING, "STRING" },
    { BOOL, "BOOL" },
    { CHAR, "CHAR" },
    { INTLIT, "INTLIT" },
    { TRUE, "TRUE" },
    { FALSE, "FALSE" },
    { FLOATLIT, "FLOATLIT" },
    { BIGINTLIT, "BIGINTLIT" },
    { STRINGLIT, "STRINGLIT" },
    { STRLITERR, "STRLITERR" },
    { CHARLIT, "CHARLIT" },
    { CHARLITERR, "CHARLITERR" },
    { LT, "LT" },
    { LTEQ, "LTEQ" },
    { GT, "GT" },
    { GTEQ, "GTEQ" },
    { EQEQ, "EQEQ" },
    { IDENT, "IDENT" },
    { IF, "IF" },
    { ELSE, "ELSE" },
    { WHILE, "WHILE" },
    { FOR, "FOR" },
    { OR, "OR" },
    { OROR, "OROR" },
    { AND, "AND" },
    { ANDAND, "ANDAND" },
    { NOT, "NOT" },
    { NOTEQ, "NOTEQ" },
    { MOD, "MOD" },
    { MODEQ, "MODEQ" },
    { FLOATLITERR, "FLOATLITERR" },
    { DISCARD, "DISCARD" },
    { ID_OR_KW, "ID_OR_KW" }
};

struct Token {
    State type;
    std::string text;
    size_t line;
    size_t collumn;

    Token();
    Token(State, std::string, size_t, size_t);
};

struct DFAnode;

struct DFAarc {
    const char* chars;
    DFAnode* destNode;
    bool allBut;

    DFAarc(const char*, DFAnode*, bool);
};

struct DFAnode {
    State state;
    std::vector<DFAarc> adjArcs;

    DFAnode(State);

    void createArcTo(const char*, DFAnode*, bool allBut = false);
};

class Lexer {
private:
    const char* input;
    size_t line = 1;
    size_t collumn = 1;
    std::vector<Token> tokens;
    std::vector<DFAnode*> DFA;

    void handleFinalState(State, std::string);
    void updateLineAndCol(char);
public:
    Lexer(const char*);
    void lex();
    std::vector<Token> getTokenList();
};

char* getInputFromFile(const char*);

#endif // TLEXER_H_INCLUDED
