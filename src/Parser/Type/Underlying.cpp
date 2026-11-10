#include "../Parser.hpp"
#include "TypeFmt.hpp"
#include "../../Data/Config.hpp"
#include <algorithm>
#include <cstdlib>
#include <expected>
#include <format>
#include <memory>
#include <print>
#include <string>
#include <string_view>

Type Parser::translateUnderlyingType(const Type::UnderlyingTypeAccessSubdata& sub_data) const{
    if(sub_data.access_index == Type::UnderlyingTypeAccessSubdata::RES_ERROR){
        return *std::get<Type::MapSubdata>(sub_data.type->sub_data).second;
    }else if(sub_data.access_index == Type::UnderlyingTypeAccessSubdata::MAP_KEY){
        return *std::get<Type::MapSubdata>(sub_data.type->sub_data).first;
    } else if(!sub_data.access_index) {
        switch (sub_data.type->catagory) {
            case Type::TypeCatagory::RESULT:
                return *std::get<Type::ResultSubdata>(sub_data.type->sub_data).first;
            case Type::TypeCatagory::MAP:
                return *std::get<Type::MapSubdata>(sub_data.type->sub_data).second;
            case Type::TypeCatagory::ARRAY:
                return *std::get<Type::ArraySubdata>(sub_data.type->sub_data).type;
            case Type::TypeCatagory::FUNCTION_PTR:
                return *std::get<Type::FnPtrSubdata>(sub_data.type->sub_data).return_type;
            default:
                return *std::get<std::shared_ptr<Type>>(sub_data.type->sub_data);
        }
    } else{
        if(sub_data.type->catagory == Type::TypeCatagory::FUNCTION_PTR){
            return std::get<Type::FnPtrSubdata>(sub_data.type->sub_data).param_types.at(*sub_data.access_index);
        } else {
            return std::get<std::vector<Type>>(sub_data.type->sub_data).at(*sub_data.access_index);
        }
    }
}

