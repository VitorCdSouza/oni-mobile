// Standalone build: SimData's lifetime, the world setup messages, and Load / Save.
#include <cfloat>
#include <cstring>
#include <ctime>
#include <new>

#include "../src/binary_buffer.h"
#include "../src/sim_common.h"

namespace {

constexpr uint32_t kUnobtaniumLikeHash = 0xa9360b34;  // unk_0x1a: never falls, never replaced
constexpr uint32_t kVacuumHash = 0x2d39bf75;
constexpr uint32_t kBoundaryHash = 0x6d95058c;  // unk_0x1e: the map's border element
constexpr uint32_t kSettleHash = 0x3fe0146e;    // cells whose temperature settles to the neighbours'

// The SimComponent interface every component exposes at +0x80: UpdateComponents and
// UpdateComponentsDataListOnly call it. One adapter per component type, placed where the
// original keeps the interface vtable pointer.
template <typename C>
struct UpdateOnly final : SimComponent {
    C *Owner() { return reinterpret_cast<C *>(reinterpret_cast<char *>(this) - 0x80); }
    void Update(float dt, SimData &sim, const Region &region) override { Owner()->Update(dt, sim, region); }
    void UpdateDataListOnly(SimData &) override {}
};
template <typename C>
struct UpdateAndDataList final : SimComponent {
    C *Owner() { return reinterpret_cast<C *>(reinterpret_cast<char *>(this) - 0x80); }
    void Update(float dt, SimData &sim, const Region &region) override { Owner()->Update(dt, sim, region); }
    void UpdateDataListOnly(SimData &sim) override { Owner()->UpdateDataListOnly(sim); }
};
struct DiseaseConsumerInterface final : SimComponent {
    void Update(float, SimData &, const Region &) override {}
    void UpdateDataListOnly(SimData &) override {}
};
static_assert(sizeof(UpdateOnly<ElementConsumer>) == sizeof(void *), "interface is a single vtable pointer");

template <typename Interface>
void *PlaceInterface(void *&slot) {
    return new (&slot) Interface();
}


// The map border: a fixed element with 9999 kg, no germs, no backwall.
void SetBorderCell(SimData &sim, int cell, uint16_t element) {
    for (CellSOA *c : {sim.cells, sim.updatedCells}) {
        c->elementIdx[cell] = element;
        c->mass[cell] = 9999.0f;
        c->temperature[cell] = 0.0f;
        c->diseaseCount[cell] = 0;
        c->diseaseIdx[cell] = kNoDisease;
    }
    BackwallSOA &bw = *sim.backwall;
    bw.elementIdx[cell] = bw.noneElementIdx;
    bw.mass[cell] = 0.0f;
    bw.temperature[cell] = 0.0f;
}

// A cell emptied to vacuum in both grids (radiation too) and in the backwall.
void ClearToVacuum(SimData &sim, int cell, bool backwall) {
    for (CellSOA *c : {sim.cells, sim.updatedCells}) {
        c->elementIdx[cell] = sim.vacuumElementIdx;
        c->mass[cell] = 0.0f;
        c->temperature[cell] = 0.0f;
        c->diseaseCount[cell] = 0;
        c->diseaseIdx[cell] = kNoDisease;
        c->unk_0xf0[cell] = 0.0f;
    }
    if (backwall) {
        BackwallSOA &bw = *sim.backwall;
        bw.elementIdx[cell] = bw.noneElementIdx;
        bw.mass[cell] = 0.0f;
        bw.temperature[cell] = 0.0f;
    }
}

}  // namespace

