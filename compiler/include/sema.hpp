/* Author: Vrushang Anand */
/* Last Date Modified: Jul 14, 2026 */
/* Header file for the Semantic Analyzer for a custom compiler for the ML
 * Inference Ecosystem */

#pragma once
#include "ast.hpp"

#include <string>
#include <vector>

enum class ActivationKind
{
    RELU,
    SOFTMAX,
    LINEAR
};

struct ResolvedLayer
{
    std::string name;
    long inSize;
    long outSize;
    ActivationKind activation;
    std::string weightsName;
    std::string biasName;
};

struct ResolvedNetwork
{
    std::string name;
    std::string inputName;
    long inputSize;
    std::vector<ResolvedLayer> layers;
};

struct SemaResult
{
    std::vector<std::string> errors;
    std::vector<ResolvedNetwork> networks;
    std::vector<Constant> constants;
};

SemaResult analyze(const Program& program);
void printResolved(const SemaResult& result);
