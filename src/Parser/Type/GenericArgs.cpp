#include "../Parser.hpp"
#include "TypeFmt.hpp"
#include "../../Data/Config.hpp"
#include <execution>
#include <expected>
#include <format>
#include <memory>
#include <algorithm>
#include <unordered_map>
#include <execution>

Type Parser::translateGenerics(const std::unordered_map<std::string, Type>& type_mappings, Type type) const{
    switch(type.catagory){
        case Type::TypeCatagory::POINTER:
        case Type::TypeCatagory::OPTION:
        case Type::TypeCatagory::SET:
        {
            auto& sub_data = std::get<std::shared_ptr<Type>>(type.sub_data);
            sub_data = std::make_shared<Type>(translateGenerics(type_mappings, *sub_data));
            return type;
        }
        case Type::TypeCatagory::UNDERLYING_TYPE_ACCESS:
        {
            auto& sub_data = std::get<Type::UnderlyingTypeAccessSubdata>(type.sub_data);
            auto sub_type = std::make_shared<Type>(translateGenerics(type_mappings, *sub_data.type));
            if(type.catagory == Type::TypeCatagory::UNDERLYING_TYPE_ACCESS
                && std::ranges::any_of(std::get<std::vector<Type>>(cfg::DIRECT_UNDERLYING_MATCH_TYPE.sub_data), [&](const Type& ty){return ty.catagory == sub_type->catagory;})
                && !sub_data.access_index.has_value()
            ){
                return translateUnderlyingType(*sub_type);
            }
            return type;
        }
        case Type::TypeCatagory::ARRAY:
        {
            auto& sub_type = std::get<Type::ArraySubdata>(type.sub_data).type;
            sub_type = std::make_shared<Type>(translateGenerics(type_mappings, *sub_type));
            return type;
        }
        case Type::TypeCatagory::STRUCTURE:
            throw "UNIMPLEMENTED.STRUCT.GENERIC";
        case Type::TypeCatagory::TUPLE:
        {
            std::vector<Type>& sub_data = std::get<std::vector<Type>>(type.sub_data);
            std::transform(std::execution::par, sub_data.cbegin(), sub_data.cend(), sub_data.begin(), [&](const Type& ty){return translateGenerics(type_mappings, ty);});
            return type;
        }
        case Type::TypeCatagory::MAP:
        case Type::TypeCatagory::RESULT:
            type.sub_data = std::make_pair<std::shared_ptr<Type>>(
                std::make_shared<Type>(translateGenerics(type_mappings, *std::get<std::pair<std::shared_ptr<Type>, std::shared_ptr<Type>>>(type.sub_data).first)),
                std::make_shared<Type>(translateGenerics(type_mappings, *std::get<std::pair<std::shared_ptr<Type>, std::shared_ptr<Type>>>(type.sub_data).second))
            );
            return type;
        case Type::TypeCatagory::ENUM:
            throw "UNIMPLEMENTED.ENUM.GENERIC";
        case Type::TypeCatagory::GENERIC:
        {
            auto& sub_data = std::get<Type::GenericSubdata>(type.sub_data);
            if(type_mappings.contains(sub_data->name)){
                return type_mappings.at(sub_data->name);
            }
            return type;
        }
        case Type::TypeCatagory::FUNCTION_PTR:
        {
            Type::FnPtrSubdata& sub_data = std::get<Type::FnPtrSubdata>(type.sub_data);
            std::transform(std::execution::par, sub_data.param_types.cbegin(), sub_data.param_types.cend(), sub_data.param_types.begin(), [&](const Type& ty){return translateGenerics(type_mappings, ty);});
            sub_data.return_type = std::make_shared<Type>(translateGenerics(type_mappings, *sub_data.return_type));
            return type;
        }
        default:
            return type;
    }
}

std::expected<std::vector<Type>, std::string> Parser::parseGenericArgs(
    std::vector<TokenRange>::iterator& it,
    std::vector<TokenRange>::iterator end
) noexcept{
    ++it;
    std::vector<Type> generic_args;
    while(it < end){
        const auto& type_res = parseType(it, end);
        if(!type_res.has_value())
            return std::unexpected{std::move(type_res.error())};
        generic_args.push_back(type_res.value());
        ++it;
        bool is_end = it >= M_token_ranges.end();
        bool is_none = (it-(M_token_ranges.end()-it)*is_end)->type == TokenRange::Type::NONE;
        if(!is_end&&is_none){
            if(Token::Type::COMMA == it->begin->type){
                ++it;
                continue;
            }
            if(Token::Type::OPERATOR == it->begin->type
                && (cfg::GENERIC_CLOSE == it->begin->value
                    || std::string{cfg::GENERIC_CLOSE} + cfg::GENERAL_ASSIGNMENT == it->begin->value)
            ) {
                break;
            }
        }
        return std::unexpected{
            std::format("On line: {}, expected \'>\' to close the generic argument list, got: {}.", ((it-is_end)->begin-!is_none)->line, is_end ? cfg::EOF_MSG : (it->begin-!is_none)->value)
        };
    };
    return std::move(generic_args);
}
