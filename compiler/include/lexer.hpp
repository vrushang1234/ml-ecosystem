/* Author: Vrushang Anand */
/* Last Date Modified: Jun 25, 2026 */
/* Header file for the Lexer for a custom compiler for the ML Inference
 * Ecosystem */

#pragma once
#include <string>
#include <vector>

enum class TokenType
{
    KEYWORD_NETWORK,
    KEYWORD_INPUT,
    LEFTBRACE,
    RIGHTBRACE,
    KEYWORD_LAYER,
    KEYWORD_WEIGHTS,
    KEYWORD_BIASES,
    KEYWORD_ACTIVATION,
    LEFTBRACK,
    RIGHTBRACK,
    IDENTIFIER,
    NUMBER,
    COLON,
    END_OF_FILE
};

struct Token
{
    TokenType type;
    std::string value;
};

Token getToken(const std::string& s);
std::vector<Token> tokenize(const std::string& s);
bool isNumber(const std::string& str);
bool isIdentifier(const std::string& str);
