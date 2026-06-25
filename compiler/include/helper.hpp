#pragma once

#include "lexer.hpp"

#include <string>
#include <vector>

void printTokens(const std::vector<Token>& tokenStream);
std::string tokenTypeToString(TokenType type);
