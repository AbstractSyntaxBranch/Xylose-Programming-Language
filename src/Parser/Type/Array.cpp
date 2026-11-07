#include "../Parser.hpp"
#include "../../Data/Config.hpp"
#include <expected>
#include <format>
#include <string>
#include <vector>

std::expected<Type, std::string> Parser::parseArrayType(
    const TokenRange& rng
) noexcept {
    if(rng.end - rng.begin == 0)
        return std::unexpected{
            std::format("On line: {}, the type expression enclosed in square brackets failed to parse, because they're empty. Unknown type.", rng.begin->line)
        };
    std::vector<TokenRange> sub_rngs = splitIntoTokenRanges(rng.begin, rng.end);
    decltype(sub_rngs)::iterator it = sub_rngs.begin();

    Type type{
        .catagory = Type::TypeCatagory::ARRAY,
    };
    const auto& subtype = parseType(it, sub_rngs.end());
    if(!subtype)
        return std::unexpected{subtype.error()};
    Type::ArraySubdata subdata{
        .type = std::make_shared<Type>(subtype.value()),
        .size = {}
    };
    ++it;
    if(it != sub_rngs.end() && it->type == TokenRange::Type::NONE && it->begin->type == Token::Type::SEMICOLON){
        //sized array
        ++it;
        bool is_eof = it == sub_rngs.end();
        bool is_valid_size = is_eof ? false : (it->type == TokenRange::Type::NONE && it->begin->type == Token::Type::NUMBER);
        if(!is_valid_size){
            return std::unexpected{
               std::format("On line: {}, expected the array size, got: {}",
                   ((it-is_eof)->end-1)->line,
                   is_eof ? cfg::EOF_MSG : (it->begin-(it->type == TokenRange::Type::NONE))->value)
           };
        }
        if(it->begin->value.contains('.'))
            return std::unexpected{std::format("On line: {}, found a non-integer array size: {}.", it->begin->line, it->begin->value)};
        subdata.size = std::stoull(it->begin->value);
    }
    type.sub_data = std::move(subdata);
    return std::move(type);
}
