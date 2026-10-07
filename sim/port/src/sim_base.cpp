// SimBase: the per-frame driver around the simulation data.
#include <cstring>

#include "sim_common.h"

namespace {

template <typename T>
void CopyVector(std::vector<T> &dst, const std::vector<T> &src) {
    dst.resize(src.size());
    if (!src.empty()) {
        std::memcpy(dst.data(), src.data(), src.size() * sizeof(T));
    }
}

}  // namespace

// Makes `cells` (the previous state read by the next pass) equal to `updatedCells`. The region
// arguments are ignored by the original too.
void SimBase::UpdatedCellsToCells(SimData &sim, Vector2<int> min, Vector2<int> max) {
    (void)min;
    (void)max;
    CellSOA &cells = *sim.cells;
    const CellSOA &upd = *sim.updatedCells;
    CopyVector(cells.elementIdx, upd.elementIdx);
    CopyVector(cells.temperature, upd.temperature);
    CopyVector(cells.mass, upd.mass);
    CopyVector(cells.properties, upd.properties);
    CopyVector(cells.insulation, upd.insulation);
    CopyVector(cells.strengthInfo, upd.strengthInfo);
    CopyVector(cells.diseaseIdx, upd.diseaseIdx);
    CopyVector(cells.diseaseCount, upd.diseaseCount);
    CopyVector(cells.diseaseInfestationTickCount, upd.diseaseInfestationTickCount);
    CopyVector(cells.diseaseAccumulatedError, upd.diseaseAccumulatedError);
    CopyVector(cells.unk_0xf0, upd.unk_0xf0);
}

void SimBase::CopyUpdatedCellsToCells(SimData &sim) {
    UpdatedCellsToCells(sim, Vector2<int>{0, 0}, Vector2<int>{sim.width, sim.height});
}

