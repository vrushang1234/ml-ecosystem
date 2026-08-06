/* Author: Vrushang Anand */
/* Last Date Modified: Jul 20, 2026 */
/* Header file for the tensor-level IR for a custom compiler for the ML
 * Inference Ecosystem.
 *
 * Tier 1: SSA over whole tensors. No buffers, no tiles, no memory. Layers from
 * the semantic analyzer are expanded here into primitive ops. */

#pragma once
#include "machine.hpp"
#include "sema.hpp"

#include <string>
#include <vector>

enum class TirOp
{
    Input,
    Const,
    MatMul,
    Add,
    Relu,
    Softmax,
    Output
};

struct TirValue
{
    int id = 0;
    std::vector<long> dims;
};

struct TirInst
{
    TirOp op = TirOp::Input;
    int result = 0;
    std::vector<int> operands;
    std::string symbol;
};

struct TirFunction
{
    std::string name;
    std::vector<TirValue> values;
    std::vector<TirInst> body;
    int inputValue = 0;
    int outputValue = 0;
};

std::vector<TirFunction> lowerToTir(const SemaResult& result);
void printTir(const std::vector<TirFunction>& functions, const MachineModel& machine);
