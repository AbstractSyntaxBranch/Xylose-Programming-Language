#include "Token.hpp"
#include <format>
#include <print>
#include <utility>

std::string getTokenTypeAsString(Token::Type type){
    switch (type) {
        case Token::Type::SINGLE_TOKEN_KEYWORD:
            return "Single-Token Keyword";
        case Token::Type::TYPE_AND_VALUE_KEYWORD:
            return "Type/value keyword";
        case Token::Type::TYPE_KEYWORD:
            return "Type keyword";
        case Token::Type::OTHER_KEYWORD:
            return "Other keyword";
        case Token::Type::ELSE_LIKE_KEYWORD:
            return "Else-like keyword";
        case Token::Type::FUNCTION_FLAG_KEYWORD:
            return "Function flag keyword";
        case Token::Type::GLOBAL_FLAGGABLE_KEYWORD:
            return "Global flaggable keyword";
        case Token::Type::GLOBAL_NON_FLAGGABLE_KEYWORD:
            return "Global non-flaggable keyword";
        case Token::Type::GLOBAL_FLAG_KEYWORD:
            return "Global flag keyword";
        case Token::Type::VARIABLE_FLAG_KEYWORD:
            return "Variable flag keyword";
        case Token::Type::PREFIX_KEYWORD:
            return "Prefix keyword";
        case Token::Type::STATEMENT_KEYWORD:
            return "Statement keyword";
        case Token::Type::SWITCH_ITEM_KEYWORD:
            return "Switch item keyword";
        case Token::Type::VALUE_KEYWORD:
            return "Value keyword";
        case Token::Type::BRACKET_OPEN:
            return "Bracket opening";
        case Token::Type::BRACKET_CLOSE:
            return "Bracket closing";
        case Token::Type::CHAR:
            return "Character";
        case Token::Type::COMMA:
            return "Comma";
        case Token::Type::IDENTIFIER:
            return "Identifier";
        case Token::Type::NUMBER:
            return "Number";
        case Token::Type::OPERATOR:
            return "Operator";
        case Token::Type::SEMICOLON:
            return "Semicolon";
        case Token::Type::STRING:
            return "String";
        case Token::Type::PIPE:
            return "Pipe";
        default:
            std::unreachable();
    }
}

void printTokens(const std::vector<Token>& tokens){
    for(const Token& token : tokens){
        std::println("Token: \"{}\" with type: {}, on line {}. Has preferred type: {}.", token.value, getTokenTypeAsString(token.type), token.line, token.preferred_type.has_value());
    }
}
