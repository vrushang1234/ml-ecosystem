/* Author: Vrushang Anand */
/* Last Date Modified: Jul 20, 2026 */
/* Source file for the machine-level IR for a custom compiler for the ML
 * Inference Ecosystem */

#include "mir.hpp"

#include <algorithm>
#include <iostream>
#include <stdexcept>
#include <string>
#include <unordered_map>
#include <vector>

namespace
{

    std::string opName(MirOp op)
    {
        switch (op) {
            case MirOp::Zero:
                return "zero";
            case MirOp::LoadT:
                return "loadT";
            case MirOp::StoreT:
                return "storeT";
            case MirOp::MatMul:
                return "matmul";
            case MirOp::Add:
                return "add";
            case MirOp::Sub:
                return "sub";
            case MirOp::Relu:
                return "relu";
            case MirOp::Softmax:
                return "softmax";
        }
        return "";
    }

    std::string bufferName(const BufferRef& buffer)
    {
        switch (buffer.kind) {
            case BufferKind::Input:
                return "IB" + std::to_string(buffer.index);
            case BufferKind::Weight:
                return "WB" + std::to_string(buffer.index);
            case BufferKind::Accum:
                return "ACC" + std::to_string(buffer.index);
        }
        return "";
    }

    BufferRef inputBuffer(int index)
    {
        return BufferRef{BufferKind::Input, index};
    }

    BufferRef weightBuffer(int index)
    {
        return BufferRef{BufferKind::Weight, index};
    }

    BufferRef accum(int index)
    {
        return BufferRef{BufferKind::Accum, index};
    }

    std::string memRefName(const MemRef& mem)
    {
        std::string out = (mem.scratch ? "%" : "@") + mem.symbol;
        out += '[' + std::to_string(mem.offset) + ' ';
        out += std::to_string(mem.rows) + 'x' + std::to_string(mem.cols);
        out += " stride " + std::to_string(mem.rowStride) + ']';
        return out;
    }

    class Emitter
    {
    public:
        Emitter(MirFunction& function, const MachineModel& machine)
            : function(function)
            , machine(machine)
        {
        }

        void zero(BufferRef dst)
        {
            function.body.push_back(MirInst{MirOp::Zero, {dst}, std::nullopt});
        }

        void load(BufferRef dst, MemRef mem)
        {
            function.body.push_back(MirInst{MirOp::LoadT, {dst}, std::move(mem)});
        }

        void store(MemRef mem, BufferRef src)
        {
            function.body.push_back(MirInst{MirOp::StoreT, {src}, std::move(mem)});
        }

        void binary(MirOp op, BufferRef dst, BufferRef a, BufferRef b)
        {
            function.body.push_back(MirInst{op, {dst, a, b}, std::nullopt});
        }

        void unary(MirOp op, BufferRef dst)
        {
            function.body.push_back(MirInst{op, {dst}, std::nullopt});
        }

        void memUnary(MirOp op, MemRef mem)
        {
            function.body.push_back(MirInst{op, {}, std::move(mem)});
        }

        void layer(const std::string& weightsSymbol,
                   const std::string& biasSymbol,
                   const std::string& srcSymbol,
                   const std::string& dstSymbol,
                   long outSize,
                   long inSize,
                   bool srcIsScratch,
                   bool fuseRelu)
        {
            long mTiles = ceilDiv(outSize, machine.tileM);
            long kTiles = ceilDiv(inSize, machine.tileK);

            for (long m = 0; m < mTiles; m++) {
                long rowBase = m * machine.tileM;
                long rows = std::min(machine.tileM, outSize - rowBase);

                int a = static_cast<int>(m % machine.accumBuffers);

                zero(accum(a));
                for (long k = 0; k < kTiles; k++) {
                    long colBase = k * machine.tileK;
                    long cols = std::min(machine.tileK, inSize - colBase);
                    int slot = static_cast<int>(k % machine.inputBuffers);
                    int wslot = static_cast<int>(k % machine.weightBuffers);

                    load(inputBuffer(slot),
                         MemRef{srcSymbol, colBase, 1, cols, inSize, srcIsScratch});
                    load(weightBuffer(wslot),
                         MemRef{weightsSymbol,
                                rowBase * inSize + colBase,
                                rows,
                                cols,
                                inSize,
                                false});
                    binary(MirOp::MatMul, accum(a), weightBuffer(wslot), inputBuffer(slot));
                }

                load(inputBuffer(0), MemRef{biasSymbol, rowBase, 1, rows, outSize, false});
                binary(MirOp::Add, accum(a), accum(a), inputBuffer(0));
                if (fuseRelu) {
                    unary(MirOp::Relu, accum(a));
                }
                store(MemRef{dstSymbol, rowBase, 1, rows, outSize, true}, accum(a));
            }
        }

