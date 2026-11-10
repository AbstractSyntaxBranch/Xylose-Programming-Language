#include "../Parser.hpp"
#include "TypeFmt.hpp"
#include "../../Data/Config.hpp"
#include <expected>
#include <format>
#include <memory>

std::expected<Type, std::string> Parser::parseFnPtrType(
    std::vector<TokenRange>::iterator& it,
    std::vector<TokenRange>::iterator end
) noexcept{
    bool ignore_non_type_data = false;
    ++it;
    if(it->type != TokenRange::Type::REGULAR || it == end)
        return std::unexpected{
            std::format("Expected the argument types for the function ptr type on line: {}.", (it-1)->begin->line)
        };

    Type::FnPtrSubdata sub_data;
    std::vector<TokenRange> sub_rngs = splitIntoTokenRanges(it->begin, it->end);

    for(std::vector<TokenRange>::iterator it = sub_rngs.begin(); it != sub_rngs.end(); ++it){
        if(it != sub_rngs.begin()){
            bool is_none = it->type == TokenRange::Type::NONE;
            if(!is_none || it->begin->type != Token::Type::COMMA)
                return std::unexpected{
                    std::format("On line: {}, a comma was expected in the tuple type, got: {}", (it->begin-is_none)->line, (it->begin-is_none)->value)
                };
            ++it;
        }
        if(it >= sub_rngs.end())
            break;
        if(it->type == TokenRange::Type::NONE && it->begin->type == Token::Type::OPERATOR && it->begin->value == cfg::GENERIC_VARARG_SYMB){
             ignore_non_type_data = true;
            if(sub_rngs.end() - it > 2){
                return std::unexpected{
                    std::format("On line: {}, an invalid generic vararg was used(\"..\"). Expected it to be at the end of the function inputs.", it->begin->line, it->begin->value)
                };
            }
        }else{
            auto type_res = parseType(it, sub_rngs.end());
            if(!type_res.has_value())
                return std::unexpected{type_res.error()};
            sub_data.param_types.push_back(type_res.value());
        }
    }
    ++it;
    if(
        it == end
        || it->type != TokenRange::Type::NONE
        || it->begin->type != Token::Type::OPERATOR
        || it->begin->value != cfg::RETURN_OPERATOR
    )
        return std::unexpected{
            std::format("On line: {}, expected a return value for the function pointer type.", ((it-(it==end))->begin-1)->line)
        };
    ++it;
    const auto ret_type = parseType(it, end);
    if(!ret_type.has_value())
        return ret_type;
    sub_data.return_type = std::make_shared<Type>(ret_type.value());
    return Type{.catagory = Type::TypeCatagory::FUNCTION_PTR, .sub_data = sub_data, .ignore_non_type_details = ignore_non_type_data};
}
