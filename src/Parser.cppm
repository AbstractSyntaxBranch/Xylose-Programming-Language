export module parser;
import lexer;
import std;
import files;

export class Parser : Lexer{
    using Tokens = std::vector<Token>;
    struct Type;
    struct TokenProcessingRange : Tokens{
        bool is_bracketed = false;
        enum Type : char{
            ARRAY = '[',
            MAP = '{',
            NORMAL = '(',
        } collection_type = NORMAL;
        Tokens::iterator original_begin, original_end;
        std::vector<Parser::Type> generic_fill_ins;
        std::uint32_t line;
    };
    using GenericFields = std::unordered_set<std::string>;
    struct AccessHandler{
        bool will_export; //if it will be exported the next time
        std::vector<std::string> allowed_file_modules; //allowed files to access the item
        bool disabled = false; //for filtering out duplicates
    };
    struct Variable;
    using VariableIndex = std::multimap<std::string, Variable>;
    struct Struct : AccessHandler{
        GenericFields generic_fields;
        VariableIndex variables;
    };
    struct Enum : AccessHandler{
        std::shared_ptr<std::vector<std::string>> values;
        inline bool operator==(Enum& other){
            return values == other.values;
        }
    };
    using Enums = std::multimap<std::string, Enum>;
    using EnumItem = std::pair<std::string, Enum>*;
    struct Type{
        using SubData = std::variant<std::vector<Type>, Struct, std::string, std::pair<std::shared_ptr<Type>, std::uint64_t>, EnumItem>;
        enum class Typename{
            ARRAY,
            I8, I16,
            I32, I64,
            U8, U16,
            U32, U64,
            STRUCTURE,
            TUPLE,
            POINTER,
            OPTION,
            RESULT,
            ENUM,
            VOID,
            GENERIC
        } type_family;
        SubData sub_data; //contains an Struct if it's an structure, else it's just a vector to the underlying type.
    };
    using GenericFieldTranslation = std::unordered_map<std::string, Type>; // used to define which generic maps to which type
    struct Typedef : Type, AccessHandler{
        std::shared_ptr<GenericFields> generic_fields;
        inline bool operator==(Typedef& other){
            return generic_fields == other.generic_fields;
        }
    };
    struct TokenASTItem;
    struct Variable{
        std::optional<Type> type; //is nothing if type isn't deduced yet
        bool is_constant;
        bool is_export;
        bool is_extern;
        std::size_t decl_line;
        std::optional<std::shared_ptr<TokenASTItem>> default_value;
    };
    struct TokenASTItem{
        TokenProcessingRange::Type collection_type;
        std::optional<Token> token; //isnt present when dealing with a tuple
        std::vector<TokenASTItem> children; // first one is reserved for the condition if its an if/while/for.
        std::optional<Operator::Type> operator_type;
        std::vector<Parser::Type> generic_fill_ins;
        enum class CallType{NONE, INDEXED, CALL} call_type = CallType::NONE;
        std::optional<std::pair<std::string, Variable>> variable;
    };
    struct Function : AccessHandler{
        bool is_extern = false;
        Type ret_type;
        GenericFields generic_fields;
        VariableIndex inputs;
        std::optional<std::shared_ptr<Tokens>> implementation;
        inline bool operator==(Function& other){
            return implementation == other.implementation; //same ptr
        }
    };
    using Functions = std::multimap<std::string, Function>;
    struct ModuleData{
        std::multimap<std::string, Typedef> typedefs; //multimap to account for the possibility of module overloads.
        std::multimap<std::string, Struct> structs;
        Enums enums;
        Functions functions;
    };
    public:
    Parser(std::string& filepath, const std::set<std::string>& keywords, const std::vector<std::vector<Operator>>& operators_ordered, const std::string& brackets)
    : Lexer(readFileByPath(filepath), keywords, operators_ordered, brackets), filepath(filepath), env_path(filepath) {}

