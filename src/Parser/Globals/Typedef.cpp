#include "../Parser.hpp"
#include "../../Data/Config.hpp"
#include <format>
#include <stdexcept>

Typedef Parser::parseTypedef(std::vector<TokenRange>::iterator& it) {
    ++it;
    bool is_end = it == M_token_ranges.end();
    bool is_none = it->type == TokenRange::Type::NONE;
    if(is_end || !is_none || it->begin->type != Token::Type::IDENTIFIER)
        throw std::runtime_error{
            std::format("On line: {}, expected the name of the alias/type definition, but got: {}.", ((it-is_end)->begin-!is_none)->line, is_end ? cfg::EOF_MSG : ((it->begin)-!is_none)->value)
        };
    Typedef typedef_;
    typedef_.alias = it->begin->value;
    ++it;
    is_end = it == M_token_ranges.end();
    is_none = it->type == TokenRange::Type::NONE;
    bool assignment_symb_already_found = false;
    if(!is_end && is_none && it->begin->type == Token::Type::OPERATOR && it->begin->value == cfg::GENERIC_OPEN){
        typedef_.generic_fields = parseGenericFields(it);
        assignment_symb_already_found = it->begin->value == std::string{cfg::GENERIC_CLOSE} + cfg::GENERAL_ASSIGNMENT;
        ++it;
        is_end = it == M_token_ranges.end();
        is_none = it->type == TokenRange::Type::NONE;
    }
    if(!assignment_symb_already_found){
        if(is_end || !is_none || it->begin->type != Token::Type::OPERATOR || it->begin->value != cfg::GENERAL_ASSIGNMENT)
            throw std::runtime_error{
                std::format("On line: {}, expected the definition of the alias/type definition, but got: \"{}\". Tip: add \'{}\'.", ((it-is_end)->begin-!is_none)->line, is_end ? cfg::EOF_MSG : ((it->begin)-!is_none)->value, cfg::GENERAL_ASSIGNMENT)
            };
        ++it;
    }
    M_defined_generics = std::move(typedef_.generic_fields);
    const auto type = parseType(it, M_token_ranges.end());
    typedef_.generic_fields = std::move(M_defined_generics);
    if(!type.has_value())
        throw std::runtime_error{type.error()};
    typedef_.type = type.value();
    return typedef_;
}