    private:
        MirFunction& function;
        const MachineModel& machine;
    };

    bool isActivation(TirOp op)
    {
        return op == TirOp::Relu || op == TirOp::Softmax;
    }

} // namespace

MirProgram lowerToMir(const std::vector<TirFunction>& functions, const MachineModel& machine)
{
    MirProgram program;
    long largest = 0;

    for (const TirFunction& source : functions) {
        MirFunction function;
        function.name = source.name;
        Emitter emitter(function, machine);

        std::unordered_map<int, std::string> symbols;
        std::unordered_map<int, bool> isScratch;
        int scratchSlot = 0;

        for (size_t i = 0; i < source.body.size(); i++) {
            const TirInst& inst = source.body[i];
            if (inst.op == TirOp::Input || inst.op == TirOp::Const) {
                symbols[inst.result] = inst.symbol;
                isScratch[inst.result] = false;
                continue;
            }
            if (inst.op != TirOp::MatMul) {
                continue;
            }

            if (i + 1 >= source.body.size() || source.body[i + 1].op != TirOp::Add) {
                throw std::runtime_error("Lowering error: matmul in '" + source.name +
                                         "' is not followed by a bias add");
            }
            const TirInst& add = source.body[i + 1];

            TirOp activation = TirOp::Output;
            if (i + 2 < source.body.size() && isActivation(source.body[i + 2].op)) {
                activation = source.body[i + 2].op;
            }

            const std::string& weightsSymbol = symbols.at(inst.operands[0]);
            const std::string& srcSymbol = symbols.at(inst.operands[1]);
            const std::string& biasSymbol = symbols.at(add.operands[1]);

            long outSize = source.values[inst.result].dims[0];
            long inSize = source.values[inst.operands[1]].dims[0];

            bool srcIsScratch = isScratch.at(inst.operands[1]);
            std::string dstSymbol = "scratch" + std::to_string(scratchSlot);
            scratchSlot = (scratchSlot + 1) % 2;
            largest = std::max(largest, outSize);

            emitter.layer(weightsSymbol,
                          biasSymbol,
                          srcSymbol,
                          dstSymbol,
                          outSize,
                          inSize,
                          srcIsScratch,
                          activation == TirOp::Relu);

            if (activation == TirOp::Softmax) {

                emitter.memUnary(MirOp::Softmax, MemRef{dstSymbol, 0, 1, outSize, outSize, true});
            }

            for (int value : {inst.result, add.result}) {
                symbols[value] = dstSymbol;
                isScratch[value] = true;
            }
            if (activation != TirOp::Output) {
                symbols[source.body[i + 2].result] = dstSymbol;
                isScratch[source.body[i + 2].result] = true;
            }
        }

        program.functions.push_back(std::move(function));
    }

    program.scratch.push_back(MirScratch{"scratch0", largest});
    program.scratch.push_back(MirScratch{"scratch1", largest});
    return program;
}

void printMir(const MirProgram& program, const MachineModel& machine)
{
    std::cout << ".format " << machine.formatName() << '\n';
    std::cout << ".tile " << machine.tileM << "x" << machine.tileK << '\n';
    long bufferElems = machine.bufferElems > 0 ? machine.bufferElems : machine.tileK;
    std::cout << ".buffers input=" << machine.inputBuffers << " weight=" << machine.weightBuffers
              << " accum=" << machine.accumBuffers << " elems=" << bufferElems << '\n';

    std::cout << ".accum q" << 2 * machine.intBits << "." << 2 * machine.fracBits << " ("
              << machine.accumBits << "b), add<<" << machine.fracBits << ", storeT>>"
              << machine.fracBits << '\n';
    for (const MirScratch& scratch : program.scratch) {
        std::cout << ".scratch %" << scratch.symbol << " " << scratch.elements << '\n';
    }

    for (const MirFunction& function : program.functions) {
        std::cout << "\nfunc @" << function.name << " {\n";
        for (const MirInst& inst : function.body) {
            std::cout << "  " << opName(inst.op);
            bool first = true;
            if (inst.op == MirOp::StoreT && inst.mem.has_value()) {
                std::cout << ' ' << memRefName(inst.mem.value());
                first = false;
            }
            for (const BufferRef& buffer : inst.buffers) {
                std::cout << (first ? " " : ", ") << bufferName(buffer);
                first = false;
            }
            if (inst.op != MirOp::StoreT && inst.mem.has_value()) {
                std::cout << (first ? " " : ", ") << memRefName(inst.mem.value());
            }
            std::cout << '\n';
        }
        std::cout << "}\n";
    }
}
