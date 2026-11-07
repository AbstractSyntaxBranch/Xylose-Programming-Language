#pragma once
#include "Token.hpp"

struct ASTToken : Token{
    std::vector<Token> children;
};
