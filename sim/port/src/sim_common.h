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

inline CellAccessor Accessor(CellSOA *soa, int cell) {
    CellAccessor a{};
    a.soa = soa;
    a.cell = cell;
    return a;
}

// Removes all disease from a cell (what the original does when the count drops below 1).
inline void ClearDisease(CellSOA &soa, int cell) {
    soa.diseaseIdx[cell] = kNoDisease;
    soa.diseaseCount[cell] = 0;
    soa.diseaseInfestationTickCount[cell] = 0;
    soa.diseaseAccumulatedError[cell] = 0.0f;
}

// The simulation's random number generator (MSVC rand()), uniform in [0, 1].
inline float SimRandom01(SimData &sim) {
    sim.rngState = sim.rngState * 0x343fdu + 0x269ec3u;
    // 0x1.0002p-15f is the original's float closest to 1 / 32767.
    return static_cast<float>(static_cast<int>((sim.rngState >> 16) & 0x7fff)) * 0x1.0002p-15f;
}

inline void PushSubstanceChange(SimEvents &events, int game_cell) {
    SubstanceChangeInfo info;
    info.cellIdx = game_cell;
    info.oldElemIdx = kInvalidElement;
    info.newElemIdx = kInvalidElement;
    events.substanceChanges.push_back(info);
}

inline void PushSpawnFX(SimEvents &events, int game_cell, int fx_hash, float rotation) {
    SpawnFXInfo info;
    info.cellIdx = game_cell;
    info.fxHash = fx_hash;
    info.rotation = rotation;
    events.spawnFX.push_back(info);
}

// Not ported yet: resolved to the original in hybrid builds.
bool DisplaceGas(SimData &sim, SimEvents &events, CellSOA *soa, int cell, uint16_t element);
void AddMassAndUpdateTemperature(CellSOA *dst, CellSOA *src, int cell, float mass, float temperature,
                                 uint8_t disease_idx, int disease_count);
void Evaporate(int cell, SimData &sim, SimEvents &events);
bool DisplaceLiquid(SimData &sim, SimEvents &events, CellSOA *soa, int cell, uint16_t element);

// minss semantics: the second operand wins unless the first is strictly smaller (matters for NaN).
inline float MinSS(float a, float b) { return a < b ? a : b; }

bool DoDensityDisplacement(SimData &sim, SimEvents &events, int cell, const ElementPostProcessData &elem,
                           float factor);
bool DoPressureBreak(SimData &sim, SimEvents &events, int cell, float pressure_ratio);
bool DoPartialMelt(SimData &sim, SimEvents &events, int cell);
void DoUnstableCheckBasic(SimData &sim, SimEvents &events, CellAccessor &cell);
void DoUnstableCheckWithDiagonals(SimData &sim, SimEvents &events, CellAccessor &cell);
char DoSublimation(SimData &sim, SimEvents &events, int cell, const ElementPostProcessData &self);
uint64_t DoPartialHeatTransition(SimData &sim, SimEvents &events, int cell);
