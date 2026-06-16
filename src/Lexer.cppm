export module lexer;
import std;

export struct Operator{
    std::string name;
    enum Type : std::uint8_t{
        LEFT = 1,
        RIGHT = 2,
        BOTH = 3
    }type = BOTH;
};

export class Lexer{
    public:

    struct Token{
        enum class Type{
            OPERATOR,
            STRING,
            BRACKET,
            NUMBER,
            KEYWORD,
            COMMA,
            CHAR,
            IDENTIFIER,
            SEMICOLON //for the array type
        } type;
        std::string value;
        std::size_t line;
    };

    Lexer(const std::string& contents, const std::set<std::string>& keywords, const std::vector<std::vector<Operator>>& operators_ordered, const std::string& brackets)
    : contents(contents), keywords(keywords), operators_ordered(operators_ordered), brackets(brackets){}

    //! if you would like to make a custom lexer, please override this one. You can use the other functions to implement custom features. lexBase is the core function for that.
    virtual std::optional<Token> getNextToken(){
        static bool wait_until_next_line = false;
        const auto character = getChar();
        if(!character)
            return std::optional<Token>{};

        if(std::isspace(*character)) {
            if(*character == '\n'){
                ++line_index;
                wait_until_next_line = false;
            }
            ++contents_read_cursor;
            return getNextToken();
        }
        if(wait_until_next_line){
            wait:
            ++contents_read_cursor;
            return getNextToken();
        }
        if(std::isalpha(*character) || *character == '_')
            return lexIdentifierToken();

        if(std::isdigit(*character))
            return lexNumberToken();

        switch(*character){
            case '"':
                return lexEncapsulatedToken('"', Token::Type::STRING);
            case '\'':
                return lexEncapsulatedToken('\'', Token::Type::CHAR);
            case ',':
                ++contents_read_cursor;
                return Token{.type = Token::Type::COMMA, .value = ",", .line = line_index};
            case ';':
                ++contents_read_cursor;
                return Token{.type = Token::Type::SEMICOLON, .value = ";", .line = line_index};
            case '#':
                wait_until_next_line = true;
                goto wait;
        }
        const auto bracket_token = checkForAndCreatePossibleBracketToken(*character);
        if(bracket_token)
            return *bracket_token;

        return lexOperatorToken();
    }

    constexpr static std::string lookupTokenTypeName(Token::Type token_type){
        switch(token_type){
            case Token::Type::OPERATOR:
                return "Operator";
            case Token::Type::STRING:
                return "String";
            case Token::Type::BRACKET:
                return "Bracket";
            case Token::Type::NUMBER:
                return "Number";
            case Token::Type::KEYWORD:
                return "Keyword";
            case Token::Type::COMMA:
                return "Comma";
            case Token::Type::CHAR:
                return "Char";
            case Token::Type::IDENTIFIER:
                return "Identifier";
            case Token::Type::SEMICOLON:
                return "Semicolon";
        }
        return ""; //stops the compiler from complaining
    }

    private:

    inline std::optional<char> getChar(){
        if(contents_read_cursor == contents.size())
            return std::optional<char>{};
        return contents.at(contents_read_cursor);
    }

    //core parsing function
    Token lexBase(const std::function<bool(char)>& is_valid, const Token::Type type){
        static std::string token_value;
        const auto character = getChar();
        if(!character || !is_valid(*character)) {
            Token out_token = Token{.type = type, .value = token_value, .line = line_index};
            token_value = "";
            return out_token;
        }
        token_value.push_back(*character);
        ++contents_read_cursor;
        return lexBase(is_valid, type);
    }

    inline Token lexIdentifierToken(){
        Token tok = lexBase([](const char character){return std::isalnum(character) || character == '_';}, Token::Type::IDENTIFIER);
        if(std::ranges::find(keywords, tok.value) != keywords.end())
            tok.type = Token::Type::KEYWORD;
        if(validateOperatorToken(tok.value))
            tok.type = Token::Type::OPERATOR;
        return tok;
    }

