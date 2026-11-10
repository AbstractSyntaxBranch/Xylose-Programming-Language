#include "../Parser.hpp"
#include "../../Data/Config.hpp"
#include "TypeFmt.hpp"
#include <algorithm>
#include <print>
#include <ranges>
#include <deque>
#include <expected>
#include <format>
#include <memory>
#include <ranges>

Type Parser::parseGenericNamedType(const std::vector<Type>& generic_args, std::shared_ptr<Typedef> typedef_) const{
    const auto type_mappings = std::views::zip(
        typedef_->generic_fields
        | std::views::transform([](const std::shared_ptr<GenericField>& e){return e->name;}),
        generic_args
    ) | std::ranges::to<std::unordered_map<std::string, Type>>();
    return translateGenerics(type_mappings, typedef_->type);
}

std::expected<Type, std::string> Parser::parseNamedType(
    std::vector<TokenRange>::iterator& it,
    std::vector<TokenRange>::iterator end
) noexcept{
    const std::string alias = it->begin->value;
    decltype(M_defined_generics)::iterator item = std::find_if(M_defined_generics.begin(), M_defined_generics.end(),
        [&](const std::shared_ptr<GenericField>& e){
            return alias == e->name;
        }
    );
    if(item != M_defined_generics.end())
        return Type{.catagory = Type::TypeCatagory::GENERIC, .sub_data = *item};

    const auto old_it = it;
    ++it;
    std::vector<Type> generic_args;
    if(it < end && it->type == TokenRange::Type::NONE && it->begin->type == Token::Type::OPERATOR && it->begin->value == cfg::GENERIC_OPEN){
        const auto generic_args_res = parseGenericArgs(it, end);
        if(!generic_args_res.has_value())
            return std::unexpected{std::move(generic_args_res.error())};
        generic_args = generic_args_res.value();
    }else --it;
    const auto typedef_entries = M_typedefs
        | std::views::filter([&](const decltype(M_typedefs)::value_type& typedef_)
            {
                return typedef_.second->alias == alias && generic_args.size() == typedef_.second->generic_fields.size()
                && std::ranges::any_of(std::views::zip(typedef_.second->generic_fields, generic_args), [](const std::pair<std::shared_ptr<GenericField> &, Type &>& e){return e.first->requirement.overlapsWith(e.second);});
                // && std::ranges::all_of(std::views::zip(typedef_.second->generic_fields, generic_args), [&](const std::pair<std::shared_ptr<GenericField> &, Type &>& e){return e.first->requirement.getTypeCompatibility(e.second);});
            }
        )
        | std::ranges::to<std::vector>();
    std::vector<Type> accessible_typedef_entries;
    for (const auto& typedef_entry : typedef_entries | std::views::reverse){
        if(!Type{.catagory = Type::TypeCatagory::GENERIC_OR, .sub_data = accessible_typedef_entries}.getTypeCompatibility(typedef_entry.second->type))
            accessible_typedef_entries.push_back(typedef_entry.second->type);
    }
    std::reverse(accessible_typedef_entries.begin(), accessible_typedef_entries.end());
    std::println("{}", accessible_typedef_entries | format_type_list);
    if(!typedef_entries.empty())
        return parseGenericNamedType(generic_args, typedef_entries.back().second);
    return std::unexpected{
        std::format("Non-matching named type: \"{}\", with {} generic argument(s) on line: {}", alias, generic_args.size(), old_it->begin->line)
    };
}
