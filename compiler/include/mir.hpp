/* Author: Vrushang Anand */
/* Last Date Modified: Jul 20, 2026 */
/* Header file for the machine-level IR for a custom compiler for the ML
 * Inference Ecosystem.
 *
 * Tier 2: buffer-explicit, tiled, and fully linearized. Tile loops are
 * unrolled at compile time because the target has no program counter or
 * branch unit. */

#pragma once
#include "machine.hpp"
#include "tir.hpp"

#include <optional>
#include <string>
#include <vector>

enum class MirOp
{
    Zero,
    LoadT,
    StoreT,
    MatMul,
    Add,
    Sub,
    Relu,
    Softmax
};

enum class BufferKind
{
    Input,
    Weight,
    Accum
};

struct BufferRef
{
    BufferKind kind = BufferKind::Input;
    int index = 0;
};

struct MemRef
{
    std::string symbol;
    long offset = 0;
    long rows = 1;
    long cols = 0;
    long rowStride = 0;
    bool scratch = false;
};

struct MirInst
{
    MirOp op = MirOp::Zero;
    std::vector<BufferRef> buffers;
    std::optional<MemRef> mem;
};

struct MirFunction
{
    std::string name;
    std::vector<MirInst> body;
};

struct MirScratch
{
    std::string symbol;
    long elements = 0;
};

struct MirProgram
{
    std::vector<MirFunction> functions;
    std::vector<MirScratch> scratch;
};

MirProgram lowerToMir(const std::vector<TirFunction>& functions, const MachineModel& machine);
void printMir(const MirProgram& program, const MachineModel& machine);
