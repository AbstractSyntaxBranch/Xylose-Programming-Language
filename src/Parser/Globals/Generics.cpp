#include "../Parser.hpp"
#include "../../Data/Config.hpp"
#include <format>
#include <memory>
#include <stdexcept>

GenericFields Parser::parseGenericFields(
    std::vector<TokenRange>::iterator& it
) {
    ++it;
    GenericFields generic_fields;
    while(it < M_token_ranges.end()){
        const auto old_it = it;
        const auto type = parseType(it, M_token_ranges.end());
        if(!type.has_value()){
            it = old_it;
            bool is_none = it->type == TokenRange::Type::NONE;
            if(!is_none || it->begin->type != Token::Type::IDENTIFIER)
                throw std::runtime_error{
                    std::format("Got invalid generic arguments on line: {}, got: {}, but expected a generic name.", (it->begin-!is_none)->line, (it->begin-!is_none)->value)
                };
            GenericField generic_field{.name = it->begin->value};
            ++it;
            if(it < M_token_ranges.end()
                && it->type == TokenRange::Type::NONE
                && it->begin->type == Token::Type::OPERATOR
                && it->begin->value == cfg::MAP_ITEM_SPLIT)
            {
                ++it;
                const auto& constraint_type = parseType(it, M_token_ranges.end());
                if(!constraint_type.has_value())
                    throw std::runtime_error{constraint_type.error()};
                generic_field.requirement = constraint_type.value();
            }
            else --it;
            generic_fields.push_back(std::make_shared<GenericField>(generic_field));
        }else{
            generic_fields.push_back(std::make_shared<GenericField>(GenericField{.requirement = type.value()}));
        }
        ++it;
        bool is_end = it >= M_token_ranges.end();
        bool is_none = (it-(M_token_ranges.end()-it)*is_end)->type == TokenRange::Type::NONE;
        if(!is_end&&is_none){
            if(Token::Type::COMMA == it->begin->type){
                ++it;
                continue;
            }
            if(Token::Type::OPERATOR == it->begin->type && (cfg::GENERIC_CLOSE == it->begin->value || std::string{cfg::GENERIC_CLOSE} + cfg::GENERAL_ASSIGNMENT == it->begin->value))
                break;
        }
        throw std::runtime_error{
            std::format("On line: {}, expected \'>\' to close the generic, got: {}.", ((it-is_end)->begin-!is_none)->line, is_end ? cfg::EOF_MSG : (it->begin-!is_none)->value)
        };
    }
    return generic_fields;
}
