/* Author: Vrushang Anand */
/* Last Date Modified: Jul 13, 2026 */
/* Source file for the AST for a custom compiler for the ML Inference
 * Ecosystem */

#include "ast.hpp"

#include <iostream>
#include <string>

static std::string dimsToString(const TensorDecl& tensor)
{
    std::string out;
    for (const auto& dim : tensor.dims) {
        out += '[';
        if (dim.has_value()) {
            out += std::to_string(dim.value());
        }
        out += ']';
    }
    return out;
}

void printAST(const Program& program)
{
    for (const NetworkDecl& network : program.networks) {
        std::cout << "network " << network.name << '\n';
        std::cout << "  input " << network.input.name << dimsToString(network.input) << '\n';
        for (const LayerDecl& layer : network.layers) {
            std::cout << "  layer " << layer.name << '\n';
            std::cout << "    weights: " << layer.weights.name << dimsToString(layer.weights)
                      << '\n';
            std::cout << "    bias: " << layer.bias.name << dimsToString(layer.bias) << '\n';
            std::cout << "    activation: " << layer.activation << '\n';
        }
    }
}
