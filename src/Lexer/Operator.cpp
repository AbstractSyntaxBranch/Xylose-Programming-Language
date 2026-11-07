#include "../Data/Config.hpp"
#include "Lexer.hpp"
#include <algorithm>
#include <cstddef>
#include <format>
#include <ranges>
#include <stdexcept>

bool Lexer::isOperatorNonAlphaStart(char c){
    return std::ranges::any_of(cfg::OPERATORS, [&](const auto& operator_group){
        return std::ranges::any_of(operator_group, [&](const auto& operator_){
            return operator_.name.front() == c && !std::isalpha(c);
        });
    });
}

Token Lexer::getOperatorToken(){
    Token token{.type = Token::Type::OPERATOR, .line = M_line};

    const std::ptrdiff_t until_end = &M_source.back() - M_read_position + 1;
    const std::size_t min_operator_size = std::min(std::size_t(until_end), M_max_operator_size);
    const auto reverse_size_iter = std::views::reverse(std::views::iota(std::size_t(1), min_operator_size + 1));

    for(const std::size_t size : reverse_size_iter){
        const std::string possible_operator = {M_read_position, M_read_position + size};
        const auto at_size_filter =
            std::views::filter(
                [&](const Operator& operator_) {
                    return size == operator_.name.size();
                }
            );
        for (const auto& operator_group : cfg::OPERATORS) {
            for (const auto& operator_ : operator_group | at_size_filter) {
                if (operator_.name == possible_operator) {
                    token.value = possible_operator;
                    M_read_position += size;
                    goto end;
                }
            }
        }
    }
    end:
    if (token.value.empty()) {
        std::string operator_;
        while (isReadPosValid() && isOperatorNonAlphaStart(*M_read_position)) {
            operator_.push_back(*M_read_position++);
        }
        throw std::runtime_error{
            std::format("Unknown operator: \"{}\", on line: {}.", operator_, M_line)
        };
    }
    return token;
}
