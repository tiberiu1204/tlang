#include <texceptions.h>

RuntimeError::RuntimeError(Token token, const std::string& message) {
    RuntimeError::token = token;
    RuntimeError::message = message;
}
