// Gas pressure equalisation between neighbouring cells of the same element (or into vacuum),
// and pressure-driven displacement of a different gas or liquid. From the disassembly.
#include <utility>

#include "sim_common.h"

// Moves gas between `cell` (element `element`, state `state`) and `other`, proportional to the
// mass difference. Returns the signed flow (positive: from cell to other).
float UpdatePressure(SimData &sim, SimEvents &events, const CellSOA *old, CellSOA *upd, int cell, uint16_t element,
                     signed char state, float flow, int other) {
    uint16_t other_element = old->elementIdx[other];
    const ElementPressureData &pd = gElementPressureData[other_element];
    float cell_mass = old->mass[cell];
    float other_mass = old->mass[other];
    if (state != 1) {
        flow = pd.flow;
    }
    float f = flow * (cell_mass - other_mass);
    f = MinSS(f, 0.125f * cell_mass);
    f = MaxSS(f, -0.125f * other_mass);

    int src, dst;
    float amount;
    uint8_t dst_state;
    uint16_t written_element, dst_element;
    if (0.0f > f) {
        src = other;
        dst = cell;
        amount = MinSS(-f, other_mass);
        dst_state = static_cast<uint8_t>(state);
        written_element = other_element;
        dst_element = element;
    } else {
        src = cell;
        dst = other;
        amount = MinSS(f, cell_mass);
        dst_state = StateOf(pd.state);
        written_element = element;
        dst_element = other_element;
    }
    float src_mass = src == cell ? cell_mass : other_mass;
    float ratio = MaxSS(MinSS(amount / src_mass, 1.0f), 0.0f);
    int moved_disease = static_cast<int>(static_cast<float>(old->diseaseCount[src]) * ratio);
    uint8_t disease_idx = upd->diseaseIdx[src];
    float src_temperature = upd->temperature[src];
    float dst_temperature = upd->temperature[dst];
    float dst_mass = upd->mass[dst];
    float total = dst_mass + amount;
    float temperature;
    if (0.0f >= total) {
        temperature = 0.0f;
    } else {
        float mixed = (dst_mass * dst_temperature + src_temperature * amount) / total;
        if (dst_temperature > src_temperature) {
            temperature = mixed < src_temperature ? src_temperature : MinSS(dst_temperature, mixed);
        } else {
            temperature = dst_temperature <= mixed ? MinSS(src_temperature, mixed) : dst_temperature;
        }
    }
    upd->mass[dst] = total;
    upd->temperature[dst] = temperature;
    gDisease->AddDiseaseToCell(upd, dst, disease_idx, moved_disease);
    upd->mass[src] = MaxSS(upd->mass[src] - amount, 0.0f);
    upd->diseaseCount[src] -= moved_disease;
    if (!(upd->diseaseCount[src] > 0)) {
        ClearDisease(*upd, src);
    }
    if (dst_state != 0) {
        return f;
    }
    // The destination was vacuum: it now holds the incoming gas (or nothing, for unk_0x1a).
    if (sim.unk_0x1a == dst_element) {
        upd->mass[dst] = 0.0f;
        upd->temperature[dst] = 0.0f;
        ClearDisease(*upd, dst);
        return f;
    }
    upd->elementIdx[dst] = written_element;
    int game_cell = ToGameCell(sim, dst);
    if (IsValidGameCell(sim, game_cell)) {
        PushSubstanceChange(events, game_cell);
    }
    sim.unk_0x68[dst] |= 0x1f;
    return f;
}

namespace {

void PushChangeAndFlag(SimData &sim, SimEvents &events, int cell) {
    int game_cell = ToGameCell(sim, cell);
    if (IsValidGameCell(sim, game_cell)) {
        PushSubstanceChange(events, game_cell);
    }
    sim.unk_0x68[cell] |= 0x1f;
}

void SwapAll(CellSOA &soa, int a, int b) {
    std::swap(soa.elementIdx[a], soa.elementIdx[b]);
    std::swap(soa.mass[a], soa.mass[b]);
    std::swap(soa.temperature[a], soa.temperature[b]);
    std::swap(soa.diseaseIdx[a], soa.diseaseIdx[b]);
    std::swap(soa.diseaseCount[a], soa.diseaseCount[b]);
    std::swap(soa.diseaseInfestationTickCount[a], soa.diseaseInfestationTickCount[b]);
    std::swap(soa.diseaseAccumulatedError[a], soa.diseaseAccumulatedError[b]);
}

// Shared tail of the two pressure displacements: `amount` of `element` at the source's previous
// temperature goes into `dst`, with 1/8 of the source's germs.
float PushIntoCell(SimData &sim, uint16_t element, int src, int dst, float amount) {
    CellSOA &upd = *sim.updatedCells;
    upd.mass[dst] = upd.mass[dst] + amount;
    const CellSOA &cells = *sim.cells;
    upd.temperature[dst] = cells.temperature[src];
    upd.elementIdx[dst] = element;
    PushChangeAndFlag(sim, *sim.events, dst);
    upd.mass[src] = MaxSS(upd.mass[src] - amount, 0.0f);
    int moved_disease = static_cast<int>(static_cast<float>(cells.diseaseCount[src]) * 0.125f);
    gDisease->AddDiseaseToCell(sim.updatedCells, dst, cells.diseaseIdx[src], moved_disease);
    CellSOA &after = *sim.updatedCells;
    after.diseaseCount[src] -= moved_disease;
    if (!(after.diseaseCount[src] > 0)) {
        ClearDisease(after, src);
    }
    return amount;
}

}  // namespace

