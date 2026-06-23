#include "helper.hpp"

#include <iostream>

std::string tokenTypeToString(TokenType type)
{
    switch (type) {
        case TokenType::KEYWORD_NETWORK:
            return "KEYWORD_NETWORK";
        case TokenType::KEYWORD_INPUT:
            return "KEYWORD_INPUT";
        case TokenType::LEFTBRACE:
            return "LEFTBRACE";
        case TokenType::RIGHTBRACE:
            return "RIGHTBRACE";
        case TokenType::KEYWORD_LAYER:
            return "KEYWORD_LAYER";
        case TokenType::KEYWORD_WEIGHTS:
            return "KEYWORD_WEIGHTS";
        case TokenType::KEYWORD_BIASES:
            return "KEYWORD_BIASES";
        case TokenType::KEYWORD_ACTIVATION:
            return "KEYWORD_ACTIVATION";
        case TokenType::LEFTBRACK:
            return "LEFTBRACK";
        case TokenType::RIGHTBRACK:
            return "RIGHTBRACK";
        case TokenType::IDENTIFIER:
            return "IDENTIFIER";
        case TokenType::NUMBER:
            return "NUMBER";
        default:
            return "UNKNOWN";
    }
}

void printTokens(const std::vector<Token>& tokenStream)
{
    for (const Token& token : tokenStream) {
        std::cout << "Type: " << tokenTypeToString(token.type) << ", Value: " << token.value
                  << std::endl;
    }
}
