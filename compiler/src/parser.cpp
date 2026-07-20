/* Author: Vrushang Anand */
/* Last Date Modified: Jul 19, 2026 */
/* Source file for the ONNX Parser for a custom compiler for the ML Inference
 * Ecosystem.
 */

#include "parser.hpp"

#include <cstdint>
#include <optional>
#include <stdexcept>
#include <string>
#include <unordered_map>
#include <unordered_set>
#include <utility>
#include <vector>

namespace
{

    class Reader
    {
    public:
        Reader(const std::uint8_t* begin, const std::uint8_t* end)
            : p(begin)
            , end(end)
        {
        }

        bool done() const
        {
            return p == end;
        }

        std::uint64_t readVarint()
        {
            std::uint64_t value = 0;
            int shift = 0;
            while (true) {
                if (p == end) {
                    throw std::runtime_error("Parse error: truncated varint in ONNX file");
                }
                std::uint8_t byte = *p++;
                if (shift < 64) {
                    value |= static_cast<std::uint64_t>(byte & 0x7f) << shift;
                }
                if ((byte & 0x80) == 0) {
                    return value;
                }
                shift += 7;
                if (shift > 70) {
                    throw std::runtime_error("Parse error: varint too long in ONNX file");
                }
            }
        }

        std::pair<std::uint32_t, std::uint32_t> readKey()
        {
            std::uint64_t key = readVarint();
            return {static_cast<std::uint32_t>(key >> 3), static_cast<std::uint32_t>(key & 7)};
        }

        Reader readLengthDelimited()
        {
            std::uint64_t length = readVarint();
            if (length > static_cast<std::uint64_t>(end - p)) {
                throw std::runtime_error("Parse error: truncated field in ONNX file");
            }
            Reader sub(p, p + length);
            p += length;
            return sub;
        }

        std::string readString()
        {
            Reader sub = readLengthDelimited();
            return std::string(reinterpret_cast<const char*>(sub.p), sub.end - sub.p);
        }

        void skip(std::uint32_t wireType)
        {
            switch (wireType) {
                case 0:
                    readVarint();
                    break;
                case 1:
                    skipBytes(8);
                    break;
                case 2:
                    readLengthDelimited();
                    break;
                case 5:
                    skipBytes(4);
                    break;
                default:
                    throw std::runtime_error("Parse error: unsupported wire type " +
                                             std::to_string(wireType) + " in ONNX file");
            }
        }

    private:
        const std::uint8_t* p;
        const std::uint8_t* end;

        void skipBytes(std::uint64_t count)
        {
            if (count > static_cast<std::uint64_t>(end - p)) {
                throw std::runtime_error("Parse error: truncated field in ONNX file");
            }
            p += count;
        }
    };

    struct Tensor
    {
        std::string name;
        std::vector<long> dims;
    };

    struct ValueInfo
    {
        std::string name;
        std::vector<std::optional<long>> dims;
    };

    struct Attribute
    {
        std::string name;
        long i = 0;
    };

    struct Node
    {
        std::string name;
        std::string opType;
        std::vector<std::string> inputs;
        std::vector<std::string> outputs;
        std::vector<Attribute> attributes;
    };

    struct Graph
    {
        std::string name;
        std::vector<Node> nodes;
        std::vector<Tensor> initializers;
        std::vector<ValueInfo> inputs;
    };

    std::vector<std::optional<long>> decodeShape(Reader reader)
    {
        std::vector<std::optional<long>> dims;
        while (!reader.done()) {
            auto [field, wireType] = reader.readKey();
            if (field == 1 && wireType == 2) {
                Reader dim = reader.readLengthDelimited();
                std::optional<long> value;
                while (!dim.done()) {
                    auto [dimField, dimWireType] = dim.readKey();
                    if (dimField == 1 && dimWireType == 0) {
                        value = static_cast<long>(static_cast<std::int64_t>(dim.readVarint()));
                    }
                    else {
                        dim.skip(dimWireType);
                    }
                }
                dims.push_back(value);
            }
            else {
                reader.skip(wireType);
            }
        }
        return dims;
    }

