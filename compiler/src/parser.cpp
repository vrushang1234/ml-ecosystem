/* Author: Vrushang Anand */
/* Last Date Modified: Jul 13, 2026 */
/* Source file for the Parser for a custom compiler for the ML Inference
 * Ecosystem */

#include "parser.hpp"

#include <stdexcept>
#include <string>
#include <utility>

Parser::Parser(std::vector<Token> tokens)
    : tokens(std::move(tokens))
{
}

const Token& Parser::peek() const
{
    return tokens[pos];
}

const Token& Parser::advance()
{
    const Token& token = tokens[pos];
    if (token.type != TokenType::END_OF_FILE) {
        pos++;
    }
    return token;
}

bool Parser::check(TokenType type) const
{
    return peek().type == type;
}

const Token& Parser::expect(TokenType type, const std::string& what)
{
    if (!check(type)) {
        throw std::runtime_error("Parse error: expected " + what + " but got '" + peek().value +
                                 "'");
    }
    return advance();
}

Program Parser::parseProgram()
{
    Program program;
    while (!check(TokenType::END_OF_FILE)) {
        program.networks.push_back(parseNetwork());
    }
    return program;
}

NetworkDecl Parser::parseNetwork()
{
    NetworkDecl network;
    expect(TokenType::KEYWORD_NETWORK, "'network'");
    network.name = expect(TokenType::IDENTIFIER, "network name").value;
    expect(TokenType::LEFTBRACE, "'{'");
    network.input = parseInput();
    while (check(TokenType::KEYWORD_LAYER)) {
        network.layers.push_back(parseLayer());
    }
    expect(TokenType::RIGHTBRACE, "'}'");
    return network;
}

TensorDecl Parser::parseInput()
{
    expect(TokenType::KEYWORD_INPUT, "'input'");
    return parseTensor();
}

LayerDecl Parser::parseLayer()
{
    LayerDecl layer;
    expect(TokenType::KEYWORD_LAYER, "'layer'");
    layer.name = expect(TokenType::IDENTIFIER, "layer name").value;
    expect(TokenType::LEFTBRACE, "'{'");

    expect(TokenType::KEYWORD_WEIGHTS, "'weights'");
    expect(TokenType::COLON, "':'");
    layer.weights = parseTensor();

    expect(TokenType::KEYWORD_BIASES, "'bias'");
    expect(TokenType::COLON, "':'");
    layer.bias = parseTensor();

    expect(TokenType::KEYWORD_ACTIVATION, "'activation'");
    expect(TokenType::COLON, "':'");
    layer.activation = expect(TokenType::IDENTIFIER, "activation name").value;

    expect(TokenType::RIGHTBRACE, "'}'");
    return layer;
}

TensorDecl Parser::parseTensor()
{
    TensorDecl tensor;
    tensor.name = expect(TokenType::IDENTIFIER, "tensor name").value;
    tensor.dims = parseDims();
    return tensor;
}

std::vector<std::optional<long>> Parser::parseDims()
{
    std::vector<std::optional<long>> dims;
    expect(TokenType::LEFTBRACK, "'['");
    while (true) {
        if (check(TokenType::NUMBER)) {
            dims.push_back(std::stol(advance().value));
        }
        else {
            dims.push_back(std::nullopt);
        }
        expect(TokenType::RIGHTBRACK, "']'");
        if (!check(TokenType::LEFTBRACK)) {
            break;
        }
        advance();
    }
    return dims;
}

Program parse(const std::vector<Token>& tokens)
{
    Parser parser(tokens);
    return parser.parseProgram();
}
