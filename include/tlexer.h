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
    STRING,
    BOOL,
    CHAR,
    INTLIT,
    TRUE,
    FALSE,
    FLOATLIT,
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
    ID_OR_KW,
    RETURN,
    PRINT,
    CLASS,
    INTLITERR,
    COLON,
    QMARK
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
    { STRING, "STRING" },
    { BOOL, "BOOL" },
    { CHAR, "CHAR" },
    { INTLIT, "INTLIT" },
    { TRUE, "TRUE" },
    { FALSE, "FALSE" },
    { FLOATLIT, "FLOATLIT" },
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
    { ID_OR_KW, "ID_OR_KW" },
    { RETURN, "RETURN" },
    { PRINT, "PRINT" },
    { CLASS, "CLASS" },
    { INTLITERR, "INTLITERR" },
    { COLON, "COLON" },
    { QMARK, "QMARK" }
};

class Token {
public:
    State type;
    std::string text;
    size_t line;
    size_t collumn;

    Token();
    Token(const State&, const std::string&, const size_t&, const size_t&);

    virtual void getValue(int&) = 0;
    virtual void getValue(double&) = 0;
    virtual void getValue(bool&) = 0;
    virtual void getValue(std::string&) = 0;
    virtual ~Token() {}
};

class BlankToken : public Token {
public:

    BlankToken(const State&, const std::string&, const size_t&, const size_t&);

    void getValue(int&) {}
    void getValue(double&) {}
    void getValue(bool&) {}
    void getValue(std::string&) {}
};

class IntToken : public Token {
public:
    int m_data;

    IntToken(const State&, const std::string&, const size_t&, const size_t&, const int&);

    void getValue(int&);
    void getValue(double&) {}
    void getValue(bool&) {}
    void getValue(std::string&) {}
};

class FloatToken : public Token {
public:
    double m_data;

    FloatToken(const State&, const std::string&, const size_t&, const size_t&, const double&);

    void getValue(int&) {}
    void getValue(double&);
    void getValue(bool&) {}
    void getValue(std::string&) {}
};

class BoolToken : public Token{
public:
    bool m_data;

    BoolToken(const State&, const std::string&, const size_t&, const size_t&, const bool&);

    void getValue(int&) {}
    void getValue(double&) {}
    void getValue(bool&);
    void getValue(std::string&) {}
};

class StringToken : public Token {
public:
    std::string m_data;

    StringToken(const State&, const std::string&, const size_t&, const size_t&, const std::string&);

    void getValue(int&) {}
    void getValue(double&) {}
    void getValue(bool&) {}
    void getValue(std::string&);
};

class Visitor {
public:
    Visitor() {}

    int intValue(Token*);
    double floatValue(Token*);
    std::string stringValue(Token*);
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
    std::vector<Token*> tokens;
    std::vector<DFAnode*> DFA;

    void handleFinalState(State, std::string);
    void updateLineAndCol(char);
public:
    Lexer(const char*);
    void lex();
    std::vector<Token*> getTokenList();
};

char* getInputFromFile(const char*);

#endif // TLEXER_H_INCLUDED
