// Helpers shared by the port's simulation code.
#pragma once
#include <cstdint>

#include "sim_types.h"

// From the original: assertion hook, its kill switch and the crash reporter message.
void AssertFunc(const char *tag, int line, const char *file);
extern bool gAssertsDisabled;
void KCrashReporterReportMessage(const char *file, int line, const char *format, ...);

// True for a non-zero, non-NaN float (how the original's "!= 0.0f" asserts compile).
inline bool IsNonZero(float x) { return x < 0.0f || x > 0.0f; }

// The disease table. Only what the port uses is declared: the first member points at one
// 0x110-byte record per disease, whose float at +4 weighs germs when two kinds meet.
struct Disease {
    const uint8_t *records;

    float Weight(uint8_t disease_idx) const {
        float w;
        __builtin_memcpy(&w, records + static_cast<size_t>(disease_idx) * 0x110 + 4, sizeof w);
        return w;
    }
    // Per-element tables inside a disease record (std::vector begin pointers).
    template <typename T>
    const T *Table(uint8_t disease_idx, size_t offset) const {
        const T *table;
        __builtin_memcpy(&table, records + static_cast<size_t>(disease_idx) * 0x110 + offset, sizeof table);
        return table;
    }
    void AddDiseaseToCell(CellSOA *soa, int cell, unsigned char disease_idx, int count);
    float GetDiffusionScale(const SimData &sim, int cell);
    uint8_t GetDiseaseIndex(uint32_t hash) const;
    void PostProcess(SimData &sim, SimEvents &events, int min_x, int max_x, int min_y, int max_y);
    void UpdateCells(float dt, SimData &sim, SimEvents &events, int a, int b);
};
extern Disease *gDisease;

#define SIM_ASSERT(cond, line, file)                       \
    do {                                                   \
        if (!(cond) && !gAssertsDisabled) {                \
            AssertFunc("BREAKPT:", (line), (file));        \
        }                                                  \
    } while (0)

// The original's small vector templates (their names appear in mangled signatures).
template <typename T>
struct Vector2 {
    T x, y;
};
template <typename T>
struct Vector4 {
    T x, y, z, w;
};

// SimBase drives the frame: only the member functions the port defines are declared; its data
// is reached through offsets until its layout is mapped.
struct SimBase {
    void UpdatedCellsToCells(SimData &sim, Vector2<int> min, Vector2<int> max);
    void CopyUpdatedCellsToCells(SimData &sim);
    void ConsolidateEvents(SimData &sim);
    void UpdateData(SimData &sim);
};

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

bool DisplaceGas(SimData &sim, SimEvents &events, CellSOA *soa, int cell, uint16_t element);
void AddMassAndUpdateTemperature(CellSOA *src, CellSOA *dst, int cell, float mass, float temperature,
                                 uint8_t disease_idx, int disease_count);
void Evaporate(int cell, SimData &sim, SimEvents &events);
void DoDisplacement(SimData &sim, SimEvents &events, CellSOA *soa, int from, int to);
bool DisplaceLiquidSimple(SimData &sim, SimEvents &events, int cell, uint16_t element, int num_candidates,
                          const int *candidates);
bool DisplaceLiquid(SimData &sim, SimEvents &events, CellSOA *soa, int cell, uint16_t element);

// minss semantics: the second operand wins unless the first is strictly smaller (matters for NaN).
inline float MinSS(float a, float b) { return a < b ? a : b; }
inline float MaxSS(float a, float b) { return a > b ? a : b; }
inline double MinSD(double a, double b) { return a < b ? a : b; }
inline double MaxSD(double a, double b) { return a > b ? a : b; }

bool DoDensityDisplacement(SimData &sim, SimEvents &events, int cell, const ElementPostProcessData &elem,
                           float factor);
bool DoPressureBreak(SimData &sim, SimEvents &events, int cell, float pressure_ratio);
uint64_t DoPartialMelt(SimData &sim, SimEvents &events, int cell);
void DoUnstableCheckBasic(SimData &sim, SimEvents &events, CellAccessor &cell);
void DoUnstableCheckWithDiagonals(SimData &sim, SimEvents &events, CellAccessor &cell);
char DoSublimation(SimData &sim, SimEvents &events, int cell, const ElementPostProcessData &self);
uint64_t DoPartialHeatTransition(SimData &sim, SimEvents &events, int cell);
void HeadlessUnstableFallSimpleSwap(SimData &sim, SimEvents &events, CellAccessor &cell);

constexpr int32_t kSimHashVoid = -1456075980;  // SimHashes.Void
uint8_t IsLiquidPermeable(const CellSOA *soa, unsigned long cell);
void UpdateTemperature(SimData &sim, SimEvents &events, int a, int b);
void UpdateTemperatureForBackwallExchange(SimData &sim, SimEvents &events, int cell);
int DoStateTransition(SimData &sim, SimEvents &events, int cell, const ElementTemperatureData &elem);
float UpdatePressure(SimData &sim, SimEvents &events, const CellSOA *old, CellSOA *upd, int cell, uint16_t element,
                     signed char state, float flow, int other);
float DoGasPressureDisplacement(uint16_t element, int cell, int neighbour, int beyond, SimData &sim);
float DoLiquidPressureDisplacement(uint16_t element, int cell, int neighbour, int beyond, SimData &sim);
void UpdateLiquid(SimData &sim, SimEvents &events, int cell);
void PostProcessCell(SimData &sim, SimEvents &events, int cell);

// Two germ populations in one cell: what survives (Disease::CalculateFinalDiseaseCount).
struct DiseaseCount {
    uint8_t idx;
    int count;
};
DiseaseCount CombineDisease(const Disease &disease, uint8_t idx_a, int count_a, uint8_t idx_b, int count_b);
void GetReachableCells(SimData &sim, int x, int y, int radius, std::vector<int> &visited,
                       std::queue<FloodFillInfo> &queue, std::vector<int> &reachable, bool skip_solid);