// CellSOA::CellSOA: no element (0xffff), full insulation byte (0xff), strength 0x84, no germs.
CellSOA *NewCellSOA(size_t n) {
    CellSOA *c = new CellSOA();
    c->elementIdx.assign(n, kInvalidElement);
    c->temperature.assign(n, 0.0f);
    c->mass.assign(n, 0.0f);
    c->properties.assign(n, 0);
    c->insulation.assign(n, 0xff);
    c->strengthInfo.assign(n, 0x84);
    c->diseaseIdx.assign(n, kNoDisease);
    c->diseaseCount.assign(n, 0);
    c->diseaseInfestationTickCount.assign(n, 0);
    c->diseaseAccumulatedError.assign(n, 0.0f);
    c->unk_0xf0.assign(n, 0.0f);
    return c;
}

SimData::SimData(int w, int h, unsigned int seed, bool radiation_enabled, bool is_headless)
    : width(w), height(h), numGameCells((w - 2) * (h - 2)), rngState(seed), flowDirection(-1), gasDirection(-1),
      savedOptions(0), pad_0x19{}, unk_0x1a(0), vacuumElementIdx(0), unk_0x1e(0), cells(nullptr),
      updatedCells(nullptr), backwall(nullptr), unk_0x38(nullptr), unk_0x58(nullptr), unk_0x60(nullptr),
      unk_0x68(nullptr), unk_0x88(nullptr), displacementDirection(0), pad_0x92{}, events(nullptr), unk_0xa0(0.001f),
      unk_0xa4(0.001f), unk_0xa8(0), pad_0xa9{}, unk_0x718(false), headless(is_headless),
      radiationEnabled(radiation_enabled), pad_0x71b{}, unk_0x71c(1.1f), unk_0x720(2000.0f), unk_0x724(0.3f),
      unk_0x728(0.7f), unk_0x72c(0.8f) {
    events = new SimEvents();
    unk_0x1a = GetElementIndex(kUnobtaniumLikeHash);
    vacuumElementIdx = GetElementIndex(kVacuumHash);
    unk_0x1e = GetElementIndex(kBoundaryHash);
    size_t n = static_cast<size_t>(static_cast<long>(w * h));
    unk_0x60 = new Vector4f[n]();
    cells = NewCellSOA(n);
    updatedCells = NewCellSOA(n);
    backwall = new BackwallSOA();
    backwall->noneElementIdx = vacuumElementIdx;
    backwall->elementIdx.assign(n, vacuumElementIdx);
    backwall->mass.assign(n, 0.0f);
    backwall->temperature.assign(n, 0.0f);
    unk_0x68 = new uint8_t[n]();
    unk_0x58 = new float[n]();
    for (size_t i = 0; i < n; i++) {
        unk_0x68[i] |= 0x1f;
    }
    unk_0x88 = new uint8_t[static_cast<size_t>(static_cast<long>(numGameCells))]();
    components.push_back(PlaceInterface<UpdateOnly<ElementConsumer>>(elementConsumer.componentVtable));
    components.push_back(PlaceInterface<UpdateAndDataList<ElementEmitter>>(elementEmitter.componentVtable));
    components.push_back(PlaceInterface<UpdateOnly<RadiationEmitter>>(radiationEmitter.componentVtable));
    components.push_back(PlaceInterface<UpdateAndDataList<ElementChunk>>(elementChunk.componentVtable));
    components.push_back(PlaceInterface<UpdateAndDataList<BuildingHeatExchange>>(buildingHeatExchange.componentVtable));
    components.push_back(
        PlaceInterface<UpdateOnly<BuildingToBuildingHeatExchange>>(buildingToBuildingHeatExchange.componentVtable));
    components.push_back(PlaceInterface<UpdateAndDataList<DiseaseEmitter>>(diseaseEmitter.componentVtable));
    components.push_back(PlaceInterface<DiseaseConsumerInterface>(diseaseConsumer.componentVtable));
}

SimData::~SimData() {
    delete cells;
    delete updatedCells;
    delete backwall;
    delete events;
    delete[] unk_0x38;
    delete[] unk_0x58;
    delete[] unk_0x60;
    delete[] unk_0x68;
    delete[] unk_0x88;
}