    ValueInfo decodeValueInfo(Reader reader)
    {
        ValueInfo info;
        while (!reader.done()) {
            auto [field, wireType] = reader.readKey();
            if (field == 1 && wireType == 2) {
                info.name = reader.readString();
            }
            else if (field == 2 && wireType == 2) {
                Reader type = reader.readLengthDelimited();
                while (!type.done()) {
                    auto [typeField, typeWireType] = type.readKey();
                    if (typeField == 1 && typeWireType == 2) {
                        Reader tensorType = type.readLengthDelimited();
                        while (!tensorType.done()) {
                            auto [ttField, ttWireType] = tensorType.readKey();
                            if (ttField == 2 && ttWireType == 2) {
                                info.dims = decodeShape(tensorType.readLengthDelimited());
                            }
                            else {
                                tensorType.skip(ttWireType);
                            }
                        }
                    }
                    else {
                        type.skip(typeWireType);
                    }
                }
            }
            else {
                reader.skip(wireType);
            }
        }
        return info;
    }

    Tensor decodeTensor(Reader reader)
    {
        Tensor tensor;
        while (!reader.done()) {
            auto [field, wireType] = reader.readKey();
            if (field == 1 && wireType == 0) {
                tensor.dims.push_back(
                    static_cast<long>(static_cast<std::int64_t>(reader.readVarint())));
            }
            else if (field == 1 && wireType == 2) {
                Reader packed = reader.readLengthDelimited();
                while (!packed.done()) {
                    tensor.dims.push_back(
                        static_cast<long>(static_cast<std::int64_t>(packed.readVarint())));
                }
            }
            else if (field == 8 && wireType == 2) {
                tensor.name = reader.readString();
            }
            else {
                reader.skip(wireType);
            }
        }
        return tensor;
    }

    Attribute decodeAttribute(Reader reader)
    {
        Attribute attribute;
        while (!reader.done()) {
            auto [field, wireType] = reader.readKey();
            if (field == 1 && wireType == 2) {
                attribute.name = reader.readString();
            }
            else if (field == 3 && wireType == 0) {
                attribute.i = static_cast<long>(static_cast<std::int64_t>(reader.readVarint()));
            }
            else {
                reader.skip(wireType);
            }
        }
        return attribute;
    }

    Node decodeNode(Reader reader)
    {
        Node node;
        while (!reader.done()) {
            auto [field, wireType] = reader.readKey();
            if (field == 1 && wireType == 2) {
                node.inputs.push_back(reader.readString());
            }
            else if (field == 2 && wireType == 2) {
                node.outputs.push_back(reader.readString());
            }
            else if (field == 3 && wireType == 2) {
                node.name = reader.readString();
            }
            else if (field == 4 && wireType == 2) {
                node.opType = reader.readString();
            }
            else if (field == 5 && wireType == 2) {
                node.attributes.push_back(decodeAttribute(reader.readLengthDelimited()));
            }
            else {
                reader.skip(wireType);
            }
        }
        return node;
    }

    Graph decodeGraph(Reader reader)
    {
        Graph graph;
        while (!reader.done()) {
            auto [field, wireType] = reader.readKey();
            if (field == 1 && wireType == 2) {
                graph.nodes.push_back(decodeNode(reader.readLengthDelimited()));
            }
            else if (field == 2 && wireType == 2) {
                graph.name = reader.readString();
            }
            else if (field == 5 && wireType == 2) {
                graph.initializers.push_back(decodeTensor(reader.readLengthDelimited()));
            }
            else if (field == 11 && wireType == 2) {
                graph.inputs.push_back(decodeValueInfo(reader.readLengthDelimited()));
            }
            else {
                reader.skip(wireType);
            }
        }
        return graph;
    }

    Graph decodeModel(Reader reader)
    {
        std::optional<Graph> graph;
        while (!reader.done()) {
            auto [field, wireType] = reader.readKey();
            if (field == 7 && wireType == 2) {
                graph = decodeGraph(reader.readLengthDelimited());
            }
            else {
                reader.skip(wireType);
            }
        }
        if (!graph.has_value()) {
            throw std::runtime_error("Parse error: ONNX model has no graph");
        }
        return std::move(graph).value();
    }

