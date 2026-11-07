#include "Lexer.hpp"

bool Lexer::isReadPosValid(){
    return M_read_position <= &M_source.back();
}