// Border of the whole map: bottom row and the side columns get the border element, the top row
// vacuum (all with 9999 kg so nothing flows into them).
void SimData::InitializeBoundary() {
    for (int x = 0; x < width; x++) {
        SetBorderCell(*this, x, gSimData->unk_0x1e);
        SetBorderCell(*this, (height - 1) * width + x, gSimData->vacuumElementIdx);
    }
    for (int y = 1; y < height - 1; y++) {
        SetBorderCell(*this, y * width, gSimData->unk_0x1e);
        SetBorderCell(*this, y * width + width - 1, gSimData->unk_0x1e);
    }
}

// Cells of the "settle" element take the mean temperature of their 8 neighbours that hold
// mass and are neither that element nor the border element.
void SettleThermalBoundaries(const SimData &sim, const CellSOA *cells, CellSOA *updated) {
    int w = gSimData->width;
    int rows = gSimData->height - 2;
    uint16_t settle = GetElementIndex(kSettleHash);
    uint16_t border = GetElementIndex(kBoundaryHash);
    if (rows <= 0 || w <= 2) {
        return;
    }
    const uint16_t *e = cells->elementIdx.data();
    const int offsets[8] = {-w - 1, -w, 1 - w, -1, 1, w - 1, w, w + 1};
    for (int y = 1; y <= rows; y++) {
        for (int c = y * sim.width + 1; c <= y * sim.width + w - 2; c++) {
            if (e[c] != settle) {
                continue;
            }
            float sum = 0.0f;
            float count = 0.0f;
            for (int k = 0; k < 8; k++) {
                int n = c + offsets[k];
                if (e[n] != settle && e[n] != border && 0.0f < cells->mass[n]) {
                    sum = k == 0 ? cells->temperature[n] : sum + cells->temperature[n];
                    count = count + 1.0f;
                }
            }
            if (!(count <= 0.0f)) {
                updated->temperature[c] = sum / count;
            }
        }
    }
}

void CopyCellSOA(CellSOA &dst, const CellSOA &src) {
    dst = src;
}

// Message: int width, int height, bool radiation, bool headless (game size; the sim adds a
// one-cell border). The seed comes from time().
SimData *AllocateCells(BinaryBufferReader &reader) {
    int w = reader.Read<int32_t>();
    int h = reader.Read<int32_t>();
    bool radiation = reader.Read<uint8_t>() != 0;
    bool headless = reader.Read<uint8_t>() != 0;
    SimData *created = new SimData(w + 2, h + 2, static_cast<unsigned>(time(nullptr)), radiation, headless);
    SimData *old = gSimData;
    gSimData = created;
    delete old;
    return gSimData;
}

// Message: one zone id per cell of the game grid, row by row.
long SetWorldZones(BinaryBufferReader &reader) {
    SimData &sim = *gSimData;
    size_t n = static_cast<size_t>(static_cast<long>(sim.width * sim.height));
    delete[] sim.unk_0x38;
    sim.unk_0x38 = new uint8_t[n]();
    for (int y = 1; y < sim.height - 1; y++) {
        reader.ReadBytes(static_cast<uint64_t>(sim.width - 2), sim.unk_0x38 + sim.width * y + 1);
    }
    if (reader.pos < reader.size && !gAssertsDisabled) {
        AssertFunc("BREAKPT:", 0x211, "SimData.cpp");
    }
    return 0;
}

// Message: int count, then count world rectangles (x, y, width, height).
long DefineWorldOffsets(BinaryBufferReader &reader) {
    int count = reader.Read<int32_t>();
    std::vector<WorldRect> &rects = gSimData->worldRects;
    rects.resize(static_cast<size_t>(static_cast<long>(count)));
    for (int i = 0; i < count; i++) {
        WorldRect &r = rects[static_cast<size_t>(i)];
        r.x = reader.Read<int32_t>();
        r.y = reader.Read<int32_t>();
        r.width = reader.Read<int32_t>();
        r.height = reader.Read<int32_t>();
    }
    return 0;
}

