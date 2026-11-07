#include "../Data/Config.hpp"
#include "Lexer.hpp"
#include <cassert>

bool Lexer::isBracketOpenChar(char c){
    for (const auto& [start, _, _] : cfg::NORMAL_BRACKETS) {
        if(c == start)
            return true;
    }
    return false;
}

bool Lexer::isBracketCloseChar(char c){
    for (const auto& [_, end, _] : cfg::NORMAL_BRACKETS) {
        if(c == end)
            return true;
    }
    return false;
}
