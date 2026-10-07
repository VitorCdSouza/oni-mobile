// Helpers shared by the port's simulation code.
#pragma once
#include <cstdint>

#include "sim_types.h"

// From the original: assertion hook and its kill switch.
void AssertFunc(const char *tag, int line, const char *file);
extern bool gAssertsDisabled;

#define SIM_ASSERT(cond, line, file)                       \
    do {                                                   \
        if (!(cond) && !gAssertsDisabled) {                \
            AssertFunc("BREAKPT:", (line), (file));        \
        }                                                  \
    } while (0)

constexpr uint16_t kInvalidElement = 0xffff;
constexpr uint8_t kNoDisease = 0xff;

// Element state, low two bits of the state byte.
enum ElementState : uint8_t { kVacuum = 0, kGas = 1, kLiquid = 2, kSolid = 3 };
inline uint8_t StateOf(uint8_t state) { return state & 3; }

// Sim cells include a one-cell border around the game grid; this maps a sim cell to the game's
// cell index (negative or >= numGameCells for border cells).
inline int ToGameCell(const SimData &sim, int cell) {
    uint64_t c = static_cast<uint64_t>(static_cast<int64_t>(cell));
    uint64_t w = static_cast<uint64_t>(static_cast<int64_t>(sim.width));
    return (static_cast<int>(c / w) - 1) * (sim.width - 2) - 1 + static_cast<int>(c % w);
}

inline bool IsValidGameCell(const SimData &sim, int game_cell) {
    return game_cell >= 0 && game_cell < sim.numGameCells;
}

// Removes all disease from a cell (what the original does when the count drops below 1).
inline void ClearDisease(CellSOA &soa, int cell) {
    soa.diseaseIdx[cell] = kNoDisease;
    soa.diseaseCount[cell] = 0;
    soa.diseaseInfestationTickCount[cell] = 0;
    soa.diseaseAccumulatedError[cell] = 0.0f;
}
