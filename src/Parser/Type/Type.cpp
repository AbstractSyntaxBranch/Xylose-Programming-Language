#include "../Parser.hpp"
#include "../../Data/Config.hpp"
#include "TypeFmt.hpp"
#include <cstdint>
#include <expected>
#include <format>
#include <memory>
#include <utility>

std::expected<Type, std::string> Parser::parseTypeBase(
        std::vector<TokenRange>::iterator& it,
        std::vector<TokenRange>::iterator end
) noexcept {
    if(it >= end)
        return std::unexpected {
            std::format("Expected type on line {}, got {}", (((it-1)->end)-1)->line, cfg::EOF_MSG)
        };
    std::uint32_t line = (it->end-(it->type == TokenRange::Type::NONE))->line;
    switch (it->type) {
        case TokenRange::Type::SQUARE:
            return std::move(Parser::parseArrayType(*it));
        case TokenRange::Type::NONE:
            switch (it->begin->type) {
                case Token::Type::TYPE_AND_VALUE_KEYWORD:
                    return cfg::TYPE_AND_VALUE_KEYWORDS.at(it->begin->value);
                case Token::Type::TYPE_KEYWORD:
                    return cfg::TYPE_KEYWORDS.at(it->begin->value);
                case Token::Type::OPERATOR:
                    return Parser::parsePrefixOpType(it, end);
                case Token::Type::PIPE:
                    return Parser::parseUnderlyingValueType(it, end);
                case Token::Type::GLOBAL_FLAGGABLE_KEYWORD:
                    if(it->begin->value == cfg::FUNCTION_KEYWORD)
                        return Parser::parseFnPtrType(it, end);
                    break;
                case Token::Type::IDENTIFIER:
                    return parseNamedType(it, end);
                case Token::Type::OTHER_KEYWORD:
                    if(it->begin->value == cfg::DYN_KEYWORD)
                        return parseDynamicType(it, end);
                    break;
                default: break;
            }
            return std::unexpected {
                std::format("On line: {}, unknown type found: \"{}\".", it->begin->line, it->begin->value)
            };
        case TokenRange::Type::CURLY:;
            return Parser::parseSetOrMapType(*it);
        case TokenRange::Type::REGULAR:
            return Parser::parseTupleType(*it);
    }
    std::unreachable();
}

std::expected<Type, std::string> Parser::parseType(
        std::vector<TokenRange>::iterator& it,
        std::vector<TokenRange>::iterator end
) noexcept {
    const auto type = parseTypeBase(it, end);
    if(!type.has_value())
        return type;
    const auto old_it = it;
    ++it;
    auto extension_type = parseType(it, end);
    if(!extension_type.has_value() || extension_type.value().catagory != Type::TypeCatagory::RESULT){
        it = old_it;
        return type;
    }
    const auto& sub_data = std::get<Type::ResultSubdata>(extension_type->sub_data);
    extension_type->sub_data = Type::ResultSubdata{sub_data.first, std::make_shared<Type>(type.value())};
    return extension_type;
}