namespace {

// SimBase fields used by UpdateData (offsets until SimBase is mapped in layout.txt).
template <typename T>
T &Field(SimBase *self, size_t offset) {
    return *reinterpret_cast<T *>(reinterpret_cast<char *>(self) + offset);
}

// Radiation emitted by a cell spreads over this 5x5 kernel: {dx, dy, weight}.
struct KernelEntry {
    int dx, dy;
    float weight;
};
constexpr KernelEntry kRadiationKernel[25] = {
    {-2, -2, 0.1f},  {-1, -2, 0.15f}, {0, -2, 0.25f}, {1, -2, 0.15f}, {2, -2, 0.1f},
    {-2, -1, 0.15f}, {-1, -1, 0.5f},  {0, -1, 0.75f}, {1, -1, 0.5f},  {2, -1, 0.15f},
    {-2, 0, 0.25f},  {-1, 0, 0.75f},  {0, 0, 1.0f},   {1, 0, 0.75f},  {2, 0, 0.25f},
    {-2, 1, 0.15f},  {-1, 1, 0.5f},   {0, 1, 0.75f},  {1, 1, 0.5f},   {2, 1, 0.15f},
    {-2, 2, 0.1f},   {-1, 2, 0.15f},  {0, 2, 0.25f},  {1, 2, 0.15f},  {2, 2, 0.1f},
};

constexpr uint32_t kRadiationSicknessHash = 0xd49f77d6u;

// C integer division by 32 (rounds toward zero), as the original computes chunk bounds.
int Div32(int x) {
    return (x < 0 ? x + 31 : x) >> 5;
}

// Pressure equalisation for one gas cell with its side, upper and upper-side neighbours.
void GasPressure(SimData &sim, int &c) {
    CellSOA *cells = sim.cells;
    CellSOA *upd = sim.updatedCells;
    const uint16_t *e = cells->elementIdx.data();
    const uint8_t *props = cells->properties.data();
    const uint16_t *ue = upd->elementIdx.data();
    const ElementPressureData *pd = gElementPressureData.data();
    const int w = sim.width;
    int dir = sim.flowDirection;
    int side = c + dir;
    if (props[c] & 1) {
        c = side;
        return;
    }
    uint16_t element = e[c];
    if (element != ue[c] || (pd[element].state & 2)) {
        c = side;
        return;
    }
    signed char state = static_cast<signed char>(pd[element].state & 3);
    float flow = pd[element].flow;
    float horizontal = 0.0f;
    if ((props[side] & 1) == 0) {
        uint16_t se = e[side];
        if (se == ue[side] && (pd[se].state & 2) == 0 && (StateOf(pd[se].state) != state || element == se)) {
            horizontal = UpdatePressure(sim, *sim.events, cells, upd, c, element, state, flow, side);
            dir = sim.flowDirection;
            horizontal = static_cast<float>(dir) * horizontal;
        }
    }
    Vector4f *flowed = sim.unk_0x60;
    flowed[c].y = flowed[c].y + horizontal;
    flowed[side].x = flowed[side].x - horizontal;
    int up = w + c;
    uint8_t up_state = gElementPressureData[e[up]].state;
    float vertical = 0.0f;
    if (element == ue[c] && (props[up] & 1) == 0 && ue[up] == e[up] && (up_state & 2) == 0 &&
        (StateOf(up_state) != state || element == e[up])) {
        vertical = UpdatePressure(sim, *sim.events, cells, upd, c, element, state, flow, up);
        flowed = sim.unk_0x60;
        dir = sim.flowDirection;
        up_state = gElementPressureData[e[up]].state;
    }
    int next = c + dir;
    flowed[c].z = flowed[c].z + vertical;
    flowed[up].w = flowed[up].w - vertical;
    if (((up_state | gElementPressureData[e[next]].state) & 3) != 3 && element == ue[c]) {
        int diagonal = next + w;
        uint16_t de = e[diagonal];
        if ((props[diagonal] & 1) == 0 && de == ue[diagonal] && (gElementPressureData[de].state & 2) == 0 &&
            (StateOf(gElementPressureData[de].state) != state || element == de)) {
            UpdatePressure(sim, *sim.events, cells, upd, c, element, state, flow, diagonal);
            dir = sim.flowDirection;
            next = c + dir;
        }
    }
    c = next;
}

// Bounds of the pressure displacement passes. The original takes min(minY, 3) where max was
// presumably meant, so these passes start at row 3 for every region above it.
struct DisplacementBounds {
    int x, y0, y1, count;
};

DisplacementBounds DisplacementArea(const SimData &sim, const ActiveRegion &r) {
    DisplacementBounds b;
    b.x = r.minX < 3 ? 3 : r.minX;
    b.y0 = r.minY <= 3 ? r.minY : 3;
    b.y1 = sim.height - 3 <= r.maxY ? sim.height - 3 : r.maxY;
    int x_end = sim.width - 3 > r.maxX ? r.maxX : sim.width - 3;
    b.count = x_end - b.x;
    int width = r.maxX - r.minX;
    if (b.count > width) {
        b.count = width;
    }
    return b;
}

void Radiation(SimData &sim, const ActiveRegion &r) {
    const int w = sim.width;
    const int h = sim.height;
    uint8_t sickness = gDisease->GetDiseaseIndex(kRadiationSicknessHash);
    size_t cells = static_cast<size_t>(h * w);
    if (sim.radiationLight.size() < cells) {
        sim.radiationLight.resize(cells, 1.0f);
    } else if (sim.radiationLight.size() > cells) {
        sim.radiationLight.resize(cells);
    }
    const ElementRadiationData *rad = gElementRadiationData.data();
    const int rw = r.maxX - r.minX;
    // Sky radiation, top down: each cell lets through what its matter does not absorb, and the
    // cell's own radiation decays.
    if (r.minY <= r.maxY) {
        for (int y = r.maxY; y != r.minY - 1; y--) {
            int start = r.minX + y * w;
            if (!(start < start + rw)) {
                continue;
            }
            CellSOA &upd = *sim.updatedCells;
            float *light = sim.radiationLight.data();
            const uint16_t *e = upd.elementIdx.data();
            const uint8_t *props = upd.properties.data();
            float *cell_radiation = upd.unk_0xf0.data();
            for (int c = start; c < start + rw; c++) {
                int above_row = c / w + 1;
                float incoming = 1.0f;
                if (above_row < r.maxY) {
                    incoming = light[above_row * w + c % w];
                }
                float absorb;
                if (static_cast<int8_t>(props[c]) < 0) {
                    absorb = rad[e[c]].radiationAbsorptionFactor * sim.unk_0x72c;
                } else {
                    absorb = rad[e[c]].radiationAbsorptionFactor *
                             ((upd.mass[c] / sim.unk_0x720) * sim.unk_0x728 + sim.unk_0x724);
                }
                absorb = MaxSS(MinSS(absorb, 1.0f), 0.0f);
                float through = (1.0f - absorb) * incoming;
                light[c] = 0.01f > through ? 0.0f : MaxSS(through, 0.0f);
                float own = cell_radiation[c];
                float decay = own / sim.unk_0x71c;
                if (decay == 0.0f) {
                    cell_radiation[c] = 1.0f < own ? own - 1.0f : decay;
                } else {
                    cell_radiation[c] = own - decay;
                }
            }
        }
    }
    if (!(r.minY <= r.maxY) || r.maxY == r.minY) {
        return;
    }
    // Emitters, germs of radiation sickness, and the sky contribution; then clamping.
    for (int y = r.minY; y != r.maxY; y++) {
        int start = r.minX + y * w;
        CellSOA &upd = *sim.updatedCells;
        const float *light = sim.radiationLight.data();
        float *cell_radiation = upd.unk_0xf0.data();
        for (int c = start; c < start + rw; c++) {
            uint16_t element = upd.elementIdx[c];
            if (0.0f < rad[element].radiationPer1000Mass) {
                float kmass = upd.mass[c] * 0.001f;
                int cx = c % w, cy = c / w;
                for (const KernelEntry &k : kRadiationKernel) {
                    int nx = k.dx + cx;
                    int ny = k.dy + cy;
                    if (r.minX <= nx && r.minY <= ny && ny <= r.maxY && nx <= r.maxX) {
                        float &target = cell_radiation[ny * w + nx];
                        target = (rad[element].radiationPer1000Mass * kmass) * k.weight + target;
                    }
                }
            }
            float value = cell_radiation[c];
            if (upd.diseaseIdx[c] == sickness) {
                value = value + static_cast<float>(upd.diseaseCount[c]) * 0.001f;
                cell_radiation[c] = value;
            }
            float sky = light[c];
            if (0.0f < sky) {
                value = value + sky * (r.cosmicRadiationIntensity / sim.unk_0x71c);
                cell_radiation[c] = value;
            }
            if (0.0f > value) {
                cell_radiation[c] = 0.0f;
            } else if (value > 9000000.0f) {
                cell_radiation[c] = 9000000.0f;
            } else if (!(0.01f < value)) {
                cell_radiation[c] = 0.0f;
            }
        }
    }
}

}  // namespace

