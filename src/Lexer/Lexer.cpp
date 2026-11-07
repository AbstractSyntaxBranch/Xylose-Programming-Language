#include "../Data/Config.hpp"
#include "Lexer.hpp"
#include <cctype>

inline std::size_t getMaxNumExtensionSize(){
    std::size_t max_extension_size = 0;
    for(const auto& [extension, type] : cfg::NUMBER_EXTENSION_MAPPINGS){
        max_extension_size = std::max(max_extension_size, extension.size());
    }
    return max_extension_size;
}

consteval std::size_t getMaxOperatorSize(){
    std::size_t max_operator_size = 0;
    for(const auto& operator_group : cfg::OPERATORS){
        for(const auto& operator_ : operator_group){
            max_operator_size = std::max(max_operator_size, operator_.name.size());
        }
    }
    return max_operator_size;
}

Lexer::Lexer(std::string& source) : M_source(source){
    M_read_position = M_source.data();
    M_max_num_extension_size = getMaxNumExtensionSize();
    M_max_operator_size = getMaxOperatorSize();
}

bool Lexer::getNextToken(){
    if (!isReadPosValid())
        return false;
    if (std::isspace(*M_read_position)) {
        M_line += *M_read_position == '\n';
        ++M_read_position;
        return getNextToken();
    }
    if(*M_read_position == cfg::COMMENT_CHAR){
        while(isReadPosValid() && *M_read_position != '\n')
            ++M_read_position;
        return getNextToken();
    }
    if (isBracketOpenChar(*M_read_position)) {
        M_tokens.push_back(getSingleCharToken(Token::Type::BRACKET_OPEN));
        return true;
    }

    if (isBracketCloseChar(*M_read_position)) {
        M_tokens.push_back(getSingleCharToken(Token::Type::BRACKET_CLOSE));
        return true;
    }

    if (isOperatorNonAlphaStart(*M_read_position)) {
        M_tokens.push_back(getOperatorToken());
        return true;
    }

    if (std::isdigit(*M_read_position)) {
        M_tokens.push_back(getNumberToken());
        return true;
    }

    if (*M_read_position == cfg::STRING_ENCLOSURE_SYMBOL) {
        M_tokens.append_range(getStringToken());
        return true;
    }

    if (*M_read_position == cfg::CHAR_ENCLOSURE_SYMBOL) {
        M_tokens.append_range(getCharToken());
        return true;
    }

    if (*M_read_position == ',') {
        M_tokens.push_back(getSingleCharToken(Token::Type::COMMA));
        return true;
    }

    if (*M_read_position == ';') {
        M_tokens.push_back(getSingleCharToken(Token::Type::SEMICOLON));
        return true;
    }

    if (*M_read_position == '|') {
        M_tokens.push_back(getSingleCharToken(Token::Type::PIPE));
        return true;
    }

    M_tokens.push_back(getIdentifierOrKeywordToken());
    return true;

}

[[nodiscard]] std::vector<Token> Lexer::splitIntoTokens() {
    while (getNextToken());
    return std::move(M_tokens);
}
