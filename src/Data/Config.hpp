#pragma once
#include "Operator.hpp"
#include "Token.hpp"
#include "TokenRange.hpp"
#include "Type/Type.hpp"
#include <initializer_list>
#include <memory>
#include <string_view>
#include <ranges>
#include <tuple>
#include <unordered_map>

namespace cfg {
    const std::unordered_map<std::string_view, Type> TYPE_KEYWORDS{
        {"i8", Type{.catagory = Type::TypeCatagory::I8}},
        {"i16", Type{.catagory = Type::TypeCatagory::I16}},
        {"i32", Type{.catagory = Type::TypeCatagory::I32}},
        {"i64", Type{.catagory = Type::TypeCatagory::I64}},
        {"u8", Type{.catagory = Type::TypeCatagory::U8}},
        {"u16", Type{.catagory = Type::TypeCatagory::U16}},
        {"u32", Type{.catagory = Type::TypeCatagory::U32}},
        {"u64", Type{.catagory = Type::TypeCatagory::U64}},
        {"bf16", Type{.catagory = Type::TypeCatagory::BF16}},
        {"f16", Type{.catagory = Type::TypeCatagory::F16}},
        {"f32", Type{.catagory = Type::TypeCatagory::F32}},
        {"f64", Type{.catagory = Type::TypeCatagory::F64}},
        {"bool", Type{.catagory = Type::TypeCatagory::BOOL}},
        {"bcmplx32", Type{.catagory = Type::TypeCatagory::BCMPLX32}},
        {"cmplx32", Type{.catagory = Type::TypeCatagory::CMPLX32}},
        {"cmplx64", Type{.catagory = Type::TypeCatagory::CMPLX64}},
        {"cmplx128", Type{.catagory = Type::TypeCatagory::CMPLX128}},
        {"any", Type{.catagory = Type::TypeCatagory::GENERIC_ANY}},
    };
    const std::unordered_map<std::string_view, Type> NUMBER_EXTENSION_MAPPINGS{
        {"u", Type{.catagory = Type::TypeCatagory::U32}},
        {"u8", Type{.catagory = Type::TypeCatagory::U8}},
        {"u16", Type{.catagory = Type::TypeCatagory::U16}},
        {"u32", Type{.catagory = Type::TypeCatagory::U32}},
        {"u64", Type{.catagory = Type::TypeCatagory::U64}},
        {"i", Type{.catagory = Type::TypeCatagory::I32}},
        {"i8", Type{.catagory = Type::TypeCatagory::I8}},
        {"i16", Type{.catagory = Type::TypeCatagory::I16}},
        {"i32", Type{.catagory = Type::TypeCatagory::I32}},
        {"i64", Type{.catagory = Type::TypeCatagory::I64}},
        {"f", Type{.catagory = Type::TypeCatagory::F32}},
        {"f16", Type{.catagory = Type::TypeCatagory::F16}},
        {"f32", Type{.catagory = Type::TypeCatagory::F32}},
        {"f64", Type{.catagory = Type::TypeCatagory::F64}},
        {"bf16", Type{.catagory = Type::TypeCatagory::BF16}},
    };
    const std::unordered_map<char, char> STRING_CHAR_REGULAR_ESCAPE_MAPPINGS{
        {'v', '\v'},
        {'a', '\a'},
        {'e', '\e'},
        {'\\', '\\'},
        {'t', '\t'},
        {'r', '\r'},
        {'f', '\f'},
        {'b', '\b'},
        {'n', '\n'},
        {'\"', '\"'},
        {'\'', '\''},
    };
    constexpr std::initializer_list<std::string_view> ELSE_LIKE_KEYWORDS{
        "else",
        "dbg"
    };
    constexpr std::initializer_list<std::string_view> STATEMENT_KEYWORDS{
        "if",
        "while",
        "confirm",
        "switch",
        "for",
        "elif",
    };
    constexpr std::initializer_list<std::string_view> VARIABLE_FLAG_KEYWORDS{
        "stat",
        "mut",
        "man", //short for manual cleanup.
    };
    constexpr std::initializer_list<std::string_view> FUNCTION_FLAG_KEYWORDS{
        "extern",
        "symbol", //to guarante the function is externally linked with it's ORIGINAL NAME.
    };
    constexpr std::initializer_list<std::string_view> GLOBAL_FLAG_KEYWORDS{
        "export",
    };
    constexpr std::initializer_list<std::string_view> SINGLE_TOKEN_KEYWORDS{
        "break",
        "continue",
    };
    constexpr std::initializer_list<std::string_view> PREFIX_KEYWORDS{
        "ret",
    };
    constexpr std::string_view FUNCTION_KEYWORD = "fn";
    constexpr std::string_view IMPORT_KEYWORD = "import";
    constexpr std::string_view STRUCTURE_KEYWORD = "struct";
    constexpr std::string_view DEPREFIX_KEYWORD = "use";
    constexpr std::string_view TYPEDEF_KEYWORD = "type";
    constexpr std::string_view ENUM_KEYWORD = "enum";
    constexpr std::initializer_list<std::string_view> GLOBAL_FLAGGABLE_KEYWORDS{
        FUNCTION_KEYWORD,
        IMPORT_KEYWORD,
        STRUCTURE_KEYWORD,
        DEPREFIX_KEYWORD,
        TYPEDEF_KEYWORD,
        ENUM_KEYWORD,
    };
    constexpr std::string_view DYN_KEYWORD = "dyn";
    constexpr std::initializer_list<std::string_view> OTHER_KEYWORDS{
        DYN_KEYWORD
    };
    constexpr std::initializer_list<std::string_view> GLOBAL_NON_FLAGGABLE_KEYWORDS{
        "prefix",
    };
    constexpr std::initializer_list<std::string_view> VALUE_KEYWORDS{
        "true",
        "false",
        "null",
    };
    const std::unordered_map<std::string_view, Type> TYPE_AND_VALUE_KEYWORDS{
        {"void", Type{.catagory = Type::TypeCatagory::VOID}},
    };
    constexpr std::initializer_list<std::string_view> SWITCH_ITEM_KEYWORDS{
        "case",
        "default",
    };
    constexpr char CHAR_ENCLOSURE_SYMBOL = '\'';
    constexpr char STRING_ENCLOSURE_SYMBOL = '\"';
    constexpr char STRING_CHAR_ESCAPE_SYMBOL = '\\';
    constexpr Token STRING_CHAR_FMT_TOKEN_START = Token{.type = Token::Type::BRACKET_OPEN, .value = "{"};
    constexpr Token STRING_CHAR_FMT_TOKEN_END = Token{.type = Token::Type::BRACKET_CLOSE, .value = "}"};
    constexpr std::initializer_list<std::tuple<char, char, TokenRange::Type>> NORMAL_BRACKETS{ //normal in the sense that, it does not overlap with operators
        {'(', ')', TokenRange::Type::REGULAR},
        {'{', '}', TokenRange::Type::CURLY},
        {'[', ']', TokenRange::Type::SQUARE},
    };
    constexpr std::string_view GENERIC_VARARG_SYMB = "..";
    constexpr std::string_view ERROR_OPERATOR = "err";
    constexpr std::initializer_list<std::initializer_list<Operator>> OPERATORS {
        {
            Operator{.name = "!"},
            Operator{.name = ":="},
            Operator{.name = "?"},
        },
        {
            Operator{.name = ":"},
            Operator{.name = "in"}, Operator{.name = "async", .input_type = Operator::InputType::RIGHT},
            Operator{.name = ERROR_OPERATOR, .input_type = Operator::InputType::RIGHT},
        },
        {
            Operator{.name = "="}, Operator{.name = "+="},
            Operator{.name = "/="}, Operator{.name = "*="},
            Operator{.name = "++", .input_type = Operator::InputType::LEFT}, Operator{.name = "++", .input_type = Operator::InputType::RIGHT},
            Operator{.name = "--", .input_type = Operator::InputType::LEFT}, Operator{.name = "--", .input_type = Operator::InputType::LEFT},
        },
        {
            Operator{.name = "and"}, Operator{.name = "or"}, Operator{.name = "xor"},
        },
        {
            Operator{.name = "!="}, Operator{.name = "=="},
            Operator{.name = "not", .input_type = Operator::InputType::RIGHT},
            Operator{.name = ">="}, Operator{.name = "<="},
            Operator{.name = ">"}, Operator{.name = "<"},
        },
        {
            Operator{.name = ".."}, Operator{.name = "-<"}, Operator{.name = "-|"}
        },
        {
            Operator{.name = "+"}, Operator{.name = "-"},
        },
        {
            Operator{.name = "/"}, Operator{.name = "*"},
            Operator{.name = "mod"},
        },
        {
            Operator{.name = "lsh"}, Operator{.name = "rsh"},
        },
        {
            Operator{.name = "*", .input_type = Operator::InputType::RIGHT},
            Operator{.name = "-", .input_type = Operator::InputType::RIGHT},
            Operator{.name = "%", .input_type = Operator::InputType::LEFT}, //percent
        },
        {
            Operator{.name = "^"},
            Operator{.name = "!", .input_type = Operator::InputType::LEFT}, //factoral
            Operator{.name = "!", .input_type = Operator::InputType::RIGHT}, //derangement
            Operator{.name = "!!", .input_type = Operator::InputType::LEFT}, // double factorial
        },
        {
            Operator{.name = "?",  .input_type = Operator::InputType::LEFT},
        },
        {
            Operator{.name = "->"},
            Operator{.name = "::"},
            Operator{.name = "&", .input_type = Operator::InputType::RIGHT},
            Operator{.name = "."}, //access operator
            Operator{.name = "await", .input_type = Operator::InputType::RIGHT},
        },
    };

