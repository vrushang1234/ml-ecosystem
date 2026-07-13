/* Author: Vrushang Anand */
/* Last Date Modified: Jul 13, 2026 */
/* Header file for the AST for a custom compiler for the ML Inference
 * Ecosystem */

#pragma once
#include <optional>
#include <string>
#include <vector>

struct TensorDecl
{
    std::string name;
    std::vector<std::optional<long>> dims;

    size_t rank() const
    {
        return dims.size();
    }
};

struct LayerDecl
{
    std::string name;
    TensorDecl weights;
    TensorDecl bias;
    std::string activation;
};

struct NetworkDecl
{
    std::string name;
    TensorDecl input;
    std::vector<LayerDecl> layers;
};

struct Program
{
    std::vector<NetworkDecl> networks;
};

void printAST(const Program& program);
