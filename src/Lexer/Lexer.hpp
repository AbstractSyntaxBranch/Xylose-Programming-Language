#pragma once
#include "../Data/Token.hpp"
#include <string>
#include <vector>

class Lexer{
public:
    Lexer(std::string& source);
    [[nodiscard]] std::vector<Token> splitIntoTokens();
private:
    static bool isIdenOrKwChar(char c);
    static bool isNumberChar(char c);
    static bool isBracketOpenChar(char c);
    static bool isBracketCloseChar(char c);
    static bool isOperatorNonAlphaStart(char c);
    std::vector<Token> getEnclosedToken(char enclosure_symbol, Token::Type type, bool allow_fmt);
    char getRegularEscapeCharacter();
    void appendEscapeCharacter(Token& current_token, std::vector<Token>& tokens, Token::Type type, bool allow_fmt);
    Token getIdentifierOrKeywordToken();
    Token getNumberToken();
    Token getOperatorToken();
    Token getSingleCharToken(Token::Type type);
    void handleFmt(Token& current_token, std::vector<Token>& tokens, Token::Type type);
    std::vector<Token> getStringToken();
    std::vector<Token> getCharToken();
    bool isReadPosValid();
    bool getNextToken();

    std::size_t M_max_num_extension_size, M_max_operator_size;
    std::uint64_t M_line = 1;
    std::string& M_source;
    std::vector<Token> M_tokens;
    const char *M_read_position;
};
