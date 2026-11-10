#pragma once
#include <cstdint>
#include <string>
#include "Type/Type.hpp"

struct Token{
    enum class Type : std::uint8_t{
        TYPE_KEYWORD,
        ELSE_LIKE_KEYWORD,
        STATEMENT_KEYWORD,
        VARIABLE_FLAG_KEYWORD,
        FUNCTION_FLAG_KEYWORD,
        GLOBAL_FLAG_KEYWORD,
        SINGLE_TOKEN_KEYWORD,
        PREFIX_KEYWORD,
        OTHER_KEYWORD,
        GLOBAL_FLAGGABLE_KEYWORD,
        GLOBAL_NON_FLAGGABLE_KEYWORD,
        VALUE_KEYWORD,
        TYPE_AND_VALUE_KEYWORD,
        SWITCH_ITEM_KEYWORD,
        OPERATOR,
        STRING,
        BRACKET_OPEN,
        BRACKET_CLOSE,
        NUMBER,
        COMMA,
        CHAR,
        IDENTIFIER,
        SEMICOLON,
        PIPE
    } type;
    std::size_t line;
    std::string value;
    std::optional<::Type> preferred_type;
    bool operator==(const Token& other);
};

void printTokens(const std::vector<Token>& tokens);