long ClearUnoccupiedCells(BinaryBufferReader &) {
    SimData &sim = *gSimData;
    for (long c = 0; c != static_cast<long>(sim.width * sim.height); c++) {
        for (CellSOA *cells : {sim.cells, sim.updatedCells}) {
            cells->elementIdx[c] = sim.vacuumElementIdx;
            cells->mass[c] = 0.0f;
            cells->temperature[c] = 0.0f;
            cells->diseaseCount[c] = 0;
            cells->diseaseIdx[c] = kNoDisease;
            cells->unk_0xf0[c] = 0.0f;
        }
    }
    return 0;
}

// Message: two bytes, options to clear and options to set.
long SetSavedOptions(BinaryBufferReader &reader) {
    uint8_t clear = reader.Read<uint8_t>();
    uint8_t set = reader.Read<uint8_t>();
    if (gSimData != nullptr) {
        gSimData->savedOptions = static_cast<uint8_t>((~clear & gSimData->savedOptions) | set);
    }
    return 0;
}

// Message: int width, int height, int x, int y (a world removed from the map): its cells
// become vacuum and its rectangle is dropped.
long FreeGridCells(BinaryBufferReader &reader) {
    int w = reader.Read<int32_t>();
    int h = reader.Read<int32_t>();
    int x = reader.Read<int32_t>();
    int y = reader.Read<int32_t>();
    SimData &sim = *gSimData;
    for (int dy = 0; dy <= h + 1 && h >= -1; dy++) {
        if (w < -1) {
            break;
        }
        for (int dx = 0; dx <= w + 1; dx++) {
            ClearToVacuum(sim, (y + dy) * sim.width + x + dx, true);
        }
    }
    std::vector<WorldRect> &rects = sim.worldRects;
    for (auto it = rects.begin(); it != rects.end(); ++it) {
        if (it->height == h && it->width == w && it->x == x && it->y == y) {
            rects.erase(it);
            break;
        }
    }
    return 0;
}

// Message: int width, int height, int x, int y, then the rest of the header (a new world area):
// its inside becomes vacuum and its edges the border element; the rectangle is recorded.
long ResizeAndInitializeVacuumCells(BinaryBufferReader &reader) {
    int unused_a = reader.Read<int32_t>();
    int unused_b = reader.Read<int32_t>();
    (void)unused_a;
    (void)unused_b;
    WorldRect r;
    r.width = reader.Read<int32_t>();
    r.height = reader.Read<int32_t>();
    r.x = reader.Read<int32_t>();
    r.y = reader.Read<int32_t>();
    SimData &sim = *gSimData;
    for (int dy = 0; dy < r.height; dy++) {
        for (int dx = 0; dx < r.width; dx++) {
            ClearToVacuum(sim, (r.y + dy + 1) * sim.width + 1 + r.x + dx, true);
        }
    }
    if (r.width >= -1) {
        for (int dx = 0; dx <= r.width + 1; dx++) {
            SetBorderCell(sim, r.y * sim.width + r.x + dx, sim.unk_0x1e);
            SetBorderCell(sim, (r.y + 1 + r.height) * sim.width + r.x + dx, sim.unk_0x1e);
        }
    }
    for (int dy = 1; dy <= r.height; dy++) {
        int row = r.x + (r.y + dy) * sim.width;
        SetBorderCell(sim, row, sim.unk_0x1e);
        SetBorderCell(sim, row + 1 + r.width, sim.unk_0x1e);
    }
    sim.worldRects.push_back(r);
    return static_cast<long>(r.width * r.height);
}

