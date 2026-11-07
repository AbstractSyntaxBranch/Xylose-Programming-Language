#include "Lexer.hpp"

Token Lexer::getSingleCharToken(Token::Type type){
    return {.type = type, .line = M_line, .value = {*M_read_position++}};
}
