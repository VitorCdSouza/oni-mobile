// Moving matter between cells: mixing mass into a cell, pushing a gas or liquid out of the way,
// and clearing a cell that runs out of mass.
#include "sim_common.h"

namespace {

void PushChangeAndFlag(SimData &sim, SimEvents &events, int cell) {
    int game_cell = ToGameCell(sim, cell);
    if (IsValidGameCell(sim, game_cell)) {
        PushSubstanceChange(events, game_cell);
    }
    sim.unk_0x68[cell] |= 0x1f;
}

}  // namespace

// Adds `mass` at `temperature` (with germs) to `cell`. The current contents are read from `src`
// and the result written to `dst`; callers pass the same CellSOA for both.
void AddMassAndUpdateTemperature(CellSOA *src, CellSOA *dst, int cell, float mass, float temperature,
                                 uint8_t disease_idx, int disease_count) {
    float cell_mass = src->mass[cell];
    float total = mass + cell_mass;
    float *out_temperature = &dst->temperature[cell];
    if (!(total > 0.0f)) {
        *out_temperature = 0.0f;
        ClearDisease(*dst, cell);
    } else {
        float cell_temperature = src->temperature[cell];
        float mixed = (mass * temperature + cell_mass * cell_temperature) / total;
        // Clamp the mix between the two inputs, with the original's minss/compare-mask semantics.
        float result;
        if (cell_temperature > temperature) {
            result = (temperature <= mixed) ? MinSS(cell_temperature, mixed) : temperature;
        } else {
            result = !(mixed >= cell_temperature) ? cell_temperature : MinSS(mixed, temperature);
        }
        dst->mass[cell] = total;
        *out_temperature = result;
        gDisease->AddDiseaseToCell(dst, cell, disease_idx, disease_count);
    }
    float t = dst->temperature[cell];
    if (IsNonZero(dst->mass[cell]) && 0.0f >= t) {
        AssertFunc("BREAKPT:", 0x30, "SimUtil.cpp");
        KCrashReporterReportMessage(
            "SimUtil.cpp", 0x30,
            "Assert failed: write_cell.mass() == 0.0f || write_cell.temperature() > SIM_MIN_TEMPERATURE");
        t = dst->temperature[cell];
    }
    if (t > 10000.0f) {
        AssertFunc("BREAKPT:", 0x31, "SimUtil.cpp");
        KCrashReporterReportMessage(
            "SimUtil.cpp", 0x31, "AddMassAndUpdateTemperature cell_idx=%d mass=%f mass_delta=%f temperature_param=%f",
            cell, static_cast<double>(src->mass[cell]), static_cast<double>(mass), static_cast<double>(temperature));
    }
}

// Moves everything in `from` into `to` and leaves `from` empty.
void DoDisplacement(SimData &sim, SimEvents &events, CellSOA *soa, int from, int to) {
    AddMassAndUpdateTemperature(soa, soa, to, soa->mass[from], soa->temperature[from], soa->diseaseIdx[from],
                                soa->diseaseCount[from]);
    soa->elementIdx[to] = soa->elementIdx[from];
    if (IsNonZero(soa->mass[to]) && soa->temperature[to] <= 0.0f) {
        AssertFunc("BREAKPT:", 0x3c, "SimUtil.cpp");
        KCrashReporterReportMessage("SimUtil.cpp", 0x3c,
                                    "Assert failed: dest_cell.mass() == 0.0f || dest_cell.temperature() > 0.0f");
    }
    CellAccessor source = Accessor(soa, from);
    sim.ClearCell(source);
    PushChangeAndFlag(sim, events, from);
    PushChangeAndFlag(sim, events, to);
}