// Message (new world from worldgen): int width, int height, uint seed, bool radiation, bool
// headless, then per game cell 16 bytes of cell data (element, properties, insulation, strength,
// temperature, mass), 12 bytes of germs (index, infestation ticks, count) and 12 bytes of backwall.
long InitializeFromCells(BinaryBufferReader &reader) {
    int w = reader.Read<int32_t>();
    int h = reader.Read<int32_t>();
    unsigned seed = reader.Read<uint32_t>();
    bool radiation = reader.Read<uint8_t>() != 0;
    bool headless = reader.Read<uint8_t>() != 0;
    long n = static_cast<long>(w * h);
    const uint8_t *cell_data = reader.Current();
    reader.SkipBytes(static_cast<uint64_t>(n) * 16);
    const uint8_t *disease_data = reader.Current();
    reader.SkipBytes(static_cast<uint64_t>(n) * 12);
    const uint8_t *backwall_data = reader.Current();
    SimData *created = new SimData(w + 2, h + 2, seed, radiation, headless);
    SimData *old = gSimData;
    gSimData = created;
    delete old;
    SimData &sim = *gSimData;
    for (int y = 0; y < h; y++) {
        for (int x = 0; x < w; x++) {
            const uint8_t *src = cell_data + static_cast<size_t>(y * w + x) * 16;
            int c = sim.width * (y + 1) + 1 + x;
            CellSOA &u = *sim.updatedCells;
            uint16_t element;
            float temperature;
            float mass;
            std::memcpy(&element, src, 2);
            std::memcpy(&temperature, src + 8, 4);
            std::memcpy(&mass, src + 12, 4);
            u.elementIdx[c] = element;
            u.temperature[c] = temperature;
            u.unk_0xf0[c] = 0.0f;
            u.mass[c] = mass;
            u.properties[c] = src[2];
            u.insulation[c] = src[3];
            u.strengthInfo[c] = src[4];
        }
    }
    for (int y = 0; y < h; y++) {
        for (int x = 0; x < w; x++) {
            const uint8_t *src = disease_data + static_cast<size_t>(y * w + x) * 12;
            int c = sim.width * (y + 1) + 1 + x;
            CellSOA &u = *sim.updatedCells;
            int32_t count;
            std::memcpy(&count, src + 4, 4);
            u.diseaseIdx[c] = src[0];
            u.diseaseCount[c] = count;
            u.diseaseInfestationTickCount[c] = src[1];
            u.diseaseAccumulatedError[c] = 0.0f;
        }
    }
    for (int y = 0; y < h; y++) {
        for (int x = 0; x < w; x++) {
            const uint8_t *src = backwall_data + static_cast<size_t>(y * w + x) * 12;
            int c = sim.width * (y + 1) + 1 + x;
            BackwallSOA &bw = *sim.backwall;
            uint16_t element;
            float mass;
            float temperature;
            std::memcpy(&element, src, 2);
            std::memcpy(&mass, src + 4, 4);
            std::memcpy(&temperature, src + 8, 4);
            bw.elementIdx[c] = element;
            bw.mass[c] = mass;
            bw.temperature[c] = temperature;
        }
    }
    SettleThermalBoundaries(sim, sim.cells, sim.updatedCells);
    sim.InitializeBoundary();
    return n;
}

// --- Load / Save ---