    constexpr std::string_view RETURN_OPERATOR = "->";
    constexpr std::string_view EOF_MSG = "(EOF)";
    constexpr char COMMENT_CHAR = '#';
    constexpr std::string_view MAP_ITEM_SPLIT = ":";
    constexpr std::string_view RESULT_OP_TYPENAME = "!";
    const std::unordered_map<std::string_view, Type> PREFIXED_OPERATOR_TYPE_MAPPINGS{
        {"*", Type{.catagory = Type::TypeCatagory::POINTER}},
        {"?", Type{.catagory = Type::TypeCatagory::OPTION}},
        {RESULT_OP_TYPENAME, Type{.catagory = Type::TypeCatagory::RESULT}},
    };
    const std::string_view GENERAL_ASSIGNMENT = "=";
    const std::string_view EXPORT_KEYWORD = "export";
    const std::string_view GENERIC_OPEN = "<";
    const std::string_view GENERIC_CLOSE = ">";
    const auto any_ptr = std::make_shared<Type>(Type{.catagory = Type::TypeCatagory::GENERIC_ANY});
    constexpr Type getDirectUnderlyingMatchType(const std::shared_ptr<Type>& sub_data_){
        return Type{
            .catagory = Type::TypeCatagory::GENERIC_OR,
            .sub_data = std::vector<Type>{
                Type{.catagory = Type::TypeCatagory::POINTER, .sub_data = sub_data_, .ignore_non_type_details = true},
                Type{.catagory = Type::TypeCatagory::OPTION, .sub_data = sub_data_, .ignore_non_type_details = true},
                Type{.catagory = Type::TypeCatagory::SET, .sub_data = sub_data_, .ignore_non_type_details = true},
                Type{.catagory = Type::TypeCatagory::MAP, .sub_data = Type::MapSubdata{cfg::any_ptr, sub_data_}, .ignore_non_type_details = true},
                Type{.catagory = Type::TypeCatagory::RESULT, .sub_data = Type::ResultSubdata{sub_data_, cfg::any_ptr}, .ignore_non_type_details = true},
                Type{.catagory = Type::TypeCatagory::ARRAY, .sub_data = Type::ArraySubdata{.type = sub_data_}, .ignore_non_type_details = true},
                Type{.catagory = Type::TypeCatagory::FUNCTION_PTR, .sub_data = Type::FnPtrSubdata{.return_type = sub_data_}, .ignore_non_type_details = true},
            },
        };
    }

