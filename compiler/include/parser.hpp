/* Author: Vrushang Anand */
/* Last Date Modified: Jul 19, 2026 */
/* Header file for the ONNX Parser for a custom compiler for the ML Inference
 * Ecosystem */

#pragma once
#include "ast.hpp"
#include "machine.hpp"

#include <cstdint>
#include <vector>

Program parse(const std::vector<std::uint8_t>& bytes, const MachineModel& machine);