namespace {

constexpr int kSaveVersion = 15;

void CellRead(CellSOA &cells, int cell, BinaryBufferReader &reader, int version) {
    uint32_t id = reader.Read<uint32_t>();
    if (id == 0x280cf79) {
        id = 0x987dac06;
    } else if (id == 0x2391c22b) {
        id = 0x4c76ae31;
    }
    cells.elementIdx[cell] = GetElementIndex(id);
    reader >> cells.temperature[cell];
    reader >> cells.mass[cell];
    if (version > 13) {
        reader >> cells.unk_0xf0[cell];
    }
    if (cells.elementIdx[cell] == kInvalidElement) {
        cells.elementIdx[cell] = GetElementIndex(kVacuumHash);
        cells.mass[cell] = 0.0f;
        cells.temperature[cell] = 0.0f;
        cells.unk_0xf0[cell] = 0.0f;
    }
}

void DiseaseRead(CellSOA &cells, int cell, BinaryBufferReader &reader) {
    uint32_t hash = reader.Read<uint32_t>();
    reader >> cells.diseaseCount[cell];
    cells.diseaseIdx[cell] = hash != 0 ? gDisease->GetDiseaseIndex(hash) : kNoDisease;
    if (cells.diseaseIdx[cell] == kNoDisease) {
        cells.diseaseCount[cell] = 0;
    }
}

void BackwallRead(BackwallSOA &bw, int cell, BinaryBufferReader &reader) {
    bw.elementIdx[cell] = GetElementIndex(reader.Read<uint32_t>());
    reader >> bw.mass[cell];
    reader >> bw.temperature[cell];
}

// A saved cell past its transition temperature (by more than 3 K) turns into the element it
// would have become, 1.5 K back inside the range.
void DoLoadTimeStateTransition(SimData &sim, int cell) {
    uint16_t &element = sim.updatedCells->elementIdx[cell];
    float &temperature = sim.updatedCells->temperature[cell];
    const ElementTemperatureData &e = gElementTemperatureData[element];
    float t = temperature;
    if (!(e.lowTemp - 3.0f <= t) && e.lowTempTransitionIdx != kInvalidElement) {
        t = t + 1.5f;
        temperature = t;
        element = e.lowTempTransitionIdx;
    } else if (e.highTemp + 3.0f < t && e.highTempTransitionIdx != kInvalidElement) {
        t = t - 1.5f;
        temperature = t;
        element = e.highTempTransitionIdx;
    }
    if (t > 10000.0f) {
        AssertFunc("BREAKPT:", 0xab, "UpdateTemperature.cpp");
    }
}

}  // namespace

// Message: a save written by SIM_BeginSave ("SIMSAVE\0", version, size, offset, options, then
// cells, germs and backwall), loaded into a rectangle of the current grid.
SimData *Load(BinaryBufferReader &reader) {
    static const char kMagic[8] = {'S', 'I', 'M', 'S', 'A', 'V', 'E', 0};
    if (std::memcmp(reader.Current(), kMagic, 8) != 0) {
        return nullptr;
    }
    reader.SkipBytes(8);
    int version = reader.Read<int32_t>();
    if (version > kSaveVersion) {
        return nullptr;
    }
    int w = reader.Read<int32_t>();
    int h = reader.Read<int32_t>();
    int x = 0;
    int y = 0;
    if (version > 13) {
        x = reader.Read<int32_t>();
        y = reader.Read<int32_t>();
    }
    uint8_t options = 0;
    if (version > 12) {
        options = reader.Read<uint8_t>();
    }
    SimData &sim = *gSimData;
    if (sim.savedOptions != 0 && sim.savedOptions != options) {
        __builtin_trap();  // the original's assert(savedOptions == 0 || savedOptions == newSaveOptions)
    }
    sim.savedOptions = options;
    int origin = y * sim.width + x;
    if (h != 0) {
        for (int row = 0; row < h && w > 0; row++) {
            for (int col = 0; col < w; col++) {
                int c = origin + row * sim.width + col;
                CellRead(*sim.updatedCells, c, reader, version);
                CellSOA &u = *sim.updatedCells;
                if (u.temperature[c] <= 0.0f) {
                    u.temperature[c] = 293.0f;
                }
                if (u.unk_0xf0[c] <= 0.0f) {
                    u.unk_0xf0[c] = 0.0f;
                }
                if (u.elementIdx[c] == sim.unk_0x1a || u.elementIdx[c] == sim.vacuumElementIdx) {
                    u.temperature[c] = 0.0f;
                    u.mass[c] = 0.0f;
                    u.unk_0xf0[c] = 0.0f;
                }
                DoLoadTimeStateTransition(sim, c);
            }
        }
        if (version > 8) {
            for (int row = 0; row < h && w != 0; row++) {
                for (int col = 0; col < w; col++) {
                    DiseaseRead(*sim.updatedCells, origin + row * sim.width + col, reader);
                }
            }
            if (version > 14) {
                for (int row = 0; row < h && w != 0; row++) {
                    for (int col = 0; col < w; col++) {
                        BackwallRead(*sim.backwall, origin + row * sim.width + col, reader);
                    }
                }
            }
        }
    } else if (version > 14) {
        if (reader.pos < reader.size && !gAssertsDisabled) {
            AssertFunc("BREAKPT:", 0x2d1, "SimBindings.cpp");
        }
        return gSimData;
    }
    if (static_cast<unsigned>(version - 8) < 7) {
        reader.SkipBytes(static_cast<uint64_t>(static_cast<long>(h) * w * 4));
    }
    if (version < 11) {
        sim.unk_0x718 = true;
    }
    if (reader.pos < reader.size && !gAssertsDisabled) {
        AssertFunc("BREAKPT:", 0x2d1, "SimBindings.cpp");
    }
    return gSimData;
}

