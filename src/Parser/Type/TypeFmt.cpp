#include "TypeFmt.hpp"
#include <format>

[[nodiscard]] static std::string formatPrefixType(const Type& type, std::string name) {
    const auto& sub_data = std::get<Type::PointerSubdata>(type.sub_data);
    return std::format("{}, with sub-type: [{}]",
        name,
        formatType(*sub_data)
    );
}

[[nodiscard]] std::string formatType(const Type& type){
    switch (type.catagory) {
        case Type::TypeCatagory::I64:
            return "Signed integer 64";
        case Type::TypeCatagory::I32:
            return "Signed integer 32";
        case Type::TypeCatagory::I16:
            return "Signed integer 16";
        case Type::TypeCatagory::I8:
            return "Signed integer 8";
        case Type::TypeCatagory::U64:
            return "Unsigned integer 64";
        case Type::TypeCatagory::U32:
            return "Unsigned integer 32";
        case Type::TypeCatagory::U16:
            return "Usigned integer 16";
        case Type::TypeCatagory::U8:
            return "Unsigned integer 8";
        case Type::TypeCatagory::BOOL:
            return "Boolean";
        case Type::TypeCatagory::BCMPLX32:
            return "(BFloat 16) complex 32";
        case Type::TypeCatagory::CMPLX32:
            return "Complex 32";
        case Type::TypeCatagory::CMPLX64:
            return "Complex 64";
        case Type::TypeCatagory::CMPLX128:
            return "Complex 128";
        case Type::TypeCatagory::BF16:
            return "BFloat 16";
        case Type::TypeCatagory::F16:
            return "Float 16";
        case Type::TypeCatagory::F32:
            return "Float 32";
        case Type::TypeCatagory::F64:
            return "Float 64";
        case Type::TypeCatagory::VOID:
            return "(Void/unit)";
        case Type::TypeCatagory::ARRAY:
        {
            const auto& sub_data = std::get<Type::ArraySubdata>(type.sub_data);
            return std::format("Array, with size: {} and subtype: [{}]", sub_data.size ? std::to_string(*sub_data.size) : "(sizeless)", formatType(*sub_data.type));
        }
        case Type::TypeCatagory::SET:
        {
            const auto& sub_data = std::get<Type::SetSubdata>(type.sub_data);
            return std::format("Set, with sub-type: [{}]", formatType(*sub_data));
        }
        case Type::TypeCatagory::UNDERLYING_TYPE_ACCESS:
        {
            const auto& sub_data = std::get<Type::UnderlyingTypeAccessSubdata>(type.sub_data);
            return std::format("Underlying type-access, with sub-type: [{}] and access index: {}",
                formatType(*sub_data.type),
                sub_data.access_index ? std::to_string(*sub_data.access_index) : "(0-derive)"
            );
        }
        case Type::TypeCatagory::MAP:
        {
            const auto& sub_data = std::get<Type::MapSubdata>(type.sub_data);
            return std::format("Map, with key type: [{}] and value type: [{}]", formatType(*sub_data.first), formatType(*sub_data.second));
        }
        case Type::TypeCatagory::TUPLE:
        {
            const auto& sub_data = std::get<Type::TupleSubdata>(type.sub_data);
            return std::format("Tuple, with sub-types: [{}]",
                sub_data
                | format_type_list
            );
        }
        case Type::TypeCatagory::GENERIC_OR:
        case Type::TypeCatagory::DYN_OR:
        {
            const auto& sub_data = std::get<Type::TupleSubdata>(type.sub_data);
            return std::format("{} type entry list, with sub-types: [{}]",
                type.catagory == Type::TypeCatagory::GENERIC_OR ? "Generic" : "Runtime",
                sub_data | format_type_list
            );
        }
        case Type::TypeCatagory::RESULT:
        {
            const auto& sub_data = std::get<Type::ResultSubdata>(type.sub_data);
            return std::format("Result, with value type: [{}] and error type: [{}]",
                formatType(*sub_data.first),
                formatType(*sub_data.second)
            );
        }
        case Type::TypeCatagory::POINTER:
            return formatPrefixType(type, "Pointer");
        case Type::TypeCatagory::OPTION:
            return formatPrefixType(type, "Option");
        case Type::TypeCatagory::FUNCTION_PTR:
        {
            const auto& sub_data = std::get<Type::FnPtrSubdata>(type.sub_data);
            return std::format("Function Pointer, with return type: [{}] and parameter types: [{}](generic varargs: {})",
                formatType(*sub_data.return_type),
                sub_data.param_types
                | format_type_list,
                sub_data.generic_variable_args
            );
        }
        case Type::TypeCatagory::GENERIC:
        {
            const auto& sub_data = std::get<Type::GenericSubdata>(type.sub_data);
            return std::format("Generic: {}, with type requirement: [{}]", sub_data->name, formatType(sub_data->requirement));
        }
        case Type::TypeCatagory::GENERIC_ANY:
            return "Generic Any-type";
        case Type::TypeCatagory::DYN_ANY:
            return "Runtime Any-type";
        default:
            return "UNFORMATTABLE";
    }
}
