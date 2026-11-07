#include "../Parser.hpp"
#include <expected>
#include <format>

std::expected<Type, std::string> Parser::parseTupleType(
    const TokenRange& rng
) noexcept{
    std::vector<TokenRange> sub_rngs = splitIntoTokenRanges(rng.begin, rng.end);
    if(sub_rngs.empty())
        return std::unexpected{
            std::format("On line: {}, a 0-sized tuple was found, which are not allowed. Please use the dedicated void type.", (rng.begin-1)->line)
        };
    Type::TupleSubdata sub_data;
    bool prev_beg = false;
    Token::Type splitter_token_type = Token::Type::COMMA;
    for(std::vector<TokenRange>::iterator it = sub_rngs.begin(); it != sub_rngs.end(); ++it){
        if(prev_beg){
            prev_beg = false;
            if(it->type == TokenRange::Type::NONE && it->begin->type == Token::Type::PIPE)
                splitter_token_type = Token::Type::PIPE;
        }
        if(it != sub_rngs.begin()){
            bool is_none = it->type == TokenRange::Type::NONE;
            if(!is_none || it->begin->type != splitter_token_type)
                return std::unexpected{
                    std::format("On line: {}, a comma was expected in the tuple type, got: {}", (it->begin-is_none)->line, (it->begin-is_none)->value)
                };
            ++it;
        }else{
            prev_beg = true;
        }
        if(it >= sub_rngs.end())
            break;
        auto type_res = parseType(it, sub_rngs.end());
        if(!type_res.has_value())
            return std::unexpected{type_res.error()};
        sub_data.push_back(type_res.value());
    }
    if(sub_data.size() == 1)
        return sub_data.front();
    return Type{.catagory = splitter_token_type == Token::Type::COMMA ? Type::TypeCatagory::TUPLE : Type::TypeCatagory::GENERIC_OR, .sub_data = std::move(sub_data)};
}
