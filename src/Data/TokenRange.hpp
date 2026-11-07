#pragma once
#include "Token.hpp"
#include <vector>

struct TokenRange{
    std::vector<Token>::iterator begin, end;
    enum class Type {
        NONE,
        REGULAR,
        CURLY,
        SQUARE,
    } type;
};