    const std::unordered_set<std::string> activationOps{
        "Relu", "Softmax", "Sigmoid", "Tanh", "Identity", "LeakyRelu", "Elu", "Gelu"};

    std::string toLower(const std::string& s)
    {
        std::string out = s;
        for (char& c : out) {
            if (c >= 'A' && c <= 'Z') {
                c = static_cast<char>(c - 'A' + 'a');
            }
        }
        return out;
    }

    long getIntAttribute(const Node& node, const std::string& name, long fallback)
    {
        for (const Attribute& attribute : node.attributes) {
            if (attribute.name == name) {
                return attribute.i;
            }
        }
        return fallback;
    }

    TensorDecl toTensorDecl(const Tensor& tensor)
    {
        TensorDecl decl;
        decl.name = tensor.name;
        for (long dim : tensor.dims) {
            decl.dims.push_back(dim);
        }
        return decl;
    }

    NetworkDecl lowerGraph(const Graph& graph)
    {
        NetworkDecl network;
        network.name = graph.name.empty() ? "model" : graph.name;

        std::unordered_map<std::string, const Tensor*> initializers;
        for (const Tensor& tensor : graph.initializers) {
            initializers.emplace(tensor.name, &tensor);
        }

        const ValueInfo* input = nullptr;
        for (const ValueInfo& candidate : graph.inputs) {
            if (initializers.find(candidate.name) == initializers.end()) {
                if (input != nullptr) {
                    throw std::runtime_error("Parse error: ONNX graph has more than one input");
                }
                input = &candidate;
            }
        }
        if (input == nullptr) {
            throw std::runtime_error("Parse error: ONNX graph has no input");
        }

        network.input.name = input->name;
        if (input->dims.size() == 2) {
            network.input.dims.push_back(input->dims[1]);
        }
        else {
            network.input.dims = input->dims;
        }

        size_t i = 0;
        while (i < graph.nodes.size()) {
            const Node& node = graph.nodes[i];
            if (node.opType != "Gemm") {
                throw std::runtime_error("Parse error: unsupported op '" + node.opType +
                                         "' in ONNX graph (expected Gemm)");
            }
            if (node.inputs.size() < 3) {
                throw std::runtime_error("Parse error: Gemm node '" + node.name +
                                         "' must have weights and bias inputs");
            }
            if (getIntAttribute(node, "transA", 0) != 0) {
                throw std::runtime_error("Parse error: Gemm node '" + node.name +
                                         "' with transA is unsupported");
            }

            LayerDecl layer;
            layer.name =
                node.name.empty() ? "l" + std::to_string(network.layers.size() + 1) : node.name;

            auto weights_it = initializers.find(node.inputs[1]);
            if (weights_it == initializers.end()) {
                throw std::runtime_error("Parse error: weights '" + node.inputs[1] + "' of Gemm '" +
                                         layer.name + "' must be an initializer");
            }
            layer.weights = toTensorDecl(*weights_it->second);
            if (getIntAttribute(node, "transB", 0) == 0 && layer.weights.dims.size() == 2) {
                std::swap(layer.weights.dims[0], layer.weights.dims[1]);
            }

            auto bias_it = initializers.find(node.inputs[2]);
            if (bias_it == initializers.end()) {
                throw std::runtime_error("Parse error: bias '" + node.inputs[2] + "' of Gemm '" +
                                         layer.name + "' must be an initializer");
            }
            layer.bias = toTensorDecl(*bias_it->second);

            i++;
            if (i < graph.nodes.size() &&
                activationOps.find(graph.nodes[i].opType) != activationOps.end()) {
                layer.activation = toLower(graph.nodes[i].opType);
                i++;
            }
            else {
                layer.activation = "linear";
            }

            network.layers.push_back(std::move(layer));
        }
        return network;
    }

} // namespace

Program parse(const std::vector<std::uint8_t>& bytes)
{
    Reader reader(bytes.data(), bytes.data() + bytes.size());
    Graph graph = decodeModel(reader);
    Program program;
    program.networks.push_back(lowerGraph(graph));
    return program;
}
