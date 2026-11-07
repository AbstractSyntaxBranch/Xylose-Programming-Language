#include "../../Data/Config.hpp"
#include "../../Data/Diagnostics.hpp"
#include "../Parser.hpp"
#include "../Type/TypeFmt.hpp"
#include <format>
#include <memory>
#include <fmt/printf.h>
#include <stdexcept>
#include <utility>

void Parser::parseGlobals() {
    std::set<std::string> current_flags;
    std::set<std::string> current_fn_flags;
    for (decltype(M_token_ranges)::iterator it = M_token_ranges.begin(); it != M_token_ranges.end(); ++it) {
        constexpr std::string_view non_global_err = "On line: {}, a valid global expression was expected. Got: \"{}\".";
        if(it->type != TokenRange::Type::NONE)
            throw std::runtime_error{
                std::format(non_global_err, (it->begin-1)->line, (it->begin-1)->value)
            };

        switch (it->begin->type) {
        case Token::Type::GLOBAL_FLAG_KEYWORD:
            current_flags.emplace(it->begin->value);
            break;
        case Token::Type::FUNCTION_FLAG_KEYWORD:
            current_fn_flags.emplace(it->begin->value);
            break;
        case Token::Type::GLOBAL_NON_FLAGGABLE_KEYWORD:
            throw std::runtime_error{
                std::format("On line: {}, global statement does not support flags.", it->begin->line)
            };
            break;
        case Token::Type::GLOBAL_FLAGGABLE_KEYWORD:
            if (cfg::FUNCTION_KEYWORD == it->begin->value){
                auto fn_res = parseFunction(it);
                bool exported = current_flags.contains(std::string{cfg::EXPORT_KEYWORD});
                current_flags.erase(std::string{cfg::EXPORT_KEYWORD});
                fn_res.flags = std::move(current_flags);
                fn_res.flags.insert_range(std::move(current_fn_flags));
                M_functions.push_back(std::make_pair(exported, std::make_shared<Function>(fn_res)));
                DIAG_DBG(
                    diagFn();
                )
            }
            if(cfg::TYPEDEF_KEYWORD == it->begin->value){
                auto typedef_ = parseTypedef(it);
                bool exported = current_flags.contains(std::string{cfg::EXPORT_KEYWORD});
                current_flags.erase(std::string{cfg::EXPORT_KEYWORD});
                typedef_.flags = std::move(current_flags);
                if(!current_fn_flags.empty())
                    throw std::runtime_error{
                        std::format("On line: {}, typedef statement does not support function flags.", it->begin->line)
                    };
                M_typedefs.push_back(std::make_pair(exported, std::make_shared<Typedef>(typedef_)));
                DIAG_DBG(
                    diagTypedef();
                )
            }
            else if (!current_fn_flags.empty()){
                throw std::runtime_error{
                    std::format("On line: {}, incompatible flags(which are specialized for functions): {}.", it->begin->line, current_fn_flags)
                };
            }
            current_flags.clear();
            current_fn_flags.clear();
            break;
        default:
            throw std::runtime_error{
                std::format(non_global_err, it->begin->line, it->begin->value)
            };
        }
    }
    if (!current_flags.empty() || !current_fn_flags.empty())
        throw std::runtime_error{
            std::format("On line: {}, flags(for example: {}) were provided, yet nothing followed.", M_token_ranges.rbegin()->begin->line, M_token_ranges.rbegin()->begin->value)
        };
}
