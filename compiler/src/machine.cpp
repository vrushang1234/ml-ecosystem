/* Author: Vrushang Anand */
/* Last Date Modified: Jul 20, 2026 */
/* Source file for the target machine model for a custom compiler for the ML
 * Inference Ecosystem */

#include "machine.hpp"

#include <stdexcept>
#include <string>

void applyFormat(MachineModel& machine, const std::string& text)
{
    if (text.size() < 4 || text[0] != 'q') {
        throw std::runtime_error("Error: bad fixed-point format '" + text + "' (expected qI.F)");
    }
    size_t dot = text.find('.');
    if (dot == std::string::npos || dot == 1 || dot + 1 == text.size()) {
        throw std::runtime_error("Error: bad fixed-point format '" + text + "' (expected qI.F)");
    }
    int intBits = 0;
    int fracBits = 0;
    try {
        intBits = std::stoi(text.substr(1, dot - 1));
        fracBits = std::stoi(text.substr(dot + 1));
    }
    catch (const std::exception&) {
        throw std::runtime_error("Error: bad fixed-point format '" + text + "' (expected qI.F)");
    }
    if (intBits < 1 || fracBits < 1) {
        throw std::runtime_error("Error: fixed-point format '" + text +
                                 "' must have at least one integer and one fractional bit");
    }
    if (intBits + fracBits != machine.storageBits) {
        throw std::runtime_error("Error: fixed-point format '" + text + "' is " +
                                 std::to_string(intBits + fracBits) + " bits but storage is " +
                                 std::to_string(machine.storageBits) + " bits");
    }
    machine.intBits = intBits;
    machine.fracBits = fracBits;
}
