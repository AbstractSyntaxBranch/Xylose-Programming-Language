#include "../Parser.hpp"
#include "TypeFmt.hpp"
#include "../../Data/Config.hpp"
#include <expected>
#include <format>

std::expected<Type, std::string> Parser::parsePrefixOpType(
    std::vector<TokenRange>::iterator& it,
    std::vector<TokenRange>::iterator end
) noexcept{
    const std::string type_symb = it->begin->value;
    if(cfg::PREFIXED_OPERATOR_TYPE_MAPPINGS.contains(type_symb)){
        Type type = cfg::PREFIXED_OPERATOR_TYPE_MAPPINGS.at(type_symb);
        ++it;
        if(it == end)
            return std::unexpected{
                std::format("On line: {}, expected a sub-type for: {}.",
                    (it-1)->begin->line,
                    (it-1)->begin->value
                )
            };
        const auto subtype = parseType(it, end);
        if (!subtype.has_value())
            return subtype;
        if(cfg::RESULT_OP_TYPENAME == type_symb)
            type.sub_data = Type::ResultSubdata{
                std::make_shared<Type>(subtype.value()),
                std::make_shared<Type>(
                    Type{
                        .catagory = Type::TypeCatagory::ARRAY,
                        .sub_data = Type::ArraySubdata{
                            .type = std::make_shared<Type>(Type{.catagory = Type::TypeCatagory::U8})
                        }
                    }
                )
            };
        else
            type.sub_data = std::make_shared<Type>(subtype.value());
        return type;
    }
    else return std::unexpected{
        std::format("On line: {}, unknown prefixed-operator type: \"{}\".",
            it->begin->line,
            it->begin->value
        )
    };
}
