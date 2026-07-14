/* Author: Vrushang Anand */
/* Last Date Modified: Jul 13, 2026 */
/* Header file for the Parser for a custom compiler for the ML Inference
 * Ecosystem */

#pragma once
#include "ast.hpp"
#include "lexer.hpp"

#include <cstddef>
#include <vector>

class Parser
{
public:
    explicit Parser(std::vector<Token> tokens);

    Program parseProgram();

private:
    std::vector<Token> tokens;
    size_t pos = 0;

    const Token& peek() const;
    const Token& advance();
    const Token& expect(TokenType type, const std::string& what);
    bool check(TokenType type) const;

    NetworkDecl parseNetwork();
    TensorDecl parseInput();
    LayerDecl parseLayer();
    TensorDecl parseTensor();
    std::vector<std::optional<long>> parseDims();
};

Program parse(const std::vector<Token>& tokens);