// One simulation tick (0.2 s) over every active region.
void SimBase::UpdateData(SimData &sim) {
    const int w = sim.width;
    if (w == 0) {
        return;
    }
    sim.flowDirection = -sim.flowDirection;
    CopyUpdatedCellsToCells(sim);
    std::vector<SimUpdateTemperatureTask *> &tasks = Field<std::vector<SimUpdateTemperatureTask *>>(this, 0xe0);
    std::vector<SimEvents *> &task_events = Field<std::vector<SimEvents *>>(this, 0xf8);
    char *queue = Field<char *>(this, 0xd8);
    for (size_t ri = 0; ri < sim.activeRegions.size(); ri++) {
        const ActiveRegion r = sim.activeRegions[ri];
        const int rw = r.maxX - r.minX;
        const int rh = r.maxY - r.minY;

        // Heat exchange and state transitions, one band of rows per worker task. The original
        // has a single worker, so running the tasks in order here is what it does.
        int workers = static_cast<int>((*reinterpret_cast<char **>(queue + 0xe8) -
                                        *reinterpret_cast<char **>(queue + 0xe0)) / 8);
        int row = r.minY;
        for (int i = 0; i < static_cast<int>(tasks.size()); i++) {
            SimUpdateTemperatureTask *task = tasks[i];
            int next = rh / workers + row + (i < rh % workers ? 1 : 0);
            task->region.minY = row;
            task->region.minX = r.minX;
            task->sim = &sim;
            task->events = task_events[i];
            task->region.maxX = r.maxX;
            task->region.maxY = next;
            task->InternalDoTask();
            row = next;
        }
        CopyUpdatedCellsToCells(sim);

        // Gas pressure, alternating the scan direction every tick.
        for (int y = r.minY; y < r.maxY; y++) {
            int start = y * sim.width + r.minX;
            // Forward scans cover [start, start + rw); backward scans (start, start + rw].
            int from = start, to = start + rw;
            if (sim.flowDirection < 0) {
                std::swap(from, to);
            }
            int c = from;
            while (c != to) {
                GasPressure(sim, c);
            }
        }

        // A heavier gas pushing a different gas out of the way.
        DisplacementBounds gb = DisplacementArea(sim, r);
        for (int y = gb.y0; y < gb.y1; y++) {
            int start = gb.x + y * sim.width;
            for (int c = start; c < start + gb.count; c++) {
                const uint16_t *e = sim.cells->elementIdx.data();
                const uint8_t *props = sim.cells->properties.data();
                uint16_t element = e[c];
                if (StateOf(gElementPressureData[element].state) != kGas) {
                    continue;
                }
                auto push = [&](int neighbour, int beyond) {
                    if (element != e[neighbour] && (props[neighbour] & 1) == 0 &&
                        StateOf(gElementPressureData[sim.cells->elementIdx[neighbour]].state) == kGas) {
                        float moved = DoGasPressureDisplacement(element, c, neighbour, beyond, sim);
                        Vector4f *flowed = sim.unk_0x60;
                        flowed[c].w = flowed[c].w - moved;
                        flowed[neighbour].z = moved + flowed[neighbour].z;
                    }
                };
                push(c - w, c + w * -2);
                int d = sim.flowDirection;
                push(c - d, c - d - d);
                d = sim.flowDirection;
                push(d + c, d * 2 + c);
                push(w + c, w * 2 + c);
            }
        }
        CopyUpdatedCellsToCells(sim);
        ConsolidateEvents(sim);

        if (r.maxX != r.minX) {
            // Liquids.
            for (int y = r.minY; y < r.maxY; y++) {
                int start = w * y + r.minX;
                for (int c = start; c < start + rw; c++) {
                    if (StateOf(gElementLiquidData[sim.cells->elementIdx[c]].state) == kLiquid &&
                        0.0f < sim.cells->mass[c]) {
                        UpdateLiquid(sim, *sim.events, c);
                    }
                }
            }
            // A liquid with enough excess pushing a different liquid out of the way.
            DisplacementBounds lb = DisplacementArea(sim, r);
            const uint16_t *e = sim.cells->elementIdx.data();
            for (int y = lb.y0; y < lb.y1; y++) {
                int start = lb.x + y * w;
                for (int c = start; c < start + lb.count; c++) {
                    uint16_t element = e[c];
                    if (StateOf(gElementPressureData[element].state) != kLiquid) {
                        continue;
                    }
                    int d = sim.flowDirection;
                    int side = c - d;
                    if (e[side] != element) {
                        float moved = DoLiquidPressureDisplacement(element, c, side, side - d, sim);
                        Vector4f *flowed = sim.unk_0x60;
                        d = sim.flowDirection;
                        flowed[c].x = flowed[c].x + moved;
                        flowed[c - 1].y = flowed[c - 1].y - moved;
                    }
                    side = c + d;
                    if (e[side] != element) {
                        float moved = DoLiquidPressureDisplacement(element, c, side, d + side, sim);
                        Vector4f *flowed = sim.unk_0x60;
                        flowed[c].x = flowed[c].x - moved;
                        flowed[c - 1].y = moved + flowed[c - 1].y;
                    }
                    int up = w + c;
                    if (e[up] != element && gElementPostProcessData[sim.updatedCells->elementIdx[c]].maxMass <=
                                                sim.updatedCells->mass[c]) {
                        float moved = DoLiquidPressureDisplacement(element, c, up, w * 2 + c, sim);
                        Vector4f *flowed = sim.unk_0x60;
                        flowed[c].w = flowed[c].w + moved;
                        flowed[up].z = flowed[up].z - moved;
                    }
                }
            }
            CopyUpdatedCellsToCells(sim);
        }

        // Germs spreading right and up.
        const uint8_t *germs = sim.cells->diseaseIdx.data();
        for (int y = r.minY; y < r.maxY; y++) {
            int start = w * y + r.minX;
            for (int c = start; c < start + rw; c++) {
                if (germs[c] != kNoDisease || germs[c + 1] != kNoDisease) {
                    gDisease->UpdateCells(0.2f, sim, *sim.events, c, c + 1);
                }
                if (germs[c] != kNoDisease || germs[c + w] != kNoDisease) {
                    gDisease->UpdateCells(0.2f, sim, *sim.events, c, c + w);
                }
            }
        }

        if (sim.radiationEnabled) {
            Radiation(sim, r);
        }

        Region region{r.minX, r.minY, r.maxX, r.maxY};
        sim.UpdateComponents(0.2f, region);

        // Per-cell post processing over the region rounded out to 32-cell chunks.
        const int sw = sim.width;
        int x0 = Div32(r.minX - 1) * 32 + 1;
        if (x0 < r.minX) {
            x0 = r.minX;
        }
        int x1 = Div32(r.maxX + 30) * 32 + 1;
        if (sw - 1 < x1) {
            x1 = sw - 1;
        }
        if (!(x1 <= r.maxX + 1)) {
            x1 = r.maxX + 1;
        }
        int y0 = Div32(r.minY - 1) * 32 + 1;
        if (y0 < r.minY) {
            y0 = r.minY;
        }
        int y1 = Div32(r.maxY + 30) * 32 + 1;
        if (sim.height - 1 < y1) {
            y1 = sim.height - 1;
        }
        if (!(y1 <= r.maxY + 1)) {
            y1 = r.maxY + 1;
        }
        if (y0 < y1 && x0 < x1) {
            if (sim.unk_0x38 != nullptr) {
                // Cells outside every world lose their matter.
                const BackwallSOA &wall = *sim.backwall;
                for (int y = y0; y < y1; y++) {
                    for (int c = y * sw + x0; c < y * sw + x1; c++) {
                        if (sim.unk_0x38[c] != 0xff || wall.elementIdx[c] != wall.noneElementIdx) {
                            continue;
                        }
                        uint8_t state = StateOf(gElementPostProcessData[sim.updatedCells->elementIdx[c]].state);
                        if (state == kSolid) {
                            continue;
                        }
                        float drain = state == kLiquid ? 20.0f : (state == kGas ? 0.02f : 0.0f);
                        float &m = sim.updatedCells->mass[c];
                        float left = m - drain;
                        m = left <= 0.0f ? 0.0f : left;
                        sim.unk_0x60[c] = Vector4f{-0.2f, 0.2f, 0.2f, -0.2f};
                    }
                }
            }
            for (int y = y0; y < y1; y++) {
                int row_w = sim.width;
                for (int c = row_w * y + x0; c < row_w * y + x1; c++) {
                    PostProcessCell(sim, *sim.events, c);
                }
            }
        }
        gDisease->PostProcess(sim, *sim.events, x0, x1, y0, y1);
    }
    sim.displacementDirection = static_cast<uint16_t>(sim.displacementDirection + 1);
}

