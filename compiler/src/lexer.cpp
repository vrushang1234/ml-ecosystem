/* Author: Vrushang Anand */
/* Last Date Modified: Jun 23, 2026 */
/* Source file for the Lexer for a custom compiler for the ML Inference
 * Ecosystem */

#include "lexer.hpp"

#include <sstream>
#include <stdexcept>
#include <string>
#include <unordered_map>

std::unordered_map<std::string, TokenType> tokens{{"network", TokenType::KEYWORD_NETWORK},
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
    if (str[0] > 47 && str[0] < 58) {
        return false;
    }
    for (char c : str) {
        if (!((c > 47 && c < 58) || (c > 64 && c < 91) || (c > 96 && c < 123) || (c == 95))) {
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
        if (tokens.find(temp_char) != tokens.end()) {
            if (buffer.size() != 0) {
                tokenList.push_back(getToken(buffer));
            }
            tokenList.push_back(getToken(temp_char));
            buffer.clear();
        }
        else if (c == 32) {
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
    return tokenList;
}

Token getToken(const std::string& s)
{
    if (tokens.find(s) != tokens.end()) {
        return Token{tokens[s], s};
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