//Let's hope I will never have to touch this function again, it's removing my SANITY;
static bool getStackedUnderlyingCompat(const Type& type, Type match_type = cfg::DIRECT_UNDERLYING_MATCH_TYPE){
    std::println("Type: {}\n", formatType(match_type));
    if(type.catagory == Type::TypeCatagory::GENERIC_OR){
        const auto sub_data = std::get<Type::GenericOrSubdata>(type.sub_data);
        return std::ranges::all_of(sub_data, [&](const Type& type_){return getStackedUnderlyingCompat(type_, match_type);});
    }
    if(type.catagory != Type::TypeCatagory::UNDERLYING_TYPE_ACCESS)
        return false;
    const auto& sub_data = std::get<Type::UnderlyingTypeAccessSubdata>(type.sub_data);
    if(!sub_data.access_index)
        match_type = cfg::getDirectUnderlyingMatchType(std::make_shared<Type>(match_type));
    else{
        if(*sub_data.access_index == Type::UnderlyingTypeAccessSubdata::RES_ERROR)
            match_type = cfg::getErrorUnderlyingMatchType(std::make_shared<Type>(match_type));
        else if(*sub_data.access_index == Type::UnderlyingTypeAccessSubdata::MAP_KEY)
            match_type = cfg::getKeyUnderlyingMatchType(std::make_shared<Type>(match_type));
        else
            match_type = cfg::getIndexedUnderlyingMatchType(match_type, *sub_data.access_index);
    }
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
            std::format(closing_pipe_err, (it->begin-1)->line, (it->begin-1)->value)
        };
    switch (it->begin->type) {
        case Token::Type::IDENTIFIER:
        {
            if(it->begin->value != cfg::KEY_ACCESS_NAME_UNDERLYING)
                break;
            ++it;
            bool is_end = it == end;
            if(is_end || it->begin->type != Token::Type::PIPE){
                return std::unexpected{
                    std::format(closing_pipe_err, (it-(is_end))->begin->line, is_end ? cfg::EOF_MSG : it->begin->value)
                };
            }
            ++it;
            const auto sub_type = parseType(it, end);
            if(!sub_type.has_value())
                return sub_type;
            if(sub_type->catagory == Type::TypeCatagory::MAP)
                return *std::get<Type::ResultSubdata>(sub_type->sub_data).first;
            if(!cfg::KEY_UNDERLYING_MATCH_TYPE.getTypeCompatibility(*sub_type) && !getStackedUnderlyingCompat(sub_type.value(), cfg::KEY_UNDERLYING_MATCH_TYPE)){
                return std::unexpected{
                    std::format("On line {}, expected a Map type, got: {}.", it->begin->line, formatType(sub_type.value()))
                };
            }
            return Type{
                .catagory = Type::TypeCatagory::UNDERLYING_TYPE_ACCESS,
                .sub_data = Type::UnderlyingTypeAccessSubdata{
                    .type = std::make_shared<Type>(sub_type.value()),
                    .access_index = Type::UnderlyingTypeAccessSubdata::MAP_KEY
                }
            };
        }
        case Token::Type::OPERATOR:
        {
            if(it->begin->value != cfg::ERROR_OPERATOR)
                break;
            ++it;
            bool is_end = it == end;
            if(is_end || it->begin->type != Token::Type::PIPE){
                return std::unexpected{
                    std::format(closing_pipe_err, (it-(is_end))->begin->line, is_end ? cfg::EOF_MSG : it->begin->value)
                };
            }
            ++it;
            const auto sub_type = parseType(it, end);
            if(!sub_type.has_value())
                return sub_type;
            if(sub_type->catagory == Type::TypeCatagory::RESULT)
                return *std::get<Type::ResultSubdata>(sub_type->sub_data).second;
            if(!cfg::ERROR_UNDERLYING_MATCH_TYPE.getTypeCompatibility(*sub_type) && !getStackedUnderlyingCompat(sub_type.value(), cfg::ERROR_UNDERLYING_MATCH_TYPE)){
                return std::unexpected{
                    std::format("On line {}, expected a Result type, got: {}.", it->begin->line, formatType(sub_type.value()))
                };
            }
            return Type{
                .catagory = Type::TypeCatagory::UNDERLYING_TYPE_ACCESS,
                .sub_data = Type::UnderlyingTypeAccessSubdata{
                    .type = std::make_shared<Type>(sub_type.value()),
                    .access_index = Type::UnderlyingTypeAccessSubdata::RES_ERROR
                }
            };
        }
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
                return translateUnderlyingType(Type::UnderlyingTypeAccessSubdata{.type = std::make_shared<Type>(*sub_type)});
            return Type{
                .catagory = Type::TypeCatagory::UNDERLYING_TYPE_ACCESS,
                .sub_data = Type::UnderlyingTypeAccessSubdata{
                    .type = std::make_shared<Type>(sub_type.value())
                }
            };
        }
        case Token::Type::NUMBER:
        {
            const std::size_t index = std::stoull(it->begin->value);
            const auto type_match = cfg::getIndexedUnderlyingMatchType(*cfg::any_ptr, index);
            ++it;
            bool is_end = it == end;
            if(is_end || it->begin->type != Token::Type::PIPE){
                return std::unexpected{
                    std::format(closing_pipe_err, (it-(is_end))->begin->line, is_end ? cfg::EOF_MSG : it->begin->value)
                };
            }
            ++it;
            const auto sub_type = parseType(it, end);
            if(!sub_type.has_value())
                return sub_type;
            if(
                !type_match.getTypeCompatibility(sub_type.value())
                && !getStackedUnderlyingCompat(sub_type.value(), type_match)
            )
                return std::unexpected{
                    std::format("On line {}, expected an indexed sub-type access compatible type with index support for {}, got: {}.", it->begin->line, index, formatType(sub_type.value()))
                };
            if(
                std::ranges::any_of(std::get<std::vector<Type>>(type_match.sub_data), [&](const Type& ty){return ty.catagory == sub_type->catagory;})
            )
                return translateUnderlyingType(Type::UnderlyingTypeAccessSubdata{.type = std::make_shared<Type>(*sub_type), .access_index = index});
            return Type{
                .catagory = Type::TypeCatagory::UNDERLYING_TYPE_ACCESS,
                .sub_data = Type::UnderlyingTypeAccessSubdata{
                    .type = std::make_shared<Type>(sub_type.value()),
                    .access_index = index
                },
            };
        }
        default: break;
    }
    return std::unexpected{std::format(closing_pipe_err, it->begin->line, it->begin->value)};
}
