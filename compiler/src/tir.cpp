/* Author: Vrushang Anand */
/* Last Date Modified: Jul 20, 2026 */
/* Source file for the tensor-level IR for a custom compiler for the ML
 * Inference Ecosystem */

#include "tir.hpp"

#include <iostream>
#include <string>
#include <vector>

namespace
{

    std::string opName(TirOp op)
    {
        switch (op) {
            case TirOp::Input:
                return "input";
            case TirOp::Const:
                return "const";
            case TirOp::MatMul:
                return "matmul";
            case TirOp::Add:
                return "add";
            case TirOp::Relu:
                return "relu";
            case TirOp::Softmax:
                return "softmax";
            case TirOp::Output:
                return "output";
        }
        return "";
    }

    std::string shapeName(const std::vector<long>& dims)
    {
        std::string out = "tensor<";
        for (size_t i = 0; i < dims.size(); i++) {
            if (i > 0) {
                out += 'x';
            }
            out += std::to_string(dims[i]);
        }
        return out + '>';
    }

    class Builder
    {
    public:
        explicit Builder(TirFunction& function)
            : function(function)
        {
        }

        int emit(TirOp op, std::vector<long> dims, std::vector<int> operands, std::string symbol)
        {
            int id = static_cast<int>(function.values.size());
            function.values.push_back(TirValue{id, std::move(dims)});
            function.body.push_back(TirInst{op, id, std::move(operands), std::move(symbol)});
            return id;
        }

    private:
        TirFunction& function;
    };

} // namespace

std::vector<TirFunction> lowerToTir(const SemaResult& result)
{
    std::vector<TirFunction> functions;
    for (const ResolvedNetwork& network : result.networks) {
        TirFunction function;
        function.name = network.name;
        Builder builder(function);

        int current = builder.emit(TirOp::Input, {network.inputSize}, {}, network.inputName);
        function.inputValue = current;

        for (const ResolvedLayer& layer : network.layers) {
            int weights =
                builder.emit(TirOp::Const, {layer.outSize, layer.inSize}, {}, layer.weightsName);
            int bias = builder.emit(TirOp::Const, {layer.outSize}, {}, layer.biasName);

            current = builder.emit(TirOp::MatMul, {layer.outSize}, {weights, current}, "");
            current = builder.emit(TirOp::Add, {layer.outSize}, {current, bias}, "");

            switch (layer.activation) {
                case ActivationKind::RELU:
                    current = builder.emit(TirOp::Relu, {layer.outSize}, {current}, "");
                    break;
                case ActivationKind::SOFTMAX:
                    current = builder.emit(TirOp::Softmax, {layer.outSize}, {current}, "");
                    break;
                case ActivationKind::LINEAR:
                    break;
            }
        }

        function.outputValue = current;
        functions.push_back(std::move(function));
    }
    return functions;
}

void printTir(const std::vector<TirFunction>& functions, const MachineModel& machine)
{
    std::cout << ".format " << machine.formatName() << '\n';
    for (const TirFunction& function : functions) {
        std::cout << "\nfunc @" << function.name << " {\n";
        for (const TirInst& inst : function.body) {
            std::cout << "  %" << inst.result << " = " << opName(inst.op);
            if (!inst.symbol.empty()) {
                std::cout << " @" << inst.symbol;
            }
            for (size_t i = 0; i < inst.operands.size(); i++) {
                std::cout << (i == 0 && inst.symbol.empty() ? " %" : ", %") << inst.operands[i];
            }
            std::cout << " : " << shapeName(function.values[inst.result].dims) << '\n';
        }
        std::cout << "  output %" << function.outputValue << " : "
                  << shapeName(function.values[function.outputValue].dims) << '\n';
        std::cout << "}\n";
    }
}
