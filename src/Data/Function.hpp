#pragma once
#include "TokenRange.hpp"
#include "Type.hpp"
#include <set>
#include <vector>

struct Function {
    GenericFields generic_fields;
    std::vector<TokenRange> source;
    std::set<std::string> flags;
    std::string name;
    std::optional<Type> return_type;
};
