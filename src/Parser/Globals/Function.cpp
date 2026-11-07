#include "../Parser.hpp"
#include "../../Data/Config.hpp"
#include <format>
#include <stdexcept>

Function Parser::parseFunction(std::vector<TokenRange>::iterator& it) {
    constexpr std::string_view name_err = "In the function declaration on line {} a function name was expected, got {}.";
    constexpr std::string_view body_err = "In the function declaration on line {} a function body was expected, got {}.";
    ++it;
    if (it == M_token_ranges.end())
        throw std::runtime_error{
            std::format(name_err, (it-1)->begin->line, cfg::EOF_MSG)
        };
    Function function;
    if(it->type != TokenRange::Type::NONE)
        throw std::runtime_error{
            std::format(name_err, (it->begin-1)->line, (it->begin-1)->value)
        };
    if(it->begin->type != Token::Type::IDENTIFIER)
        throw std::runtime_error{
            std::format(name_err, it->begin->line, it->begin->value)
        };
    function.name = it->begin->value;
    if(++it == M_token_ranges.end())
        throw std::runtime_error{
            std::format(body_err, (it-1)->begin->line, cfg::EOF_MSG)
        };
    bool matches_ret_kw = it->type == TokenRange::Type::NONE
        && it->begin->type == Token::Type::OPERATOR
        && it->begin->value == cfg::RETURN_OPERATOR;
    if(matches_ret_kw){
        const auto ret_type = parseType(++it, M_token_ranges.end());
        if(!ret_type)
            throw std::runtime_error{ret_type.error()};
        function.return_type = ret_type.value();
    }
    return function;
}
