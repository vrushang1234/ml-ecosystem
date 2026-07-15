/* Author: Vrushang Anand */
/* Last Date Modified: Jul 14, 2026 */
/* Source file for the Semantic Analyzer for a custom compiler for the ML
 * Inference Ecosystem */

#include "sema.hpp"

#include <algorithm>
#include <cstddef>
#include <iostream>
#include <optional>
#include <string>
#include <unordered_map>
#include <unordered_set>
#include <vector>

const std::unordered_map<std::string, ActivationKind> activationMap{
    {"relu", ActivationKind::RELU}, {"softmax", ActivationKind::SOFTMAX}};

struct Source
{
    long value;
    std::string desc;
};

static std::string activationToString(ActivationKind kind)
{
    switch (kind) {
        case ActivationKind::RELU:
            return "relu";
        case ActivationKind::SOFTMAX:
            return "softmax";
    }
    return "";
}

static std::string validActivations()
{
    std::vector<std::string> names;
    for (const auto& entry : activationMap) {
        names.push_back(entry.first);
    }
    std::sort(names.begin(), names.end());
    std::string out;
    for (const std::string& name : names) {
        if (!out.empty()) {
            out += ", ";
        }
        out += name;
    }
    return out;
}

static bool checkTensor(const TensorDecl& tensor,
                        size_t expectedRank,
                        const std::string& kind,
                        const std::string& networkName,
                        std::vector<std::string>& errors)
{
    bool ok = true;
    if (tensor.rank() != expectedRank) {
        errors.push_back(kind + " '" + tensor.name + "' in network '" + networkName +
                         "' must have rank " + std::to_string(expectedRank) + " but has rank " +
                         std::to_string(tensor.rank()));
        ok = false;
    }
    for (const auto& dim : tensor.dims) {
        if (dim.has_value() && dim.value() < 1) {
            errors.push_back(kind + " '" + tensor.name + "' in network '" + networkName +
                             "' has invalid dimension " + std::to_string(dim.value()));
            ok = false;
        }
    }
    return ok;
}

static void analyzeNetwork(const NetworkDecl& network, size_t errorsBefore, SemaResult& result)
{
    std::vector<std::string>& errors = result.errors;

    std::unordered_map<std::string, std::string> seen;
    auto declare = [&](const std::string& name, const std::string& kind) {
        auto [it, inserted] = seen.emplace(name, kind);
        if (!inserted) {
            errors.push_back("duplicate name '" + name + "' in network '" + network.name +
                             "': " + kind + " conflicts with " + it->second);
        }
    };
    declare(network.input.name, "input");
    for (const LayerDecl& layer : network.layers) {
        declare(layer.name, "layer");
        declare(layer.weights.name, "weights");
        declare(layer.bias.name, "bias");
    }

    bool inputValid = checkTensor(network.input, 1, "input", network.name, errors);
    std::vector<bool> weightsValid;
    std::vector<bool> biasValid;
    std::vector<ActivationKind> activations;
    for (const LayerDecl& layer : network.layers) {
        weightsValid.push_back(checkTensor(layer.weights, 2, "weights", network.name, errors));
        biasValid.push_back(checkTensor(layer.bias, 1, "bias", network.name, errors));
        auto activation_it = activationMap.find(layer.activation);
        if (activation_it == activationMap.end()) {
            errors.push_back("unknown activation '" + layer.activation + "' in layer '" +
                             layer.name + "' of network '" + network.name +
                             "' (valid: " + validActivations() + ")");
            activations.push_back(ActivationKind::RELU);
        }
        else {
            activations.push_back(activation_it->second);
        }
    }

    if (network.layers.empty()) {
        errors.push_back("network '" + network.name + "' has no layers");
        return;
    }

    size_t n = network.layers.size();
    std::vector<std::optional<long>> boundaries(n + 1);
    for (size_t i = 0; i <= n; i++) {
        std::vector<Source> sources;
        if (i == 0 && inputValid && network.input.dims[0].has_value()) {
            sources.push_back({network.input.dims[0].value(),
                               "input '" + network.input.name + "' has size " +
                                   std::to_string(network.input.dims[0].value())});
        }
        if (i > 0) {
            const LayerDecl& layer = network.layers[i - 1];
            if (weightsValid[i - 1] && layer.weights.dims[0].has_value()) {
                sources.push_back({layer.weights.dims[0].value(),
                                   "layer '" + layer.name + "' outputs " +
                                       std::to_string(layer.weights.dims[0].value())});
            }
            if (biasValid[i - 1] && layer.bias.dims[0].has_value()) {
                sources.push_back({layer.bias.dims[0].value(),
                                   "bias '" + layer.bias.name + "' has size " +
                                       std::to_string(layer.bias.dims[0].value())});
            }
        }
        if (i < n) {
            const LayerDecl& next = network.layers[i];
            if (weightsValid[i] && next.weights.dims[1].has_value()) {
                sources.push_back({next.weights.dims[1].value(),
                                   "weights '" + next.weights.name + "' expects input size " +
                                       std::to_string(next.weights.dims[1].value())});
            }
        }
        if (sources.empty()) {
            if (i == 0) {
                errors.push_back("cannot infer input size of network '" + network.name + "'");
            }
            else {
                errors.push_back("cannot infer output size of layer '" +
                                 network.layers[i - 1].name + "' in network '" + network.name +
                                 "'");
            }
            continue;
        }
        bool consistent = true;
        for (size_t j = 1; j < sources.size(); j++) {
            if (sources[j].value != sources[0].value) {
                errors.push_back("size mismatch in network '" + network.name +
                                 "': " + sources[j].desc + " but " + sources[0].desc);
                consistent = false;
            }
        }
        if (consistent) {
            boundaries[i] = sources[0].value;
        }
    }

    if (errors.size() != errorsBefore) {
        return;
    }

    ResolvedNetwork resolved;
    resolved.name = network.name;
    resolved.inputName = network.input.name;
    resolved.inputSize = boundaries[0].value();
    for (size_t i = 0; i < n; i++) {
        const LayerDecl& layer = network.layers[i];
        resolved.layers.push_back(ResolvedLayer{layer.name,
                                                boundaries[i].value(),
                                                boundaries[i + 1].value(),
                                                activations[i],
                                                layer.weights.name,
                                                layer.bias.name});
    }
    result.networks.push_back(resolved);
}

SemaResult analyze(const Program& program)
{
    SemaResult result;
    std::unordered_set<std::string> networkNames;
    for (const NetworkDecl& network : program.networks) {
        size_t errorsBefore = result.errors.size();
        if (!networkNames.insert(network.name).second) {
            result.errors.push_back("duplicate network name '" + network.name + "'");
        }
        analyzeNetwork(network, errorsBefore, result);
    }
    return result;
}

void printResolved(const SemaResult& result)
{
    for (const ResolvedNetwork& network : result.networks) {
        std::cout << "network " << network.name << '\n';
        std::cout << "  input " << network.inputName << '[' << network.inputSize << "]\n";
        for (const ResolvedLayer& layer : network.layers) {
            std::cout << "  layer " << layer.name << ": [" << layer.inSize << " -> "
                      << layer.outSize << "] " << activationToString(layer.activation) << '\n';
        }
    }
}