// Pushes the gas in `cell` into a neighbour holding the same gas or vacuum: first the four
// direct neighbours in a rotating order, then the two upper diagonals (only when the side cell
// between them is not solid). `element` is the gas that may be merged into.
bool DisplaceGas(SimData &sim, SimEvents &events, CellSOA *soa, int cell, uint16_t element) {
    if (soa->mass[cell] <= 0.0f || StateOf(gElementPostProcessData[soa->elementIdx[cell]].state) != kGas) {
        return false;
    }
    const uint16_t *elements = soa->elementIdx.data();
    unsigned direction = sim.displacementDirection;
    const int neighbours[4] = {sim.width + cell, cell - 1, cell + 1, cell - sim.width};
    auto accepts = [&](int n) {
        return (elements[n] == element || elements[n] == sim.vacuumElementIdx) && (soa->properties[n] & 1) == 0;
    };
    for (unsigned k = 0; k < 4; k++) {
        int n = neighbours[(direction + k) & 3];
        if (accepts(n)) {
            DoDisplacement(sim, events, soa, cell, n);
            return true;
        }
    }
    int above = neighbours[0];
    const int diagonals[2] = {above - 1, above + 1};
    const int sides[2] = {cell - 1, cell + 1};
    for (unsigned k = 0; k < 2; k++) {
        unsigned i = (direction + k) & 1;
        int n = diagonals[i];
        bool side_solid = StateOf(gElementLiquidData[sim.updatedCells->elementIdx[sides[i]]].state) == kSolid;
        if (elements[n] == element && !side_solid && (soa->properties[n] & 1) == 0) {
            DoDisplacement(sim, events, soa, cell, n);
            return true;
        }
    }
    return false;
}

// Spreads the liquid in `cell` evenly over the candidates holding the same liquid or vacuum
// (and not liquid-impermeable), then empties `cell`.
bool DisplaceLiquidSimple(SimData &sim, SimEvents &events, int cell, uint16_t element, int num_candidates,
                          const int *candidates) {
    if (num_candidates >= 5) {
        AssertFunc("BREAKPT:", 0x81, "SimUtil.cpp");
        KCrashReporterReportMessage("SimUtil.cpp", 0x81, "Assert failed: num_candidate_cells <= kMaxNeighbors");
    }
    CellSOA *soa = sim.updatedCells;
    float mass = soa->mass[cell];
    if (mass < 0.01f) {
        return false;
    }
    if (num_candidates < 5 && num_candidates < 1) {
        return false;
    }
    const uint16_t *elements = soa->elementIdx.data();
    int targets[4];
    int count = 0;
    for (int i = 0; i < num_candidates; i++) {
        int n = candidates[i];
        if ((elements[n] == element || elements[n] == sim.vacuumElementIdx) && (soa->properties[n] & 2) == 0) {
            targets[count++] = n;
        }
    }
    if (count == 0) {
        return false;
    }
    CellAccessor source = Accessor(soa, cell);
    int disease = soa->diseaseCount[cell];
    for (int i = 0; i < count; i++) {
        int n = targets[i];
        AddMassAndUpdateTemperature(soa, soa, n, mass / static_cast<float>(count), source.soa->temperature[cell],
                                    source.soa->diseaseIdx[cell], (disease + count - 1) / count);
        sim.updatedCells->elementIdx[n] = element;
        PushChangeAndFlag(sim, events, n);
        soa = sim.updatedCells;
    }
    sim.ClearCell(source);
    PushChangeAndFlag(sim, events, cell);
    return true;
}

// Moves the liquid in `cell` out of the way: into matching neighbours, or by first pushing a
// neighbouring gas away and trying again.
bool DisplaceLiquid(SimData &sim, SimEvents &events, CellSOA *soa, int cell, uint16_t element) {
    if (!(0.0f < soa->mass[cell]) || StateOf(gElementPostProcessData[soa->elementIdx[cell]].state) != kLiquid) {
        return false;
    }
    const int neighbours[4] = {cell + 1, cell - 1, sim.width + cell, cell - sim.width};
    if (DisplaceLiquidSimple(sim, events, cell, element, 4, neighbours)) {
        return true;
    }
    for (unsigned k = 0; k < 4; k++) {
        int n = neighbours[(sim.displacementDirection + k) & 3];
        uint16_t n_element = soa->elementIdx[n];
        if (StateOf(gElements[n_element].state) == kGas && DisplaceGas(sim, events, soa, n, n_element)) {
            return DisplaceLiquidSimple(sim, events, cell, element, 4, neighbours);
        }
    }
    return false;
}

// Empties a cell that ran out of mass.
void Evaporate(int cell, SimData &sim, SimEvents &events) {
    CellAccessor accessor = Accessor(sim.updatedCells, cell);
    sim.ClearCell(accessor);
    PushChangeAndFlag(sim, events, cell);
}
