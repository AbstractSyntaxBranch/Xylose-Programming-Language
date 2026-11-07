#include "../Data/Config.hpp"
#include "Lexer.hpp"
#include <algorithm>
#include <cctype>
#include <cstddef>
#include <ranges>

constexpr char DIGIT_FRACT_SEP = '.';

bool Lexer::isNumberChar(char c){
    return std::isdigit(c) || c == DIGIT_FRACT_SEP;
}

Token Lexer::getNumberToken(){
    Token token{.type = Token::Type::NUMBER, .line = M_line};
    while (isReadPosValid() && isNumberChar(*M_read_position)){
        if(*M_read_position == DIGIT_FRACT_SEP && token.value.contains(DIGIT_FRACT_SEP))
            return token;
        token.value.push_back(*M_read_position++);
    }

    const std::ptrdiff_t until_end = &M_source.back() - M_read_position + 1;
    const std::size_t min_extension_size = std::min(std::size_t(until_end), M_max_num_extension_size);
    const auto reverse_size_iter = std::views::reverse(std::views::iota(std::size_t(1), min_extension_size + 1));
    for(const std::size_t size : reverse_size_iter){
        const auto at_size_filter =
            std::views::filter(
                [&](const std::pair<std::string_view, Type> pair) {
                    return size == pair.first.size();
                }
            );
        const std::string possible_extension = {M_read_position, M_read_position + size};
        const auto at_size_extension_mappings = cfg::NUMBER_EXTENSION_MAPPINGS | at_size_filter | std::ranges::to<std::unordered_map>();
        if(at_size_extension_mappings.contains(possible_extension)){
            token.preferred_type = at_size_extension_mappings.at(possible_extension);
            M_read_position += size;
            break;
        }
    }
    return token;
}
