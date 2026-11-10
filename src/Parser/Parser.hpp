#pragma once
#include "../Data/ASTToken.hpp"
#include "../Data/Function.hpp"
#include "../Data/TokenRange.hpp"
#include <expected>
#include <memory>
#include <unordered_map>
#include <vector>

class Parser{
public:
    Parser(std::vector<Token>& tokens);
    static std::vector<TokenRange> splitIntoTokenRanges(const std::vector<Token>::iterator begin, const std::vector<Token>::iterator end);
    void groupIntoGlobalTokenRanges();
    void parseGlobals();
    void diagFn();
    void diagTypedef();
    void diagGenericFields(const GenericFields& fields);
    Type translateGenerics(const std::unordered_map<std::string, Type>& type_mappings, Type type) const;
    Function parseFunction(std::vector<TokenRange>::iterator& it);
    std::expected<Type, std::string> parseType(
        std::vector<TokenRange>::iterator& it,
        std::vector<TokenRange>::iterator end
    ) noexcept;
    std::expected<Type, std::string> parseUnderlyingValueType(
        std::vector<TokenRange>::iterator& it,
        std::vector<TokenRange>::iterator end
    ) noexcept;
    std::expected<std::vector<Type>, std::string> parseGenericArgs(
        std::vector<TokenRange>::iterator& it,
        std::vector<TokenRange>::iterator end
    ) noexcept;
    Type parseGenericNamedType(
        const std::vector<Type>& generic_args,
        std::shared_ptr<Typedef> typedef_
    ) const;
    std::expected<Type, std::string> parseNamedType(
        std::vector<TokenRange>::iterator& it,
        std::vector<TokenRange>::iterator end
    ) noexcept;
    std::expected<Type, std::string> parseDynamicType(
        std::vector<TokenRange>::iterator& it,
        std::vector<TokenRange>::iterator end
    ) noexcept;
    std::expected<Type, std::string> parseTypeBase(
            std::vector<TokenRange>::iterator& it,
            std::vector<TokenRange>::iterator end
    ) noexcept;
    GenericFields parseGenericFields(
        std::vector<TokenRange>::iterator& it
    );
    Typedef parseTypedef(std::vector<TokenRange>::iterator& it);
    std::expected<Type, std::string> parseArrayType(
        const TokenRange& rng
    ) noexcept;
    std::expected<Type, std::string> parseSetOrMapType(
        const TokenRange& rng
    ) noexcept;
    std::expected<Type, std::string> parseTupleType(
        const TokenRange& rng
    ) noexcept;
    std::expected<Type, std::string> parsePrefixOpType(
        std::vector<TokenRange>::iterator& it,
        std::vector<TokenRange>::iterator end
    ) noexcept;
    std::expected<Type, std::string> parseFnPtrType(
        std::vector<TokenRange>::iterator& it,
        std::vector<TokenRange>::iterator end
    ) noexcept;
    Type translateUnderlyingType(const Type::UnderlyingTypeAccessSubdata& sub_data) const;
private:
    std::vector<Token>& M_tokens; //->
    std::vector<TokenRange> M_token_ranges; //->
    std::vector<ASTToken> M_AST_tokens;
    std::vector<std::pair<bool, std::shared_ptr<Function>>> M_functions; //bool is if it will get exported
    std::vector<std::pair<bool, std::shared_ptr<Typedef>>> M_typedefs;
    GenericFields M_defined_generics;
};