// Moves the gas in `from` one cell further (to `to`) when `to` holds the same gas or vacuum.
uint64_t DisplaceGasDirectional(SimData &sim, SimEvents &events, int from, int to, uint16_t element) {
    CellSOA *upd = sim.updatedCells;
    if (!(0.0f < upd->mass[from])) {
        return 0;
    }
    uint16_t target = upd->elementIdx[to];
    if ((target == element || target == sim.vacuumElementIdx) && (upd->properties[to] & 1) == 0) {
        DoDisplacement(sim, events, upd, from, to);
        return 1;
    }
    return 0;
}

// A heavier gas in `cell` pushes the different gas in `neighbour` on into `beyond` and takes 1/8
// of its own mass there. Returns the mass moved.
float DoGasPressureDisplacement(uint16_t element, int cell, int neighbour, int beyond, SimData &sim) {
    const CellSOA &cells = *sim.cells;
    uint16_t neighbour_element = cells.elementIdx[neighbour];
    if (StateOf(gElementPressureData[neighbour_element].state) != kGas) {
        return 0.0f;
    }
    float mass = cells.mass[cell];
    if (!(mass > cells.mass[neighbour])) {
        return 0.0f;
    }
    const CellSOA &upd = *sim.updatedCells;
    if (upd.elementIdx[cell] != element || upd.elementIdx[neighbour] != neighbour_element) {
        return 0.0f;
    }
    if (!static_cast<char>(DisplaceGasDirectional(sim, *sim.events, neighbour, beyond, neighbour_element))) {
        return 0.0f;
    }
    float amount = MinSS(mass * 0.125f, sim.updatedCells->mass[cell]);
    return PushIntoCell(sim, element, cell, neighbour, amount);
}

// Moves the liquid in `from` into `to`: swapping with a gas there (after pushing it away), or
// merging into the same liquid.
uint64_t DisplaceLiquidDirectional(SimData &sim, SimEvents &events, int from, int to, uint16_t element) {
    (void)element;
    CellSOA *upd = sim.updatedCells;
    float mass = upd->mass[from];
    if (!(0.0f < mass) || (upd->properties[to] & 2) != 0) {
        return 0;
    }
    uint16_t target = upd->elementIdx[to];
    CellAccessor source = Accessor(upd, from);
    if ((gElementLiquidData[target].state & 2) == 0) {
        bool displaced = DisplaceGas(sim, events, upd, to, target);
        if (!displaced) {
            return 0;
        }
        if (IsNonZero(upd->mass[to])) {
            AssertFunc("BREAKPT:", 0x1ac, "UpdateLiquid.cpp");
            KCrashReporterReportMessage("UpdateLiquid.cpp", 0x1ac, "Assert failed: dest_cell.mass() == 0.0f");
        }
        SwapAll(*sim.updatedCells, from, to);
        PushChangeAndFlag(sim, events, from);
        PushChangeAndFlag(sim, events, to);
        return displaced;
    }
    if (StateOf(gElementLiquidData[target].state) == kLiquid && target == upd->elementIdx[from]) {
        AddMassAndUpdateTemperature(upd, upd, to, mass, upd->temperature[from], upd->diseaseIdx[from],
                                    upd->diseaseCount[from]);
        sim.ClearCell(source);
        PushChangeAndFlag(sim, events, from);
        return 1;
    }
    return 0;
}

// A liquid with more mass (net of what already flowed this tick) than `neighbour` and `beyond`
// together pushes the different liquid in `neighbour` on and takes 1/8 of the excess.
float DoLiquidPressureDisplacement(uint16_t element, int cell, int neighbour, int beyond, SimData &sim) {
    const CellSOA &upd = *sim.updatedCells;
    if (upd.properties[neighbour] & 2) {
        return 0.0f;
    }
    const CellSOA &cells = *sim.cells;
    uint16_t neighbour_element = cells.elementIdx[neighbour];
    if (neighbour_element == element || upd.elementIdx[cell] != element ||
        upd.elementIdx[neighbour] != neighbour_element) {
        return 0.0f;
    }
    if (StateOf(gElementPressureData[neighbour_element].state) != kLiquid) {
        return 0.0f;
    }
    const Vector4f &flowed = sim.unk_0x60[cell];
    float available = ((cells.mass[cell] - flowed.x) - (flowed.y + flowed.z)) - flowed.w;
    if (!(available > cells.mass[beyond] + cells.mass[neighbour])) {
        return 0.0f;
    }
    if (!static_cast<char>(DisplaceLiquidDirectional(sim, *sim.events, neighbour, beyond, neighbour_element))) {
        return 0.0f;
    }
    return PushIntoCell(sim, element, cell, neighbour, available * 0.125f);
}
