#include "../../Data/Type.hpp"
#include <string>
#include <ranges>

std::string formatType(const Type& type);

constexpr auto format_type_list =
    std::views::transform([](const auto& e){
        return formatType(e) + ", ";
    })
    | std::views::join
    | std::ranges::to<std::string>();
