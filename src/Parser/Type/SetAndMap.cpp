#include "../Parser.hpp"
#include "../../Data/Config.hpp"
#include <expected>
#include <format>
#include <memory>

std::expected<Type, std::string> Parser::parseSetOrMapType(
    const TokenRange& rng
) noexcept {
    if(rng.end == rng.begin)
        return std::unexpected{
            std::format("On line: {}, the type expression enclosed in curly brackets failed to parse, because they're empty. Unknown type.", rng.begin->line)
        };
    std::vector<TokenRange> sub_rngs = splitIntoTokenRanges(rng.begin, rng.end);
    auto it = sub_rngs.begin();
    auto subtype_res = parseType(it, sub_rngs.end());
    if(!subtype_res.has_value())
        return std::unexpected{subtype_res.error()};
    ++it;
    if(sub_rngs.end() == it)
        return Type{
            .catagory = Type::TypeCatagory::SET,
            .sub_data = std::make_shared<Type>(subtype_res.value())
        };
    bool is_end = it != sub_rngs.end();
    bool matches_split_char = is_end
        && it->type == TokenRange::Type::NONE
        && it->begin->type == Token::Type::OPERATOR
        && it->begin->value == cfg::MAP_ITEM_SPLIT;
    if(!matches_split_char)
        return std::unexpected{
            std::format("On line: {}, expected a {} to split the key and value types.", (it-is_end)->begin->line, cfg::MAP_ITEM_SPLIT)
        };
    ++it;
    if(sub_rngs.end() == it)
        return std::unexpected{
            std::format("On line: {}, expected a value type in the map type, but got none.", ((it-1)->end-1)->line)
        };
    auto subtype2_res = parseType(it, sub_rngs.end());
    if(!subtype2_res.has_value())
        return std::unexpected{subtype_res.error()};
    return Type{
        .catagory = Type::TypeCatagory::MAP,
            .sub_data = Type::MapSubdata{std::make_shared<Type>(subtype_res.value()), std::make_shared<Type>(subtype2_res.value())}
    };
}
