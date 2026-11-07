#include "Token.hpp"

bool Token::operator==(const Token& other){
    return this->value == other.value && this->type == other.type;
}