    inline static Parser getDefault(std::string& filepath){
        return Parser
        {
            filepath, {
                "ret", "fn", "if",
                "elif", "else", "do",
                "while", "import", "export",
                "struct", "enum", "for",
                "in", "type", "extern",
                "const", "break", "continue",
                "case", "switch",
                "true", "false",
                "i8", "u8", "i16", "u16", "i32", "u32", "i64", "u64", "bf16"
                "void"
            },
            {
                {
                    Operator{.name = "!"},
                    Operator{.name = ":="}, //ignored while parsing, for declarations
                    Operator{.name = "?"}, //for optionals
                },
                {
                    Operator{.name = ":"}, // : for map item access, so not ignored
                    Operator{.name = "in"}, Operator{.name = "by"}, Operator{.name = "thread", .type = Operator::Type::RIGHT},
                    Operator{.name = "where"}, // filter
                },
                {
                    Operator{.name = "="}, Operator{.name = "+="},
                    Operator{.name = "/="}, Operator{.name = "*="},
                    Operator{.name = "++", .type = Operator::Type::LEFT}, Operator{.name = "++", .type = Operator::Type::RIGHT},
                    Operator{.name = "--", .type = Operator::Type::LEFT}, Operator{.name = "--", .type = Operator::Type::LEFT},
                },
                {
                    Operator{.name = "and"}, Operator{.name = "or"}, Operator{.name = "xor"},
                },
                {
                    Operator{.name = "!="}, Operator{.name = "=="},
                    Operator{.name = "not", .type = Operator::Type::RIGHT},
                    Operator{.name = ">="}, Operator{.name = "<="},
                    Operator{.name = ">"}, Operator{.name = "<"},
                },
                {
                    Operator{.name = ".."}, Operator{.name = "-<"}, Operator{.name = "-."}
                },
                {
                    Operator{.name = "<<"}, Operator{.name = ">>"},
                },
                {
                    Operator{.name = "+"}, Operator{.name = "-"},
                },
                {
                    Operator{.name = "/"}, Operator{.name = "*"},
                    Operator{.name = "mod"},
                },
                {
                    Operator{.name = "<<"}, Operator{.name = ">>"},
                },
                {
                    Operator{.name = "*", .type = Operator::Type::RIGHT},
                    Operator{.name = "-", .type = Operator::Type::RIGHT},
                    Operator{.name = "%", .type = Operator::Type::LEFT}, //percent
                },
                {
                    Operator{.name = "^"},
                    Operator{.name = "!", .type = Operator::Type::LEFT}, Operator{.name = "!!", .type = Operator::Type::LEFT}, //factorial and double factorial
                },
                {
                    Operator{.name = "?",  .type = Operator::Type::LEFT},
                },
                {
                    Operator{.name = "->"},
                    Operator{.name = "::"},
                    Operator{.name = "&", .type = Operator::Type::RIGHT},
                    Operator{.name = "."}, //access operator
                    Operator{.name = "collect",  .type = Operator::Type::RIGHT},
                },
            },
        "{}()[]"
        };
    }
    void parseToGlobals(){
        generateLexerData();
        splitLexerGlobals();
    }
    void generateLexerData(){
        std::optional<Token> token;
        while(token = getNextToken())
            tokens.push_back(*token);
        current_token = tokens.begin();
    }
    void splitLexerGlobals(){
        static std::string valid_global_msg = "valid global keyword";
        if(current_token == tokens.end())
            return;
        static bool is_next_extern = false, is_next_export = false;
        static const auto no_extern = [&](){
            if(is_next_extern)
                throw std::runtime_error{
                    std::format("On line: {}, there's been attempted on loading something externally which can't be loaded externally.", current_token->line)
                };
        };
        static const auto reusage_of_flag = [&](){
            throw std::runtime_error{
                std::format("On line: {}, multiple of the same linkage flags found.", current_token->line)
            };
        };
        if(current_token->type != Token::Type::KEYWORD)
            expectSmthGotSmthIncToken(valid_global_msg, true);
        if(current_token->value == "extern"){
            if(is_next_extern)
                reusage_of_flag();
            is_next_extern = true;
        }
        else if(current_token->value == "export"){
            if(is_next_export)
                reusage_of_flag();
            is_next_export = true;
        }
        else if(current_token->value == "fn"){
            parseGlobalFnSignature(is_next_extern, is_next_export);
        }
        else if(current_token->value == "struct"){
            no_extern();
            const std::string body_expected_msg = "struct body";
            expectSmthGotSmthIncToken("struct name", false, Token::Type::IDENTIFIER);
            const std::string name = current_token->value;
            expectSmthGotSmthIncToken(body_expected_msg, false, Token::Type::BRACKET, "{");
            Struct structure;
            parseOptionalGeneric(structure.generic_fields);
            if(structure.generic_fields.size())
                expectSmthGotSmthIncToken(body_expected_msg, false, Token::Type::BRACKET, "{");
            while(true){
                expectSmthGotSmthIncToken("the end of the struct body", false);
                if(current_token->value == "}" && current_token->type == Token::Type::BRACKET)
                    break;
                structure.variables.emplace(parseVariableDeclaration(structure.generic_fields, true));
            }
            structs.emplace(name, structure);

        }
        else if(current_token->value == "type"){
            no_extern();
            parseTypeDecl(is_next_export);
            is_next_export = false;
        }
        else if(current_token->value == "enum"){
            no_extern();
            expectSmthGotSmthIncToken("enum name", false, Token::Type::IDENTIFIER);
            const std::string name = current_token->value;
            expectSmthGotSmthIncToken("enum body", false, Token::Type::BRACKET, "{");
            const auto enum_properties = parseEnumItems(is_next_export);
            enums.emplace(name, enum_properties);
            is_next_export = false;
        }
        else if(current_token->value == "import"){
            no_extern();
            handleImport(is_next_export);
        }
        else expectSmthGotSmthIncToken(valid_global_msg, true);

        ++current_token;
        splitLexerGlobals();
    }
    std::vector<TokenProcessingRange> getTokenViews(const Tokens& tokens, const GenericFields& generics){
        current_implementation_source = tokens;
        std::uint32_t indent = 0, iterations_since = 0;
        TokenProcessingRange current_view;
        std::vector<TokenProcessingRange> out;
        std::uint64_t first_opened_bracket_line;
        for(const auto& [index, token] : std::views::enumerate(tokens)){
            ++iterations_since;
            bool decreased = false;
            bool changed_indent = false;
            if(token.type == Token::Type::BRACKET && !token.value.empty() && (token.value[0] == TokenProcessingRange::NORMAL || token.value[0] == TokenProcessingRange::ARRAY || token.value[0] == TokenProcessingRange::MAP)){
                if(!indent){
                    first_opened_bracket_line = token.line;
                    switch(token.value[0]){
                        case '(':
                            current_view.collection_type = TokenProcessingRange::NORMAL; break;
                        case '[':
                            current_view.collection_type = TokenProcessingRange::ARRAY; break;
                        case '{':
                            current_view.collection_type = TokenProcessingRange::MAP; break;
                    }
                }
                ++indent;
                changed_indent = true;
            }
            else if(token.type == Token::Type::BRACKET && (token.value == ")" || token.value == "]" || token.value == "}")){
                --indent;
                if(!indent &&
                    (
                        (current_view.collection_type == TokenProcessingRange::NORMAL) != (token.value == ")") ||
                        (current_view.collection_type == TokenProcessingRange::ARRAY) != (token.value == "]") ||
                        (current_view.collection_type == TokenProcessingRange::MAP) != (token.value == "}")
                    )
                )
                    throw std::runtime_error{std::format("On line: {}, mismatched open/close brackets.", first_opened_bracket_line)};
                decreased = changed_indent = true;
            }
            if(indent > (1-decreased) || !changed_indent){
                if(current_view.empty())
                    current_view.original_begin = current_implementation_source.begin() + index;
                current_view.push_back(token);
            }
            if(!indent){
                current_view.original_end = current_implementation_source.begin() + index + 1;
                current_view.is_bracketed = iterations_since-1;
                out.push_back(current_view);
                current_view.clear();
                iterations_since = 0;
            }
            if(current_view.empty())
                current_view.line = token.line;
        }
        if(indent)
            throw std::runtime_error{std::format("Unclosed brackets on line(started at): {}. Missing ')' to match '('.", first_opened_bracket_line)};
        out = groupGenericRangeInTokenViews(out, tokens, generics);

        return out;
    }
    std::vector<TokenProcessingRange> groupGenericRangeInTokenViews(const std::vector<TokenProcessingRange>& token_views, const Tokens& tokens, const GenericFields& generics){
        const auto& is_opening = [](const auto& token_view){return !token_view.is_bracketed && token_view.front().type == Token::Type::OPERATOR && token_view.front().value == "<";};
        const auto& is_closing = [](const auto& token_view){return !token_view.is_bracketed && token_view.front().type == Token::Type::OPERATOR && token_view.front().value == ">";};
        const auto& is_double_closing = [](const auto& token_view){return !token_view.is_bracketed && token_view.front().type == Token::Type::OPERATOR && token_view.front().value == ">>";};
        std::vector<TokenProcessingRange> out;
        std::size_t minimum_to_continue = 0;
        for(const auto& [index, token_view] : std::views::enumerate(token_views)){
            if(minimum_to_continue > index)
                continue;
            if(is_opening(token_view)){
                std::size_t indent = 0, start = index;
                for(std::size_t i = index; i < token_views.size(); ++i){
                    if(is_opening(token_views[i]))
                        ++indent;
                    else if(is_closing(token_views[i]))
                        --indent;
                    else if(is_double_closing(token_views[i])){
                        if(indent == 1)
                            break; // after the generic, there isnt usually another >
                        indent -= 2;
                    }
                    if(!indent){
                        //is it even a function call?
                        if(token_views.size() != i + 1 && token_views[i+1].is_bracketed && token_views[i+1].collection_type == TokenProcessingRange::NORMAL && start && !token_views[start-1].is_bracketed && token_views[start-1].front().type == Token::Type::IDENTIFIER){
                            std::vector<Type> generic_fields;
                            const auto& tokens_copy = this->tokens;
                            this->tokens = tokens;
                            current_token = this->tokens.begin() + start;
                            try{
                                generic_fields = parseGenericArguments(generics);
                            }catch(...){
                                break;
                            }
                            this->tokens = tokens_copy;
                            minimum_to_continue = i + 1;
                            out.back().generic_fill_ins = generic_fields;
                            goto skip;
                        }
                        break;
                    }
                }
            }
            out.push_back(token_view);
            skip:
        }
        printTokenProcessingRange(out);
        return out;
    }
    bool mustBeOperator(bool current_non_operator, bool current_multi_size, const TokenProcessingRange& token_view, Operator::Type type){
        return !current_non_operator && !current_multi_size && std::ranges::none_of(
            operators_ordered,
            [&](const auto& operator_list){
                return std::ranges::any_of(
                    operator_list,
                    [&](const auto& _operator){
                        return _operator.name == token_view.front().value
                            && _operator.type != type;
                    }
                );
            }
        );
    }
    std::vector<std::vector<TokenProcessingRange>> getLines(const std::vector<TokenProcessingRange>& token_views){
        std::vector<std::vector<TokenProcessingRange>> out;
        //lol, so much variables; this function does so much, yet so little
        bool prev_left_operator = false;
        bool prev_non_operator = false;
        bool prev_multi_size = false;
        bool current_line_preserve_brackets = false;
        bool current_line_preserve_next = false;
        bool prev_do_kw = false;
        bool prev2_do_kw = false;
        bool prev3_do_kw = false;
        bool prev_ident = false;
        bool prev_is_single_kw = false;
        bool is_in_type = false;
        bool prev_should_close_for_generic = false;
        std::uint32_t generic_indent = 0;
        std::vector<TokenProcessingRange> current_line;
        for(const TokenProcessingRange& token_view : token_views){
            bool current_non_operator = !token_view.empty() && token_view.front().type != Token::Type::OPERATOR;
            bool current_multi_size = token_view.is_bracketed;
            is_in_type = (is_in_type && (current_multi_size ? true : token_view.front().value != "=")) || (!current_multi_size && token_view.front().value == ":");
            bool current_right_operator = mustBeOperator(current_non_operator, current_multi_size, token_view, Operator::RIGHT);
            bool is_semicolon = !current_multi_size && token_view.front().type == Token::Type::SEMICOLON;
            bool is_single_kw = !current_multi_size && (token_view.front().value == "break" || token_view.front().value == "continue") && token_view.front().type == Token::Type::KEYWORD;
            bool generic_indent_changed = false;
            if(is_in_type && !current_multi_size && token_view.front().type == Token::Type::OPERATOR){
                generic_indent_changed = true;
                if(token_view.front().value == "<")
                    ++generic_indent;
                else if(token_view.front().value == ">")
                    --generic_indent;
                else if(token_view.front().value == ">>"){
                    if(generic_indent == 1)
                        throw std::runtime_error{std::format("On line: {}, double generic indentation closing, where only one was required.", token_view.line)};
                    generic_indent -= 2;
                }
                else generic_indent_changed = false;
            }
            if(
                (
                    (
                        (prev_left_operator && (current_right_operator || current_non_operator)) ||
                        (prev_non_operator && ((current_non_operator && !current_multi_size) || current_right_operator)) ||
                        current_multi_size ? (!prev_ident && prev_non_operator) : false
                    ) && !prev_do_kw && !prev2_do_kw && !prev3_do_kw
                     && !current_line_preserve_next
                ) || is_semicolon || is_single_kw || prev_is_single_kw || (prev_should_close_for_generic && (token_view.is_bracketed || token_view.front().value != "=" || token_view.front().type != Token::Type::OPERATOR))
            ){
                if(current_line_preserve_brackets && !current_line_preserve_next)
                    current_line_preserve_brackets = false;
                else{
                    if(!current_line.empty())
                        out.push_back(current_line);
                    current_line.clear();
                    is_in_type = false;
                }
            }
            if(!is_semicolon)
                current_line.push_back(token_view);

            prev_left_operator = mustBeOperator(current_non_operator, current_multi_size, token_view, Operator::LEFT);
            prev_multi_size = current_multi_size;
            prev_non_operator = current_non_operator;
            current_line_preserve_next = current_line.size() == 1 && !current_multi_size && token_view.front().type == Token::Type::KEYWORD && (token_view.front().value == "do" || token_view.front().value == "while" || token_view.front().value == "if" || token_view.front().value == "ret" || token_view.front().value == "for" || token_view.front().value == "elif" || token_view.front().value == "else"); // true and false may not force the line to keep going on
            current_line_preserve_brackets |= current_line_preserve_next;
            prev3_do_kw = prev2_do_kw;
            prev2_do_kw = prev_do_kw;
            prev_do_kw = current_line.size() == 1 && !current_multi_size && token_view.front().type == Token::Type::KEYWORD && token_view.front().value == "do";
            prev_ident = !current_multi_size && token_view.front().type == Token::Type::IDENTIFIER;
            prev_is_single_kw = is_single_kw;
            prev_should_close_for_generic = generic_indent_changed && generic_indent == 0;
        }
        out.push_back(current_line);
        return out;
    }
    void printTokenProcessingRange(const std::vector<std::vector<TokenProcessingRange>>& token_views_collection){
        for(const auto& token_views : token_views_collection){
            std::println("---LINE:---");
            printTokenProcessingRange(token_views);
        }
    }
    void printTokenProcessingRange(const std::vector<TokenProcessingRange>& token_views){
        for(const auto& token_view : token_views)
            printTokenProcessingRange(token_view);
    }
    void printTokenProcessingRange(const TokenProcessingRange& token_view){
        std::println("[bracketed: {}]:", token_view.is_bracketed);
        for(const auto& token : token_view){
            std::println("\tName: {}, type: {}", token.value, lookupTokenTypeName(token.type));
        }
    }
    std::optional<std::size_t> getIndexTopOfAST(const std::vector<TokenProcessingRange>& token_views){
        bool prev_left_operator = false;
        bool prev_non_operator;
        for(const auto& operator_list : operators_ordered | std::views::drop(1)) //dropping the first one, that one's reserved for special parsing
            for(const auto& [index, token_view] : std::views::reverse(std::views::enumerate(token_views))){
                if(!token_view.is_bracketed
                    && token_view.front().type == Token::Type::OPERATOR
                    && std::ranges::any_of(operator_list, [&](const auto& _operator){
                            return !token_view.is_bracketed && _operator.name == token_view.front().value && _operator.type == (index == token_views.size()-1 ? Operator::Type::LEFT : (index == 0 ? Operator::Type::RIGHT : Operator::Type::BOTH));
                    })
                    //function
                    && (index ? !mustBeOperator((prev_non_operator = token_views[index-1].is_bracketed || token_views[index-1].front().type != Token::Type::OPERATOR), token_view.is_bracketed, token_views[index-1], Operator::BOTH) : true)
                )
                    return index;
            }
        return {};
    }
    TokenASTItem getLineAST(const std::vector<TokenProcessingRange>& token_views, std::uint64_t line, const GenericFields& generic_fields){
        const std::optional<std::uint32_t> index = getIndexTopOfAST(token_views);
        TokenASTItem token;
        if(token_views.size() == 1){
            if(!token_views.front().is_bracketed){
                if(token_views.front().front().type == Token::Type::SEMICOLON)
                    throw std::runtime_error{std::format("Unexpected semicolon on line: {}, expected value.", line)};
                if(token_views.front().front().type == Token::Type::COMMA)
                    throw std::runtime_error{std::format("Unexpected comma on line: {}, expected value.", line)};
                if(token_views.front().front().type == Token::Type::OPERATOR)
                    throw std::runtime_error{std::format("Non-matching operator {} on line: {}, expected value.", token_views.front().front().value, line)};
                return TokenASTItem{.token = token_views.front().front(), .children = {}, .operator_type = {}};
            }
            //tuples or brackets
            auto children = std::vector<TokenASTItem>{};
            if(!token_views.front().empty())
                children = parseExprList(token_views.front(), line, generic_fields);
            if(children.size() == 1 && token_views.front().collection_type == TokenProcessingRange::NORMAL)
                return children.front();
            if(children.empty() && token_views.front().collection_type == TokenProcessingRange::NORMAL)
                throw std::runtime_error{std::format("No empty tuple value allowed on line {}, please consider using the built-in void keyword.", line)};
            token.children = children;
            token.collection_type = token_views.front().collection_type;
            return token;
        }else if(token_views.size() == 2 && !token_views.front().is_bracketed && token_views.front().front().type == Token::Type::IDENTIFIER && token_views[1].is_bracketed && token_views[1].collection_type != TokenProcessingRange::MAP && !index){
            token.children = token_views[1].empty() ? std::vector<TokenASTItem>{} : parseExprList(token_views[1], line, generic_fields);
            token.token = token_views[0].front();
            token.call_type = token_views[1].collection_type == TokenProcessingRange::NORMAL ? TokenASTItem::CallType::CALL : TokenASTItem::CallType::INDEXED;
            token.generic_fill_ins = token_views.front().generic_fill_ins;
            return token;
        }else if (
            isOneOfKeywords(token_views.front(), {"while", "for", "if", "elif"}) &&
            token_views.back().is_bracketed && token_views.back().collection_type == TokenProcessingRange::MAP &&
            token_views.size() > 2
        ){
            const auto condition = getLineAST(std::vector(token_views.begin() + 1, token_views.end() - 1), line, generic_fields);
            token.children.push_back(condition);
            auto implementation_source_cpy = current_implementation_source;
            const auto lines = token_views.back().empty() ? std::vector<TokenASTItem>{} : parseLinesFromTokenList(token_views.back(), generic_fields);
            current_implementation_source = implementation_source_cpy;
            token.children.append_range(lines);
            token.token = token_views.front().front();
            return token;
        }
        else if(
            isOneOfKeywords(token_views.front(), {"ret"})
        ){
            token.token = token_views.front().front();
            if(token_views.size() > 1)
                token.children.push_back(getLineAST(std::vector(token_views.begin() + 1, token_views.end()), line, generic_fields));
            return token;
        }
        else if(
            token_views.size() > 3 &&
            isOneOfKeywords(token_views.front(), {"do"}) &&
            token_views.at(1).is_bracketed && token_views.at(1).collection_type == TokenProcessingRange::MAP &&
            isOneOfKeywords(token_views.at(2), {"while"})
        ){
            const auto condition = getLineAST(std::vector(token_views.begin() + 3, token_views.end()), line, generic_fields);
            token.children.push_back(condition);
            const auto lines = token_views.at(1).empty() ? std::vector<TokenASTItem>{} : parseLinesFromTokenList(token_views.at(1), generic_fields);
            token.children.append_range(lines);
            token.token = token_views.front().front();
            return token;
        }
        else if(
            token_views.size() == 2 &&
            isOneOfKeywords(token_views.front(), {"else"}) &&
            token_views.at(1).is_bracketed && token_views.at(1).collection_type == TokenProcessingRange::MAP
        ){
            const auto lines = token_views.at(1).empty() ? std::vector<TokenASTItem>{} : parseLinesFromTokenList(token_views.at(1), generic_fields);
            token.children.append_range(lines);
            token.token = token_views.front().front();
            return token;
        }
        else if(!index)
            throw std::runtime_error{std::format("Couldn't parse the following expression (on line: {}): \n{}", line, getLineAsText(token_views.front().original_begin, token_views.back().original_end))};

        token.token = token_views[*index].front();
        token.operator_type = *index ? (*index == token_views.size()-1 ? Operator::LEFT : Operator::BOTH) : Operator::RIGHT;
        if(*token.operator_type != Operator::RIGHT){
            TokenASTItem token_child = getLineAST(std::vector<TokenProcessingRange>{token_views.begin(), token_views.begin() + *index}, line, generic_fields);
            token.children.push_back(token_child);
        }
        if(*token.operator_type != Operator::LEFT){
            TokenASTItem token_child = getLineAST(std::vector<TokenProcessingRange>{token_views.begin() + *index + 1, token_views.end()}, line, generic_fields);
            token.children.push_back(token_child);
        }
        return token;
    }
    bool isOneOfKeywords(const TokenProcessingRange& token_view, const std::vector<std::string>& keywords){
        return !token_view.is_bracketed &&
                token_view.front().type == Token::Type::KEYWORD &&
                std::ranges::any_of(keywords, [&](const auto& keyword){return keyword == token_view.front().value;});
    }
    std::vector<TokenASTItem> parseExprList(const TokenProcessingRange& token_view_parent, std::uint64_t line, const GenericFields& generic_fields){
        std::vector<TokenASTItem> out;
        const auto& token_views = getTokenViews(token_view_parent, generic_fields);
        std::vector<TokenProcessingRange> current_item;
        for(const auto& token_view : token_views){
            if(!token_view.is_bracketed && token_view.front().type == Token::Type::COMMA){
                out.push_back(getLineAST(current_item, line, generic_fields));
                current_item.clear();
            }
            else
                current_item.push_back(token_view);
        }
        out.push_back(getLineAST(current_item, line, generic_fields));
        return out;
    }
    std::vector<TokenASTItem> parseLinesFromTokenList(const Tokens& tokens, const GenericFields& generic_fields){
        if(tokens.empty())
            return {};
        std::vector<TokenASTItem> out;
        const auto& lines = getLines(getTokenViews(tokens, generic_fields));
        for(const auto& token_views : lines){
            if(std::ranges::none_of(token_views, [&](const TokenProcessingRange& token_view){return !token_view.is_bracketed && token_view.front().type == Token::Type::OPERATOR && (token_view.front().value == ":" || token_view.front().value == ":=");}))
                out.push_back(getLineAST(token_views, token_views.front().line, generic_fields));
            else{
                Tokens tokens_cpy = std::move(this->tokens);
                this->tokens = Tokens(token_views.front().original_begin, token_views.back().original_end);
                current_token = this->tokens.begin();
                out.push_back(TokenASTItem{.variable = parseVariableDeclaration(generic_fields, true)});
                this->tokens = std::move(tokens_cpy);
            }
        }
        return out;
    }
    std::string getLineAsText(std::vector<Token>::iterator begin, std::vector<Token>::iterator end){
        std::string out;
        for(; begin != end; ++begin)
            out.append(" " + begin->value + " ");
        return out;
    }
    std::string getGenericArgumentsAsText(const std::vector<Type>& types){
        std::string out;
        for(const auto& type : types){
            out.append(lookupVariableTypeName(type.type_family) + " ");
        }
        return out;
    }
    void printLineAST(const TokenASTItem& token){
        static std::string indent = "";
        if(token.token)
            std::print("⹃{}>Token: \"{}\"", indent, token.token->value);
        else if(token.variable){
            std::print("⹃{}> Variable declaration of: {} with (simplified) type: {}", indent, token.variable->first, token.variable->second.type ? lookupVariableTypeName(token.variable->second.type->type_family) : "UNKNOWN");
            if(token.variable->second.default_value){
                std::println(" and default value:");
                indent += "-";
                printLineAST(**token.variable->second.default_value);
                indent.pop_back();
            }else
                std::println();
        }
        else
            std::print("⹃{}>{}", indent, token.collection_type == TokenProcessingRange::ARRAY ? "Array" : token.collection_type == TokenProcessingRange::NORMAL ? "Tuple" : "MAP");
        if(!token.variable)
            std::println("{}", token.call_type == TokenASTItem::CallType::NONE ? "" : (token.call_type == TokenASTItem::CallType::INDEXED ? " <- array access" : " <- call"));
        if(token.call_type == TokenASTItem::CallType::CALL)
            std::println("Generic arguments of call: {}", getGenericArgumentsAsText(token.generic_fill_ins));
        indent += "-";
        for(const TokenASTItem& child : token.children){
            printLineAST(child);
        }
        indent.pop_back();
    }
    //for if smth is included twice
    void foldDuplicateEntries(){
        foldDuplicateEntries(functions);
        foldDuplicateEntries(enums);
        foldDuplicateEntries(typedefs);
    }
    template <typename T>
    void foldDuplicateEntries(std::multimap<std::string, T>& collection){
        std::multimap<std::string, T> out_collection;
        for(auto& [name, item] : collection){
            if(item.disabled)
                continue;
            for(auto instance = ++collection.lower_bound(name); instance != collection.upper_bound(name); ++instance){
                if(item == instance->second){
                    instance->second.disabled = true;
                    item.allowed_file_modules.append_range(instance->second.allowed_file_modules);
                }
            }
            out_collection.emplace(name, item);
        }
        collection = out_collection;
    }
    void parseGlobalFnSignature(bool& is_next_extern, bool& is_next_export){
        std::pair<std::string,Function> fn = parseFunctionHead();
        fn.second.is_extern = is_next_extern;
        fn.second.allowed_file_modules = {filepath};
        fn.second.will_export = is_next_export;
        is_next_extern = false;
        is_next_export = false;
        if(!fn.second.is_extern){
            fn.second.implementation = getFunctionBody();
        }
        functions.emplace(fn);
    }
    void handleImport(bool& is_export){
        expectSmthGotSmthIncToken("filepath of the imported file", false, Token::Type::STRING);
        std::string full_path = std::filesystem::absolute(current_token->value);
        if(!included_full_paths.contains(full_path)){
            Parser sub_proc = getDefault(full_path); // maybe i can make it multi-threaded in the future
            included_full_paths.emplace(full_path);
            bool is_exported_import = is_export;
            is_export = false;
            sub_proc.parseToGlobals();
            is_export = is_exported_import;
            ModuleData module;
            module.functions.insert_range(sub_proc.functions);
            module.typedefs.insert_range(sub_proc.typedefs);
            module.enums.insert_range(sub_proc.enums);
            modules[full_path] = module;
        }
        ModuleData module = modules[full_path];
        iteratableAccessImportUpdate(module.functions, is_export);
        iteratableAccessImportUpdate(module.typedefs, is_export);
        iteratableAccessImportUpdate(module.enums, is_export);
        functions.insert_range(module.functions);
        typedefs.insert_range(module.typedefs);
        enums.insert_range(module.enums);
        is_export = false;
    }
    template <typename T>
    void iteratableAccessImportUpdate(std::multimap<std::string, T>& items, bool is_export){
        for(auto& [_, item] : items){
            itemAccessImportUpdate(item, is_export);
        }
    }
    void itemAccessImportUpdate(AccessHandler& handler, bool is_export){
        if(handler.will_export)
            handler.allowed_file_modules.push_back(filepath);
        handler.will_export &= is_export;
    }
    //to account for the duplicates that will be made before folding
    std::shared_ptr<Tokens> getFunctionBody(){
        auto start = current_token + 2;
        std::uint32_t indentation = 0;
        do{
            expectSmthGotSmthIncToken("closed off function body");
            if(current_token->type == Token::Type::BRACKET && current_token->value == "{")
                ++indentation;
            else if(current_token->type == Token::Type::BRACKET && current_token->value == "}")
                --indentation;
        }while(indentation != 0);
        return std::make_shared<Tokens>(std::vector(start, current_token));
    }
    void printFunction(const std::pair<std::string, Function>& fn){
        std::println("Function with name: {}", fn.first);
        std::println("Linkage: [extern: {}, allowed modules to access: {}]", fn.second.is_extern, fn.second.allowed_file_modules);
        std::println("---INPUT VARIABLES:---");
        printVariables(fn.second.inputs);
        std::println("---RETURN TYPE:---");
        printTypeAST(fn.second.ret_type);
        std::println("---IMPLEMENTATION:---");
        if(fn.second.implementation)
        for(const auto& token : **fn.second.implementation){
            println("Token: {}, with type: {}", token.value, lookupTokenTypeName(token.type));
        }
    }
    void printFunctions(){
        std::println("---FUNCTIONS:---");
        for(const auto& fn : functions){
            printFunction(fn);
        }
    }
    void printTypedefs(){
        std::println("---TYPEDEFS:---");
        for(const auto [name, type] : typedefs){
            std::println("Typename: {} and type:", name);
            printTypeAST(type);
        }
    }
    //automaticly goes one further.
    std::pair<std::string, Function> parseFunctionHead(){
        Function current_function;
        expectSmthGotSmthIncToken("function name", false, Token::Type::IDENTIFIER);
        std::string name = current_token->value;
        const auto& inc_token_arg_and_func_body = [&](){
            expectIncToken(std::format("On line: {}, arguments and function body missing.", current_token->line));
        };
        inc_token_arg_and_func_body();
        parseOptionalGeneric(current_function.generic_fields);
        if(current_function.generic_fields.size())
            inc_token_arg_and_func_body();
        current_function.inputs = parseInputVariables(current_function.generic_fields);
        ++current_token;
        if(current_token != tokens.end() && current_token->type == Token::Type::OPERATOR && current_token->value == "->"){
            expectSmthGotSmthIncToken("function return type");
            current_function.ret_type = parseType(current_function.generic_fields);
        }else{
            --current_token; // restore the original offset: function is expected to end with current_token set to the last used token
            current_function.ret_type = Type{.type_family = Type::Typename::VOID};
        }
        return {name, current_function};
    }
    void printVariables(const VariableIndex& vars){
        for(const auto& var : vars)
            printVariable(var);
    }
    void printVariable(const std::pair<std::string, Variable>& var){
        std::println("Variable: {}(constant: {}) with type:", var.first, var.second.is_constant);
        if(var.second.type)
            printTypeAST(*var.second.type);
        else
            std::println("Variable type: UNKNOWN");
    }
    Type parseType(const GenericFields& generics){
        switch(current_token->type) {
            case Token::Type::KEYWORD:
                return parseKeywordBasedType();
            case Token::Type::IDENTIFIER:
                return parseCustomType(generics);
            case Token::Type::OPERATOR:
                return parseOperatorBasedType(generics);
            case Token::Type::BRACKET:
                return parseBracketBasedType(generics);
            default:
                throw std::runtime_error{std::format("At line: {}, type expected, got: \"{}\".", current_token->line, current_token->value)};
        }
    }
    void parseTypeDecl(bool is_export){
        expectSmthGotSmthIncToken("typename", false, Token::Type::IDENTIFIER);
        const std::string& name = current_token->value;
        const std::string type_assign_msg = "type assignment";
        expectSmthGotSmthIncToken(type_assign_msg);
        GenericFields generic_fields;
        parseOptionalGeneric(generic_fields);
        if(!generic_fields.empty())
            expectSmthGotSmthIncToken(type_assign_msg);
        if(current_token->value != "=" || current_token->type != Token::Type::OPERATOR)
            expectSmthGotSmthIncToken(type_assign_msg, true);
        expectSmthGotSmthIncToken("type");
        Typedef type = static_cast<Typedef>(parseType(generic_fields));
        type.generic_fields = std::make_shared<GenericFields>(generic_fields);
        type.will_export = is_export;
        type.allowed_file_modules = {filepath};
        typedefs.emplace(name, type);
    }
    Type parseKeywordBasedType(){
        if(current_token->value == "i8")
            return Type{.type_family = Type::Typename::I8};
        if(current_token->value == "u8")
            return Type{.type_family = Type::Typename::U8};
        if(current_token->value == "i16")
            return Type{.type_family = Type::Typename::I16};
        if(current_token->value == "u16")
            return Type{.type_family = Type::Typename::U16};
        if(current_token->value == "i32")
            return Type{.type_family = Type::Typename::I32};
        if(current_token->value == "u32")
            return Type{.type_family = Type::Typename::U32};
        if(current_token->value == "i64")
            return Type{.type_family = Type::Typename::I64};
        if(current_token->value == "u64")
            return Type{.type_family = Type::Typename::U64};
        if(current_token->value == "void")
            return Type{.type_family = Type::Typename::VOID};
        throw std::runtime_error{std::format("Unexpected keyword on line: {}. Expected type keyword.", current_token->value)};
    }
    std::optional<Type> parseTypedefUse(const GenericFields& generics){
        const auto start_token_pos = current_token;
        std::string name = current_token->value;
        ++current_token;
        std::vector<Type> generic_args = parseGenericArguments(generics);
        GenericFieldTranslation translation_spec;
        //Do you know what's going on? Me neither!
        //Heck, I hate accessing multimaps
        //At least it works.... kinda.
        std::optional<Type> to_type;
        bool found = false;
        for(const auto& [key, type] : typedefs){
            if(std::ranges::any_of(type.allowed_file_modules,
                [&](const std::string& allowed_module){
                    bool match = allowed_module == env_path && key == name;
                    const auto option_translation_spec = linkGenericFieldTranslation(*type.generic_fields, generic_args);
                    if(option_translation_spec)
                        translation_spec = *option_translation_spec;
                    return option_translation_spec && match;
                }
            ))
                to_type = type;
        }
        if(to_type)
            applyGenericFieldTranslation(*to_type, translation_spec);
        else current_token = start_token_pos; //revert
        return to_type;
    }
    std::optional<Type> parseEnumUse(){
        std::string name = current_token->value;
        for(const auto& _enum : enums){
            if(std::any_of(_enum.second.allowed_file_modules.rbegin(), _enum.second.allowed_file_modules.rend(),
                [&](const std::string& allowed_module){
                    return allowed_module == env_path && _enum.first == name;
                }
            ))
                return Type{.type_family = Type::Typename::ENUM, .sub_data = (EnumItem)&_enum};
        }
        return {};
    }
    Type parseCustomType(const GenericFields& generics){
        Type type = Type{.sub_data = current_token->value};
        if(generics.contains(current_token->value)){
            type.type_family = Type::Typename::GENERIC;
            return type;
        }
        std::optional<Type> option_type = parseTypedefUse(generics);
        if(option_type) return *option_type;
        option_type = parseEnumUse();
        if(option_type) return *option_type;
        if(structs.contains(current_token->value)){
            type.type_family = Type::Typename::STRUCTURE;
            return type;
        }

        throw std::runtime_error{std::format("Unknown type/generic: {} at line: {}.", current_token->value, current_token->line)};
    }
    Type parseOperatorBasedType(const GenericFields& generics){
        if(current_token->value == "*"){
            expectIncToken(std::format("On line: {}, incomplete type to match: * (POINTER)", current_token->line));
            return Type{.type_family = Type::Typename::POINTER, .sub_data = std::vector{parseType(generics)}};
        }
        if(current_token->value == "?"){
            expectIncToken(std::format("On line: {}, incomplete type to match: ? (OPTIONAL)", current_token->line));
            return Type{.type_family = Type::Typename::OPTION, .sub_data = std::vector{parseType(generics)}};
        }
        if(current_token->value == "!"){
            expectIncToken(std::format("On line: {}, incomplete type to match: ! (RESULT)", current_token->line));
            return Type{.type_family = Type::Typename::RESULT, .sub_data = std::vector{parseType(generics)}};
        }
        throw std::runtime_error{
            std::format("On line {}, an invalid operator-based type found: \"{}\".", current_token->line, current_token->value)
        };
    }
    Type parseBracketBasedType(const GenericFields& generics){
        switch(current_token->value[0]){
            case '(':
                return parseTupleType(generics);
            case '[':
                return parseArrayType(generics);
            default:
                expectSmthGotSmthIncToken("type", true);
        }
        //makes the compiler SHUT UP
        return {};
    }
    //parse_call is the call that parses the items in the listing
    void parseNormalBracketEnumeration(std::function<void()> parse_call, char open_bracket = '(', char closing_bracket = ')', Token::Type token_type = Token::Type::BRACKET){
        const auto should_stop = [&](){
            if(current_token->value == ">>" && current_token->type == token_type && closing_bracket == '>'){ //hard-coded solution to a bug
                const auto i = std::distance(tokens.begin(), current_token); // iterator has no guarantee still being valid after a reallocation has been filed
                tokens.emplace(current_token, Token{.type = token_type, .value = ">", .line = current_token->line});
                current_token = tokens.begin() + i;
                (current_token + 1)->value = ">";
            }
            return current_token->value == std::string{closing_bracket} && current_token->type == token_type;
        };
        const std::string closing_bracket_msg = std::format("closing bracket(a.k.a '{}')", closing_bracket);
        const std::string comma_and_end_msg = std::format("comma(a.k.a ',') or {}", closing_bracket_msg);
        while(true){
            expectSmthGotSmthIncToken(closing_bracket_msg);
            if(should_stop())
                break;
            parse_call();
            expectSmthGotSmthIncToken(comma_and_end_msg);
            if(should_stop())
                break;
            if(current_token->type != Token::Type::COMMA)
                expectSmthGotSmthIncToken(comma_and_end_msg, true);
        }
    }
    Enum parseEnumItems(bool is_export){
        std::vector<std::string> items;
        parseNormalBracketEnumeration(
        [&](){
            if(current_token->type != Token::Type::IDENTIFIER)
                expectSmthGotSmthIncToken("enum field name", true);
            items.push_back(current_token->value);
        }, '{', '}');
        return Enum{is_export, {filepath}, false, std::make_unique<std::vector<std::string>>(items)};
    }
    std::vector<Type> parseGenericArguments(const GenericFields& generics){
        if(current_token == tokens.end() || current_token->value != "<" || current_token->type != Token::Type::OPERATOR){
            --current_token;
            return {};
        }
        std::vector<Type> out;
        parseNormalBracketEnumeration([&](){out.push_back(parseType(generics));}, '<', '>', Token::Type::OPERATOR);
        return out;
    }
    std::optional<GenericFieldTranslation> linkGenericFieldTranslation(const GenericFields& generics, std::vector<Type> aliases){
        if(generics.size() != aliases.size())
            return {};
        GenericFieldTranslation out;
        for(const auto& [index, generic] : std::views::enumerate(generics)){
            out[generic] = aliases[index];
        }
        return out;
    }
    void applyGenericFieldTranslation(Type& type, const GenericFieldTranslation& translation_spec){
        switch(type.type_family){
            case Type::Typename::ARRAY:
            {
                auto& [type_array, array_size] = std::get<std::pair<std::shared_ptr<Type>, std::uint64_t>>(type.sub_data);
                //copy to avoid having to change the other instances
                auto copy = *type_array;
                applyGenericFieldTranslation(copy, translation_spec);
                type_array = std::make_shared<Type>(copy);
                return;
            }
            case Type::Typename::TUPLE:
            {
                auto& types = std::get<std::vector<Type>>(type.sub_data);
                for(auto& type : types)
                    applyGenericFieldTranslation(type, translation_spec);
                return;
            }
            case Type::Typename::POINTER:
            {
                auto& ptr_subtype = std::get<std::vector<Type>>(type.sub_data).front();
                applyGenericFieldTranslation(ptr_subtype, translation_spec);
                return;
            }
            case Type::Typename::GENERIC:
                const std::string& name = std::get<std::string>(type.sub_data);
                if(translation_spec.contains(name)){
                    type = translation_spec.at(name);
                }
                return;
        }
    }
    VariableIndex parseInputVariables(const GenericFields& generics){
        VariableIndex var_index;
        parseNormalBracketEnumeration([&](){
            var_index.emplace(parseVariableDeclaration(generics));
        });
        return var_index;
    }
    std::pair<std::string, Variable> parseVariableDeclaration(const GenericFields& generics, bool ignore_arg_list = false){
        static const std::string type_expected_err_msg_input = "variable type";
        Variable var{
            .is_constant = current_token->type == Token::Type::KEYWORD&&current_token->value == "const",
            .decl_line = current_token->line
        };
        if(var.is_constant)
            expectSmthGotSmthIncToken("variable name", false, Token::Type::IDENTIFIER);
        const std::string name = current_token->value;
        expectSmthGotSmthIncToken(type_expected_err_msg_input, false, Token::Type::OPERATOR); // if := or : is missing, so is the type probably.
        if(current_token->value == ":"){
            expectSmthGotSmthIncToken(type_expected_err_msg_input);
            var.type = parseType(generics);
            if(++current_token == tokens.end() || current_token->value != "=" || current_token->type != Token::Type::OPERATOR){
                --current_token;
                return {name, var}; // no assignment... returning.
            }
        }else if(current_token->value != ":=")
            expectSmthGotSmthIncToken(type_expected_err_msg_input);
        const Tokens used_tokens = getTokensUntilCommaForFunction(ignore_arg_list);
        if(!used_tokens.empty()){
            const auto token_views = getTokenViews(used_tokens, generics);
            const auto lines = getLines(token_views);
            if(lines.size() > 1)
                throw std::runtime_error{std::format("Only one line of code expected for default value on line: {}", current_token->line)};
            var.default_value = std::make_shared<TokenASTItem>(getLineAST(lines.front(), used_tokens.front().line, generics));
        }
        return {name, var};
    }
    std::vector<Token> getTokensUntilCommaForFunction(bool ignore_arg_list = false){
        std::vector<Token> out;
        std::int32_t indent = 1;

        while(true){
            if(!ignore_arg_list)
                expectSmthGotSmthIncToken("end of argument list");
            else{
                ++current_token;
                if(current_token == tokens.end())
                    return out;
            }
            out.push_back(*current_token);
            if(current_token->type == Token::Type::BRACKET){
                const std::int32_t index = std::distance(brackets.begin(), std::ranges::find(brackets, current_token->value[0]));
                indent += ((index % 2) ? -1: 1);
            }
            if(indent == 0 || (indent == 1 && current_token->type == Token::Type::COMMA)){
                out.pop_back();
                --current_token;
                return out;
            }
        }
    }
    Type parseTupleType(const GenericFields& generics){
        std::vector<Type> types;
        Type type_out{.type_family = Type::Typename::TUPLE};
        parseNormalBracketEnumeration([&](){
            types.push_back(parseType(generics));
        });
        if(types.size() < 2)
            throw std::runtime_error{std::format("Misformed type on line: {}. A tuple must have at least two subtypes, got {}.", (current_token-1)->line, types.size())};
        type_out.sub_data = types;
        return type_out;
    }
    Type parseArrayType(const GenericFields& generics){
        Type type_out{.type_family = Type::Typename::ARRAY};
        expectSmthGotSmthIncToken("the type of the array");
        std::shared_ptr<Type> sub_type = std::make_shared<Type>(parseType(generics));
        expectSmthGotSmthIncToken("semicolon(a.k.a ';')", false, Token::Type::SEMICOLON, ";");
        expectSmthGotSmthIncToken("array size", false, Token::Type::NUMBER);
        std::uint64_t size = std::stoll(current_token->value); //i mean... i already checked for if it's a number.
        expectSmthGotSmthIncToken("array type closing bracket(a.k.a ']')", false, Token::Type::BRACKET, "]");
        return Type{.type_family = Type::Typename::ARRAY, .sub_data = std::make_pair<std::shared_ptr<Type>, std::uint64_t>(std::move(sub_type), std::move(size))};
    }
    void expectIncToken(const std::string msg){
        ++current_token;
        if(current_token == tokens.end())
            throw std::runtime_error{msg};
    }
    std::string expectSmthGotSmthMsg(const std::string& expected){
        return std::format("\nExpected {} on line {}.", expected, current_token->line);
    }
    std::string expectSmthGotSmthButGotExtensionMsg(){
        return std::format("\nGot {}({})", current_token->value, lookupTokenTypeName(current_token->type));
    }
    void expectSmthGotSmthIncToken(const std::string& expected, bool ignore_inc = false, std::optional<Token::Type> type_req = {}, const std::string& exact_match = ""){
        const std::string msg = expectSmthGotSmthMsg(expected);
        if(ignore_inc)
            throw std::runtime_error{msg+expectSmthGotSmthButGotExtensionMsg()};
        expectIncToken(msg + "\nBut the code ended unexpectedly!");
        if((type_req && current_token->type != *type_req) || (!exact_match.empty() && current_token->value != exact_match))
            throw std::runtime_error{msg+expectSmthGotSmthButGotExtensionMsg()};
    }
    void parseOptionalGeneric(GenericFields& generics){
        if(current_token->type != Token::Type::OPERATOR || current_token->value != "<")
            return;

        const auto& expect_inc_token_local = [&](){
            expectIncToken(std::format("On line: {}, missing '>' to match '<'", current_token->line));
            return current_token->type != Token::Type::OPERATOR || current_token->value != ">";
        };
        while(true){
            if(!expect_inc_token_local())
                return;
            if(current_token->type != Token::Type::IDENTIFIER)
                throw std::runtime_error{
                    std::format("Expected generic name on line: {}. Got: \"{}\"({}).",
                        current_token->line, current_token->value, lookupTokenTypeName(current_token->type))
                };
            if(generics.contains(current_token->value))
                throw std::runtime_error{std::format("Redefinition of generic: \"{}\" on line: {}.", current_token->value, current_token->line)};
            generics.emplace(current_token->value);
            if(!expect_inc_token_local())
                return;
            if(current_token->type != Token::Type::COMMA)
                expectSmthGotSmthIncToken("comma(a.k.a ',') for template arguments",true);
        }
    }
    static std::string lookupVariableTypeName(const Type::Typename type) noexcept {
        switch(type){
            case Type::Typename::ARRAY:
                return "Array";
            case Type::Typename::I8:
                return "Signed Integer(8-bit)";
            case Type::Typename::I16:
                return "Signed Integer(16-bit)";
            case Type::Typename::I32:
                return "Signed Integer(32-bit)";
            case Type::Typename::I64:
                return "Signed Integer(64-bit)";
            case Type::Typename::U8:
                return "Unsigned Integer(8-bit)";
            case Type::Typename::U16:
                return "Unsigned Integer(16-bit)";
            case Type::Typename::U32:
                return "Unsigned Integer(32-bit)";
            case Type::Typename::U64:
                return "Unsigned Integer(64-bit)";
            case Type::Typename::STRUCTURE:
                return "Structure";
            case Type::Typename::TUPLE:
                return "Tuple";
            case Type::Typename::POINTER:
                return "Pointer";
            case Type::Typename::OPTION:
                return "Option";
            case Type::Typename::RESULT:
                return "Result";
            case Type::Typename::ENUM:
                return "Enum";
            case Type::Typename::VOID:
                return "Void";
            case Type::Typename::GENERIC:
                return "Generic";
        }
        return "";
    }
    static void printTypeAST(const Type& type){
        static std::string indent;
        std::println("{}Type: {} with children", indent, lookupVariableTypeName(type.type_family));
        std::println("{}{{", indent);
        indent += "\t";
        switch(type.type_family){
            case Type::Typename::ARRAY:
            {
                const auto& [type_array, size] = std::get<std::pair<std::shared_ptr<Type>, std::uint64_t>>(type.sub_data);
                std::println("{}Size: {} and type:", indent, size);
                printTypeAST(*type_array);
                break;
            }
            case Type::Typename::TUPLE:
            {
                const auto& types = std::get<std::vector<Type>>(type.sub_data);
                for(const auto& type : types){
                    printTypeAST(type);
                }
                break;
            }
            case Type::Typename::POINTER:
            case Type::Typename::RESULT:
            case Type::Typename::OPTION:
            {
                printTypeAST(std::get<std::vector<Type>>(type.sub_data).front());
                break;
            }
            case Type::Typename::GENERIC:
            case Type::Typename::STRUCTURE:
                std::println("{}Name of type: {}", indent, std::get<std::string>(type.sub_data));
                break;
            case Type::Typename::ENUM:
                EnumItem _enum = std::get<EnumItem>(type.sub_data);
                std::println("{}Name of enum type: {}", indent, _enum->first);
        }
        indent.pop_back();
        std::println("{}}}", indent);
    }

    const std::string filepath;
    Tokens tokens;
    Tokens::iterator current_token;
    Tokens current_implementation_source;
    std::multimap<std::string, Typedef> typedefs; //multimap to account for the possibility of module overloads.
    std::multimap<std::string, Struct> structs;
    Enums enums;
    Functions functions;
    //environment path is the path of the current file being processed, this would differ from filepath when parsing the inner-functions after including all modules
    std::string env_path;
    static std::set<std::string> included_full_paths;
    static std::map<std::string, ModuleData> modules;
};
std::set<std::string> Parser::included_full_paths = {};
std::map<std::string, Parser::ModuleData> Parser::modules = {};