    constexpr Type getErrorUnderlyingMatchType(const std::shared_ptr<Type>& sub_data_){
        return Type{.catagory = Type::TypeCatagory::RESULT, .sub_data = Type::MapSubdata{any_ptr, sub_data_}, .ignore_non_type_details = true};
    }

    constexpr Type getKeyUnderlyingMatchType(const std::shared_ptr<Type>& sub_data_){
        return Type{.catagory = Type::TypeCatagory::MAP, .sub_data = Type::MapSubdata{sub_data_, any_ptr}, .ignore_non_type_details = true};
    }

    constexpr Type getIndexedUnderlyingMatchType(const Type& sub_data_, std::size_t index){
        std::vector<Type> sub_types;
        for(const std::size_t _ : std::views::iota(0u, index)){
            sub_types.push_back(*any_ptr);
        }
        sub_types.push_back(sub_data_);
        return Type{
            .catagory = Type::TypeCatagory::GENERIC_OR,
            .sub_data = std::vector<Type>{
                Type{.catagory = Type::TypeCatagory::TUPLE, .sub_data = sub_types, .ignore_non_type_details = true},
                Type{.catagory = Type::TypeCatagory::FUNCTION_PTR, .sub_data = Type::FnPtrSubdata{.return_type = any_ptr, .param_types = sub_types}, .ignore_non_type_details = true},
            },
        };
    }

    constexpr std::string_view KEY_ACCESS_NAME_UNDERLYING = "key";

    const Type DIRECT_UNDERLYING_MATCH_TYPE = getDirectUnderlyingMatchType(any_ptr);
    const Type ERROR_UNDERLYING_MATCH_TYPE = getErrorUnderlyingMatchType(any_ptr);
    const Type KEY_UNDERLYING_MATCH_TYPE = getKeyUnderlyingMatchType(any_ptr);

    const std::uint8_t VERSION_MAJOR = 0;
    const std::uint8_t VERSION_MINOR = 1;
    const std::uint8_t VERSION_PATCH = 0;
}