namespace {

std::vector<uint8_t> *gSaveBuffer = nullptr;

template <typename T>
void Write(std::vector<uint8_t> &out, T value) {
    const uint8_t *p = reinterpret_cast<const uint8_t *>(&value);
    out.insert(out.end(), p, p + sizeof value);
}

}  // namespace

extern "C" __attribute__((visibility("default"))) void SIM_EndSave() {
    delete gSaveBuffer;
    gSaveBuffer = nullptr;
}

// The whole grid as a save: header, then per cell (element hash, temperature, mass, radiation),
// per cell (germ hash, count) and per cell backwall (element hash, mass, temperature).
extern "C" __attribute__((visibility("default"))) uint8_t *SIM_BeginSave(int *size, int x, int y) {
    SIM_EndSave();
    SimData &sim = *gSimData;
    int n = sim.width * sim.height;
    *size = n * 0x24 + 0x1d;
    gSaveBuffer = new std::vector<uint8_t>();
    std::vector<uint8_t> &out = *gSaveBuffer;
    out.reserve(static_cast<size_t>(*size));
    const char magic[8] = {'S', 'I', 'M', 'S', 'A', 'V', 'E', 0};
    out.insert(out.end(), magic, magic + 8);
    Write<int32_t>(out, kSaveVersion);
    Write<int32_t>(out, sim.width);
    Write<int32_t>(out, sim.height);
    Write<int32_t>(out, x);
    Write<int32_t>(out, y);
    Write<uint8_t>(out, sim.savedOptions);
    const CellSOA &c = *sim.updatedCells;
    for (int i = 0; i < n; i++) {
        Write<int32_t>(out, gElements[c.elementIdx[i]].id);
        Write<float>(out, c.temperature[i]);
        Write<float>(out, c.mass[i]);
        Write<float>(out, c.unk_0xf0[i]);
    }
    for (int i = 0; i < n; i++) {
        uint8_t idx = c.diseaseIdx[i];
        Write<uint32_t>(out, idx != kNoDisease ? gDisease->Info(idx).hash : 0);
        Write<int32_t>(out, c.diseaseCount[i]);
    }
    BackwallSOA &bw = *sim.backwall;
    for (int i = 0; i < n; i++) {
        uint16_t element = bw.elementIdx[i];
        if (element == kInvalidElement) {
            element = GetElementIndex(kVacuumHash);
            bw.mass[i] = 0.0f;
            bw.temperature[i] = 0.0f;
        }
        Write<int32_t>(out, gElements[element].id);
        Write<float>(out, bw.mass[i]);
        Write<float>(out, bw.temperature[i]);
    }
    if (static_cast<int>(out.size()) != *size && !gAssertsDisabled) {
        AssertFunc("BREAKPT:", 0x1a2, "SimBindings.cpp");
    }
    return out.data();
}
