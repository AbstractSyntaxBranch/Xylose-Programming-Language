#pragma once
#include <cstddef>
#include <memory>
#include <optional>
#include <set>
#include <variant>
#include <vector>
struct OwnedStructSubData;

struct GenericField;

struct Type{
    enum class TypeCatagory{
        ARRAY,
        I8, I16,
        I32, I64,
        U8, U16,
        U32, U64,
        F16, F32,
        F64, BF16,
        BOOL,
        BCMPLX32,
        CMPLX32,
        CMPLX64,
        CMPLX128,
        STRUCTURE,
        TUPLE,
        POINTER,
        OPTION,
        RESULT,
        UNDERLYING_TYPE_ACCESS,
        ENUM,
        VOID,
        GENERIC,
        SET,
        MAP,
        GENERIC_OR,
        DYN_ANY,
        DYN_OR,
        FUNCTION_PTR,
        GENERIC_ANY,
    } catagory;

    struct ArraySubdata{
        //Shared resource, so it doesn't have to copy data if the Type is from a type definition.
        //Due to incomplete type rules, it also can't be a non-pointer.
        std::shared_ptr<Type> type;
        std::optional<std::size_t> size;
    };

    using StructSubdata = std::shared_ptr<OwnedStructSubData>;
    using TupleSubdata = std::vector<Type>;
    using GenericOrSubdata = std::vector<Type>;
    using PointerSubdata = std::shared_ptr<Type>;
    struct UnderlyingTypeAccessSubdata{
        std::shared_ptr<Type> type;
        enum : std::int64_t{
            RES_FAILED = -1,
            RES_SUCCESS = -2,
            FN_RET = -3,
            MAP_KEY = -4,
            MAP_VALUE = -5
        };
        std::optional<std::int64_t> access_index;
    };
    using OptionSubdata = std::shared_ptr<Type>;
    using ResultSubdata = std::pair<std::shared_ptr<Type>, std::shared_ptr<Type>>;

    struct OwnedEnumSubData{
        std::vector<std::string> items;
    };
    using EnumSubdata = std::size_t;
    using GenericSubdata = std::shared_ptr<GenericField>;
    using SetSubdata = std::shared_ptr<Type>;
    using MapSubdata = std::pair<std::shared_ptr<Type>, std::shared_ptr<Type>>;
    struct FnPtrSubdata{
        std::shared_ptr<Type> return_type;
        std::vector<Type> param_types;
        bool generic_variable_args = false;
    };
    std::variant<
        ArraySubdata,
        StructSubdata,
        TupleSubdata,
        std::shared_ptr<Type>,
        std::pair<std::shared_ptr<Type>, std::shared_ptr<Type>>,
        EnumSubdata,
        GenericSubdata,
        FnPtrSubdata,
        UnderlyingTypeAccessSubdata
    > sub_data;

    bool ignore_non_type_details = false;
    bool getTypeCompatibility(const Type& other) const;
};

struct GenericField{
    std::string name;
    Type requirement = {.catagory = Type::TypeCatagory::GENERIC_ANY};
};

using GenericFields = std::vector<std::shared_ptr<GenericField>>;

struct OwnedStructSubData{
    GenericFields generic_fields;
    //structure data
};

struct Typedef{
    std::string alias;
    Type type;
    GenericFields generic_fields;
    std::set<std::string> flags;
};
