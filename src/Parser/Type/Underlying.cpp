#include "../Parser.hpp"
#include "TypeFmt.hpp"
#include "../../Data/Config.hpp"
#include <algorithm>
#include <expected>
#include <format>
#include <memory>
#include <string_view>

Type Parser::translateUnderlyingType(const Type& type) const{
    switch (type.catagory) {
        case Type::TypeCatagory::RESULT:
            return *std::get<Type::ResultSubdata>(type.sub_data).first;
        case Type::TypeCatagory::MAP:
            return *std::get<Type::MapSubdata>(type.sub_data).second;
        case Type::TypeCatagory::ARRAY:
            return *std::get<Type::ArraySubdata>(type.sub_data).type;
        case Type::TypeCatagory::FUNCTION_PTR:
            return *std::get<Type::FnPtrSubdata>(type.sub_data).return_type;
        default:
            return *std::get<std::shared_ptr<Type>>(type.sub_data);
    }
}

//Let's hope I will never have to touch this function again, it's removing my SANITY;
static bool getStackedUnderlyingCompat(const Type& type, Type match_type = cfg::DIRECT_UNDERLYING_MATCH_TYPE){
    if(type.catagory == Type::TypeCatagory::GENERIC_OR){
        const auto sub_data = std::get<Type::GenericOrSubdata>(type.sub_data);
        return std::ranges::all_of(sub_data, [&](const Type& type_){return getStackedUnderlyingCompat(type_, match_type);});
    }
    if(type.catagory != Type::TypeCatagory::UNDERLYING_TYPE_ACCESS)
        return false;
    match_type = cfg::getUnderlyingMatchType(std::make_shared<Type>(match_type));
    return
        match_type.getTypeCompatibility(*std::get<Type::UnderlyingTypeAccessSubdata>(type.sub_data).type)
        || getStackedUnderlyingCompat(*std::get<Type::UnderlyingTypeAccessSubdata>(type.sub_data).type, match_type);
}

std::expected<Type, std::string> Parser::parseUnderlyingValueType(
    std::vector<TokenRange>::iterator& it,
    std::vector<TokenRange>::iterator end
) noexcept{
    constexpr std::string_view closing_pipe_err = "On line {}, expected a closing pipe, got: {}.";
    ++it;
    if(it == end)
        return std::unexpected{
            std::format(closing_pipe_err, (it-1)->begin->line, cfg::EOF_MSG)
        };
    if(it->type != TokenRange::Type::NONE)
        return std::unexpected{
            std::format(closing_pipe_err, (it-1)->begin->line, (it->begin-1)->line, (it->begin-1)->value)
        };
    switch (it->begin->type) {
        case Token::Type::PIPE:
        {
            ++it;
            const auto sub_type = parseType(it, end);
            if(!sub_type.has_value())
                return sub_type;
            if(
                !cfg::DIRECT_UNDERLYING_MATCH_TYPE.getTypeCompatibility(sub_type.value())
                && !getStackedUnderlyingCompat(sub_type.value())
            )
                return std::unexpected{
                    std::format("On line {}, expected a 0-input sub-type access compatible type, got: {}.", it->begin->line, formatType(sub_type.value()))
                };
            if(
                std::ranges::any_of(std::get<std::vector<Type>>(cfg::DIRECT_UNDERLYING_MATCH_TYPE.sub_data), [&](const Type& ty){return ty.catagory == sub_type->catagory;})
            )
                return translateUnderlyingType(*sub_type);
            return Type{
                .catagory = Type::TypeCatagory::UNDERLYING_TYPE_ACCESS,
                .sub_data = Type::UnderlyingTypeAccessSubdata{
                    .type = std::make_shared<Type>(sub_type.value())
                }
            };
        }
        default:
            return std::unexpected{std::format(closing_pipe_err, it->begin->line, it->begin->value)};
    }
}
