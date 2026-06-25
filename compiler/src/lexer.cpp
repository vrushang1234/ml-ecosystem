/* Author: Vrushang Anand */
/* Last Date Modified: Jun 25, 2026 */
/* Source file for the Lexer for a custom compiler for the ML Inference
 * Ecosystem */

#include "lexer.hpp"

#include <cctype>
#include <sstream>
#include <stdexcept>
#include <string>
#include <unordered_map>

const std::unordered_map<std::string, TokenType> tokenMap{
    {"network", TokenType::KEYWORD_NETWORK},
    {"input", TokenType::KEYWORD_INPUT},
    {"{", TokenType::LEFTBRACE},
    {"}", TokenType::RIGHTBRACE},
    {"layer", TokenType::KEYWORD_LAYER},
    {"weights", TokenType::KEYWORD_WEIGHTS},
    {"bias", TokenType::KEYWORD_BIASES},
    {"activation", TokenType::KEYWORD_ACTIVATION},
    {"[", TokenType::LEFTBRACK},
    {"]", TokenType::RIGHTBRACK},
    {":", TokenType::COLON}};

bool isNumber(const std::string& str)
{
    std::istringstream in(str);
    long val;
    return (in >> val) && in.eof();
}

bool isIdentifier(const std::string& str)
{
    if (str.empty()) {
        return false;
    }
    if (str[0] >= '0' && str[0] <= '9') {
        return false;
    }
    for (char c : str) {
        if (!((c >= '0' && c <= '9') || (c >= 'A' && c <= 'Z') || (c >= 'a' && c <= 'z') ||
              (c == '_'))) {
            return false;
        }
    }
    return true;
}

std::vector<Token> tokenize(const std::string& s)
{
    std::string buffer;
    std::vector<Token> tokenList;
    for (char c : s) {
        std::string temp_char(1, c);
        if (tokenMap.find(temp_char) != tokenMap.end()) {
            if (buffer.size() != 0) {
                tokenList.push_back(getToken(buffer));
            }
            tokenList.push_back(getToken(temp_char));
            buffer.clear();
        }
        else if (std::isspace(static_cast<unsigned char>(c))) {
            if (buffer.size() != 0) {
                tokenList.push_back(getToken(buffer));
            }
            buffer.clear();
        }
        else {
            buffer += c;
        }
    }
    if (buffer.size() != 0)
        tokenList.push_back(getToken(buffer));
    tokenList.push_back(Token{TokenType::END_OF_FILE, "EOF"});
    return tokenList;
}

Token getToken(const std::string& s)
{
    auto token_it = tokenMap.find(s);
    if (token_it != tokenMap.end()) {
        return Token{token_it->second, s};
    }
    else if (isNumber(s)) {
        return Token{TokenType::NUMBER, s};
    }
    else if (isIdentifier(s)) {
        return Token{TokenType::IDENTIFIER, s};
    }
    else {
        throw std::runtime_error("Invalid Token Request");
    }
}
