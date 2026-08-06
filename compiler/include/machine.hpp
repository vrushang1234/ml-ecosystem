/* Author: Vrushang Anand */
/* Last Date Modified: Jul 20, 2026 */
/* Header file for the target machine model for a custom compiler for the ML
 * Inference Ecosystem */

#pragma once
#include <string>

struct MachineModel
{
    int fracBits = 16;
    int intBits = 16;
    int storageBits = 32;
    int accumBits = 64;
    long tileM = 4;
    long tileK = 4;
    long inputBuffers = 2;
    long weightBuffers = 2;
    long accumBuffers = 2;

    long bufferElems = 0;

    std::string formatName() const
    {
        return "q" + std::to_string(intBits) + "." + std::to_string(fracBits);
    }
};

void applyFormat(MachineModel& machine, const std::string& text);

inline long ceilDiv(long value, long divisor)
{
    return (value + divisor - 1) / divisor;
}
