#include "Parser.hpp"
#include "../Data/Config.hpp"
#include <cstdint>
#include <format>
#include <stdexcept>
#include <vector>

TokenRange::Type getAndValidateRangeType(const std::vector<Token>::iterator token_open, const std::vector<Token>::iterator token_close){
    for (const auto& [open, close, type] : cfg::NORMAL_BRACKETS) {
        if (open == token_open->value.front() && close == token_close->value.front())
            return type;
    }
    throw std::runtime_error{
        std::format("On line: {}, mismatched closing bracket found to match {}. Got: {}.", token_close->line, token_open->value, token_close->value)
    };
}

std::vector<TokenRange> Parser::splitIntoTokenRanges(const std::vector<Token>::iterator begin, const std::vector<Token>::iterator end) {
    std::vector<TokenRange> token_ranges;
    std::uint32_t indent = 0;
    TokenRange current_token_range{.begin = begin};
    std::vector<Token>::iterator opening;
    for (std::vector<Token>::iterator token = begin; token != end; ++token) {
        bool is_bracket_open = token->type == Token::Type::BRACKET_OPEN;
        bool is_bracket_close = token->type == Token::Type::BRACKET_CLOSE;
        if (is_bracket_open){
            if(!indent)
                opening = token;
            ++indent;
        }
        if (is_bracket_close) {
            if(indent)
                --indent;
            else throw std::runtime_error{
                std::format("On line: {}, found closing bracket: {}, but no brackets where opened.", token->line, token->value)
            };
        }
        if (indent == 0) {
            current_token_range.type = is_bracket_close ? getAndValidateRangeType(current_token_range.begin, token) : TokenRange::Type::NONE;
            current_token_range.end = token-is_bracket_close+1;
            current_token_range.begin += is_bracket_close;
            token_ranges.push_back(current_token_range);
            current_token_range.begin = token + 1;
        }
    }
    if (indent)
        throw std::runtime_error{
            std::format("Expected closing bracket to match {} on line: {}.", opening->value, opening->line)
        };
    return std::move(token_ranges);
}

void Parser::groupIntoGlobalTokenRanges() {
    M_token_ranges = std::move(splitIntoTokenRanges(M_tokens.begin(), M_tokens.end()));
}
