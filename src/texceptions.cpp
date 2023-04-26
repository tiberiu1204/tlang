#include <texceptions.h>

RuntimeError::RuntimeError(const Token& token, const std::string& message) {
    RuntimeError::token = token;
    RuntimeError::message = message;
}

ReturnStmt::ReturnStmt(Object* value) {
    ReturnStmt::value = value;
}

void printErrorMsg(const Token& token, const std::string& message) {
    if(token.type == END) {
        std::cout<<"[ERROR] at end: "<<message<<"\n";
    } else {
        std::cout<<"[ERROR] line "<<token.line<<" collumn "<<token.collumn<<" at '"<<token.text<<"'"<<": Parse Error: "<<message<<"\n";
    }
}

ParseError error(const Token& token, const std::string& message) {
    printErrorMsg(token, message);
    return ParseError();
}
