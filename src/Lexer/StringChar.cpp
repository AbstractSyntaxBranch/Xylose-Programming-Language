#include "../Data/Config.hpp"
#include "Lexer.hpp"
#include <cassert>
#include <format>
#include <initializer_list>
#include <stdexcept>
#include <ranges>

char Lexer::getRegularEscapeCharacter(){
    if(isReadPosValid() && cfg::STRING_CHAR_REGULAR_ESCAPE_MAPPINGS.contains(*M_read_position))
        return cfg::STRING_CHAR_REGULAR_ESCAPE_MAPPINGS.at(*M_read_position);
    else throw std::runtime_error{
        std::format("Expected valid escape sequence on line: {}, got: \"\\{}\".", M_line, *M_read_position)
    };
}

//Fun tip: do not change this if not needed. It's a mess.
void Lexer::handleFmt(Token& current_token, std::vector<Token>& tokens, Token::Type type){
    current_token.line = M_line;
    current_token.type = type;
    tokens.push_back(current_token);
    tokens.append_range(
        std::initializer_list{
            Token{.type = Token::Type::OPERATOR, .line = M_line, .value = ".."},
            Token{.type = Token::Type::IDENTIFIER, .line = M_line, .value = "cast"},
            Token{.type = Token::Type::OPERATOR, .line = M_line, .value = "<"},
            Token{.type = Token::Type::BRACKET_OPEN, .line = M_line, .value = "["},
            Token{.type = Token::Type::TYPE_KEYWORD, .line = M_line, .value = "u8"},
            Token{.type = Token::Type::BRACKET_CLOSE, .line = M_line, .value = "]"},
            Token{.type = Token::Type::OPERATOR, .line = M_line, .value = ">"},
            Token{.type = Token::Type::BRACKET_OPEN, .line = M_line, .value = "("},
        }
    );
    const auto start_offset = M_read_position+1-M_source.data();
    const auto read_pos_index_cpy = M_read_position-M_source.data();
    const auto tokens_cpy = std::move(M_tokens);
    const auto source_cpy = std::move(M_source);
    M_source = "";
    M_tokens.clear();
    M_source.append_range(source_cpy | std::views::drop(start_offset));
    M_read_position = M_source.data();
    std::size_t indent = 1;
    while (getNextToken()) {
        if(M_tokens.back() == cfg::STRING_CHAR_FMT_TOKEN_START)
            ++indent;
        else if(M_tokens.back() == cfg::STRING_CHAR_FMT_TOKEN_END)
            --indent;
        if(!indent)
            break;
    }
    tokens.append_range(std::move(M_tokens) | std::views::take(M_tokens.size()-1));
    const auto new_read_pos_offset = M_read_position - M_source.data();
    M_tokens = std::move(tokens_cpy);
    M_source = std::move(source_cpy);
    M_read_position = M_source.data() + read_pos_index_cpy + new_read_pos_offset;
    tokens.append_range(
        std::initializer_list{
            Token{.type = Token::Type::BRACKET_CLOSE, .line = M_line, .value = ")"},
            Token{.type = Token::Type::OPERATOR, .line = M_line, .value = ".."},
        }
    );

    current_token.value = "";
}

void Lexer::appendEscapeCharacter(Token& current_token, std::vector<Token>& tokens, Token::Type type, bool allow_fmt){
    ++M_read_position;
    if (*(M_read_position) == cfg::STRING_CHAR_FMT_TOKEN_START.value.front() && allow_fmt) {
        handleFmt(current_token, tokens, type);
    }
    else
        current_token.value.push_back(getRegularEscapeCharacter());
}

std::vector<Token> Lexer::getEnclosedToken(char enclosure_symbol, Token::Type type, bool allow_fmt = false){
    assert(*M_read_position == enclosure_symbol);

    std::vector<Token> tokens;
    Token current_token;
    while(*++M_read_position != enclosure_symbol){
        if(!isReadPosValid() || *M_read_position == '\n')
            throw std::runtime_error{"On line: {}, expected \" to close the string."};
        if(*M_read_position == cfg::STRING_CHAR_ESCAPE_SYMBOL) {
            appendEscapeCharacter(current_token, tokens, type, allow_fmt);
        }
        else {
            current_token.value.push_back(*M_read_position);
        }
    }
    ++M_read_position;
    current_token.type = type;
    current_token.line = M_line;
    tokens.push_back(current_token);
    return tokens;
}

std::vector<Token> Lexer::getStringToken(){
    return getEnclosedToken(cfg::STRING_ENCLOSURE_SYMBOL, Token::Type::STRING, true);
}

std::vector<Token> Lexer::getCharToken(){
    return getEnclosedToken(cfg::CHAR_ENCLOSURE_SYMBOL, Token::Type::CHAR);
}
