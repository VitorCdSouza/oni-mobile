// Helpers shared by the port's simulation code.
#pragma once
#include <cstddef>
#include <cstdint>
#include <string>
#include <vector>

#include "sim_types.h"

// From the original: assertion hook, its kill switch and the crash reporter message.
void AssertFunc(const char *tag, int line, const char *file);
extern bool gAssertsDisabled;
void KCrashReporterReportMessage(const char *file, int line, const char *format, ...);

// True for a non-zero, non-NaN float (how the original's "!= 0.0f" asserts compile).
inline bool IsNonZero(float x) { return x < 0.0f || x > 0.0f; }

// Germ types, sent by the game at load (one DiseaseInfo per germ, Klei.AI.Disease in C#).
struct DiseaseInfo {
    // C# Klei.AI.Disease.RangeInfo: minViable, minGrowth, maxGrowth, maxViable.
    struct RangeInfo {
        float v[4];
    };
    // C# ElemGrowthInfo, one vector entry per element.
    struct ElemGrowthInfo {
        std::vector<float> underPopulationDeathRate;
        std::vector<float> populationHalfLife;
        std::vector<float> overPopulationHalfLife;
        std::vector<float> diffusionScale;
        std::vector<float> minCountPerKG;
        std::vector<float> maxCountPerKG;
        std::vector<int32_t> minDiffusionCount;
        std::vector<uint8_t> minDiffusionInfestationTickCount;
        void Initialize(int element_count);
    };
    uint32_t hash;                   // name hash
    float strength;                  // weighs germs when two kinds meet in a cell
    RangeInfo temperatureRange;
    RangeInfo temperatureHalfLives;
    RangeInfo pressureRange;
    RangeInfo pressureHalfLives;
    ElemGrowthInfo growth;
    float radiationKillRate;
};
static_assert(sizeof(DiseaseInfo) == 0x110, "DiseaseInfo size");
static_assert(offsetof(DiseaseInfo, growth) == 0x48, "DiseaseInfo::growth offset");
static_assert(offsetof(DiseaseInfo, radiationKillRate) == 0x108, "DiseaseInfo::radiationKillRate offset");

struct StreamReader;

struct Disease {
    std::vector<DiseaseInfo> infos;
    std::vector<std::string> names;

    Disease(StreamReader &reader);

    const DiseaseInfo &Info(uint8_t disease_idx) const { return infos[disease_idx]; }
    float Weight(uint8_t disease_idx) const { return Info(disease_idx).strength; }
    void AddDiseaseToCell(CellSOA *soa, int cell, unsigned char disease_idx, int count);
    float GetDiffusionScale(const SimData &sim, int cell);
    uint8_t GetDiseaseIndex(uint32_t hash) const;
    void PostProcess(SimData &sim, SimEvents &events, int min_x, int max_x, int min_y, int max_y);
    void UpdateCells(float dt, SimData &sim, SimEvents &events, int a, int b);
};
static_assert(sizeof(Disease) == 0x30, "Disease size");
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
struct GameData;
struct SimBase {
    void UpdatedCellsToCells(SimData &sim, Vector2<int> min, Vector2<int> max);
    void CopyUpdatedCellsToCells(SimData &sim);
    void ConsolidateEvents(SimData &sim);
    void UpdateData(SimData &sim);
    void Step(SimData &sim, GameData &game, const GameData &previous);
    void CopySimDataToGame(SimData &sim, GameData &game, const GameData &previous, int frames);
};

// Row copy of one per-cell array from the bordered sim grid into the game's grid (the
// UpdateCellSOATask layout, for a single array).
template <typename T>
struct UpdateCellTask {
    void *vtable;
    T *src;
    T *dst;
    int32_t numRows;
    int32_t firstRow;
    int32_t srcStride;
    int32_t dstFirstRow;
    int32_t rowLength;
    void InternalDoTask();
};
static_assert(sizeof(UpdateCellTask<Vector4<float>>) == 0x30, "UpdateCellTask size");
template <>
void UpdateCellTask<Vector4<float>>::InternalDoTask();

// The interface every component exposes at offset 0x80; SimData::components points there.
struct SimComponent {
    virtual void Update(float dt, SimData &sim, const Region &region) = 0;
    virtual void UpdateDataListOnly(SimData &sim) = 0;
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
uint16_t GetElementIndex(uint32_t hash);

// Two germ populations in one cell: what survives (Disease::CalculateFinalDiseaseCount).
struct DiseaseCount {
    uint8_t idx;
    int count;
};
DiseaseCount CombineDisease(const Disease &disease, uint8_t idx_a, int count_a, uint8_t idx_b, int count_b);
void GetReachableCells(SimData &sim, int x, int y, int radius, std::vector<int> &visited,
                       std::queue<FloodFillInfo> &queue, std::vector<int> &reachable, bool skip_solid);
