#include "../Parser.hpp"
#include <expected>
#include <format>

std::expected<Type, std::string> Parser::parseDynamicType(
    std::vector<TokenRange>::iterator& it,
    std::vector<TokenRange>::iterator end
) noexcept{
    ++it;
    const auto type_ = parseType(it, end);
    if(!type_.has_value())
        return type_;
    Type type = type_.value();
    switch (type.catagory) {
        case Type::TypeCatagory::GENERIC_ANY:
            type.catagory = Type::TypeCatagory::DYN_ANY;
            break;
        case Type::TypeCatagory::GENERIC_OR:
            type.catagory = Type::TypeCatagory::DYN_OR;
            break;
        default:
            return std::unexpected{
                std::format("On line: {}, an invalid dynamic construct was found. The only allowed constructs are (... | ...) and any.", (it->end-1)->line)
            };
    }
    return type;
}
