#include "../Data/Config.hpp"
#include "Lexer.hpp"
#include <cctype>

bool Lexer::isIdenOrKwChar(char c){
    return !isBracketCloseChar(c) && !isBracketOpenChar(c) && !isOperatorNonAlphaStart(c) && !std::isspace(c) && c != ';'&& c != ',' && c != '|' && c != cfg::COMMENT_CHAR;
}

#define TEST_FOR_KEYWORD(keyword_type) \
    if(std::ranges::find(cfg::keyword_type##S, token.value) != cfg::keyword_type##S.end()){ \
        token.type = Token::Type::keyword_type; \
        return token; \
    }
Token Lexer::getIdentifierOrKeywordToken(){
    Token token{.type = Token::Type::IDENTIFIER, .line = M_line};
    while(isReadPosValid() && isIdenOrKwChar(*M_read_position)){
        token.value.push_back(*M_read_position++);
    }

    for (const auto& operator_group : cfg::OPERATORS) {
        for (const auto& operator_ : operator_group) {
            if(token.value == operator_.name){
                token.type = Token::Type::OPERATOR;
                return token;
            }
        }
    }

    if(cfg::TYPE_KEYWORDS.contains(token.value)){
        token.type = Token::Type::TYPE_KEYWORD;
        return token;
    }
    if(cfg::TYPE_AND_VALUE_KEYWORDS.contains(token.value)){
        token.type = Token::Type::TYPE_AND_VALUE_KEYWORD;
        return token;
    }
    TEST_FOR_KEYWORD(ELSE_LIKE_KEYWORD)
    TEST_FOR_KEYWORD(STATEMENT_KEYWORD)
    TEST_FOR_KEYWORD(VARIABLE_FLAG_KEYWORD)
    TEST_FOR_KEYWORD(FUNCTION_FLAG_KEYWORD)
    TEST_FOR_KEYWORD(GLOBAL_FLAG_KEYWORD)
    TEST_FOR_KEYWORD(SINGLE_TOKEN_KEYWORD)
    TEST_FOR_KEYWORD(PREFIX_KEYWORD)
    TEST_FOR_KEYWORD(GLOBAL_FLAGGABLE_KEYWORD)
    TEST_FOR_KEYWORD(GLOBAL_NON_FLAGGABLE_KEYWORD)
    TEST_FOR_KEYWORD(VALUE_KEYWORD)
    TEST_FOR_KEYWORD(OTHER_KEYWORD)
    TEST_FOR_KEYWORD(SWITCH_ITEM_KEYWORD)

    return token;
}
#undef TEST_FOR_KEYWORD