void SimData::UpdateComponents(float dt, const Region &region) {
    for (void *component : components) {
        static_cast<SimComponent *>(component)->Update(dt, *this, region);
    }
}

void SimData::UpdateComponentsDataListOnly() {
    for (void *component : components) {
        static_cast<SimComponent *>(component)->UpdateDataListOnly(*this);
    }
}

// Runs every frame the game queued since the last call. A frame with a positive dt advances the
// simulation in fixed 0.2 s ticks (at least one), carrying the leftover time in the remainder at
// 0x158; a paused frame only applies its messages and refreshes what the components report.
void SimBase::Step(SimData &sim, GameData &game, const GameData &previous) {
    char *self = reinterpret_cast<char *>(this);
    SimFrameManager &frames = *reinterpret_cast<SimFrameManager *>(self + 8);
    float &remainder = *reinterpret_cast<float *>(self + 0x158);
    int frame_count = frames.BeginFrameProcessing();
    std::memset(sim.unk_0x60, 0, static_cast<long>(sim.width) * static_cast<long>(sim.height) * sizeof(Vector4f));
    if (frame_count <= 0) {
        sim.UpdateComponentsDataListOnly();
    } else {
        for (long frame = 0; frame < frame_count; ++frame) {
            float dt = frames.ProcessNextFrame(sim);
            if (dt > 0.0f) {
                float total = dt + remainder;
                int ticks = static_cast<int>(total * 5.0f);
                if (ticks <= 0) {
                    ticks = 1;
                }
                remainder = MaxSS(total - static_cast<float>(ticks) * 0.2f, 0.0f);
                for (int tick = 0; tick < ticks; ++tick) {
                    UpdateData(sim);
                }
            } else {
                CopyUpdatedCellsToCells(sim);
                sim.UpdateComponentsDataListOnly();
            }
        }
    }
    frames.EndFrameProcessing();
    CopySimDataToGame(sim, game, previous, frame_count);
}