    inline Token lexNumberToken(){
        return lexBase([](const char character){return std::isdigit(character) || character == '.';}, Token::Type::NUMBER);
    }

    //for parsing encapsulated things like strings. Doesn't check for the 1st one and skips it. The used_character isn't included in the result.
    Token lexEncapsulatedToken(char used_character, Token::Type type){
        static bool prev_backslash = false;
        static std::string token_value;
        ++contents_read_cursor;
        const auto character = getChar();
        if(!character || *character == '\n')
            throw std::runtime_error{std::format("Unclosed string at line: {}", line_index)};

        if(prev_backslash) {
            prev_backslash = false;
            handleEscapeSequence(character, token_value, used_character);
        }
        else if(*character == '\\')
            prev_backslash = true;
        else if(*character == used_character) {
            Token ret_token = Token{.type = type, .value = token_value, .line = line_index};
            token_value = "";
            ++contents_read_cursor;
            return ret_token;
        }
        else
            token_value.push_back(*character);
        return lexEncapsulatedToken(used_character, type);
    }

    void handleEscapeSequence(const std::optional<char>& character, std::string& token_value, const char used_character){
        switch(*character){
            case '\\':
                token_value.push_back('\\'); break;
            case 'n':
                token_value.push_back('\n'); break;
            case 'v':
                token_value.push_back('\v'); break;
            case 't':
                token_value.push_back('\t'); break;
            case '0':
                token_value.push_back('\0'); break;
            case 'a':
                token_value.push_back('\a'); break;
            default:
                if(*character == used_character)
                    token_value.push_back(used_character); break;
                throw std::runtime_error{std::format("Unknown escape sequence at line: {}", line_index)};
        }
    }

    Token lexOperatorToken(){
        static std::string token_name;
        const auto character = getChar();

        if(operatorTokenNameCouldExtend(token_name, character))
            token_name += *character;
        else if(token_name.size() == 0)
            throw std::runtime_error{std::format("Unknown symbol: {} at line: {}", *character, line_index)};
        else if(validateOperatorToken(token_name)){
            Token out_token = Token{.type = Token::Type::OPERATOR, .value = token_name, .line = line_index};
            token_name = "";
            return out_token;
        }
        else throw std::runtime_error{std::format("Unknown operator: {} at line: {}", token_name, line_index)};
        ++contents_read_cursor;
        return lexOperatorToken();
    }

    std::optional<Token> checkForAndCreatePossibleBracketToken(const char character){
        if(brackets.contains(character)){
            ++contents_read_cursor;
            return Token{.type = Token::Type::BRACKET, .value = std::string{character}, .line = line_index};
        }
        else
            return std::optional<Token>{};
    }

    bool operatorTokenNameCouldExtend(const std::string& token_name, const std::optional<char>& character){
        return character && std::ranges::any_of(operators_ordered,
            [&](const auto& operators)
            {
                return std::ranges::any_of(operators,
                    [&](const auto& operator_)
                    {
                        return operator_.name.size() > token_name.size() &&         //is there even anything to be added for this operator
                                !std::isalpha(operator_.name.at(0)) &&              //is it NOT an alphabetic-based operator(handled elsewhere)
                                operator_.name.starts_with(token_name) &&           //is it compatible with the operator
                                operator_.name.at(token_name.size()) == *character; //is the character the same as the next operator character
                    }
                    );
            }
            );
    }

    inline bool validateOperatorToken(const std::string& operator_name){
        return std::ranges::any_of(operators_ordered,
            [&](const auto& operators)
            {
                return std::ranges::any_of(operators, [&](const auto& operator_){return operator_.name == operator_name;});
            }
        );
    }

    protected:

    const std::string contents;
    const std::string brackets;
    const std::set<std::string> keywords;
    const std::vector<std::vector<Operator>> operators_ordered; //reverse-mathematical-order
    std::size_t contents_read_cursor = 0, line_index = 1;
};
