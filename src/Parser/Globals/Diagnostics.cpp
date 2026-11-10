#include "../Parser.hpp"
#include "../../Data/Diagnostics.hpp"
#include "../Type/TypeFmt.hpp"
#include <memory>
#include <ranges>
#include "fmt/color.h"

void Parser::diagGenericFields(const GenericFields& generic_fields){
    fmt::print(diag::DIAGNOSTIC, "-- Generics: ");
    fmt::println("{}", generic_fields
        | std::views::transform(
            [](const std::shared_ptr<GenericField>& e){
                return fmt::format("{}: {}, ", e->name.empty() ? "<NAMELESS>" : e->name, formatType(e->requirement));
            }
        )
        | std::views::join
        | std::ranges::to<std::string>()
    );
}

void Parser::diagFn(){
    const auto& fn = M_functions.back().second;
    fmt::print(diag::DIAGNOSTIC_HEADER, "Function: ");
    fmt::println("{}", fn->name);
    fmt::print(diag::DIAGNOSTIC, "-- Exported: ");
    fmt::println("{}", M_functions.back().first);
    fmt::print(diag::DIAGNOSTIC, "-- Flags: ");
    fmt::println("{}",
        fn->flags
        | std::views::transform(
            [](const std::string& e){
                return e + ", ";
            })
        | std::views::join
        | std::ranges::to<std::string>()
    );
    diagGenericFields(fn->generic_fields);
    fmt::print(diag::DIAGNOSTIC, "-- Return type: ");
    const auto ret = fn->return_type;
    fmt::println("{}", ret.has_value() ? formatType(ret.value()) : diag::UNKNOWN_TYPE);
}

void Parser::diagTypedef(){
    const auto& typedef_ = M_typedefs.back().second;
    fmt::print(diag::DIAGNOSTIC_HEADER, "Typedef: ");
    fmt::println("{}", typedef_->alias);
    fmt::print(diag::DIAGNOSTIC, "-- Exported: ");
    fmt::println("{}", M_typedefs.back().first);
    fmt::print(diag::DIAGNOSTIC, "-- Flags: ");
    fmt::println("{}",
        typedef_->flags
        | std::views::transform(
            [](const std::string& e){
                return e + ", ";
            })
        | std::views::join
        | std::ranges::to<std::string>()
    );
    diagGenericFields(typedef_->generic_fields);
    fmt::print(diag::DIAGNOSTIC, "-- Type: ");
    fmt::println("{} <- Generic Dependant: {}", formatType(typedef_->type), typedef_->type.isGenericDependant());
}
