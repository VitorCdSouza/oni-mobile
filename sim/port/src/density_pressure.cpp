// Fluids pushing each other around: heavier fluid sinking below a lighter one of the same state,
// and a fluid over its maximum mass breaking the solid tiles holding it.
#include <utility>

#include "sim_common.h"

namespace {

void PushChangeAndFlag(SimData &sim, SimEvents &events, int cell) {
    int game_cell = ToGameCell(sim, cell);
    if (IsValidGameCell(sim, game_cell)) {
        PushSubstanceChange(events, game_cell);
    }
    sim.unk_0x68[cell] |= 0x1f;
}

void SwapContents(CellSOA &soa, int a, int b, bool with_element) {
    if (with_element) {
        std::swap(soa.elementIdx[a], soa.elementIdx[b]);
    }
    std::swap(soa.mass[a], soa.mass[b]);
    std::swap(soa.temperature[a], soa.temperature[b]);
    std::swap(soa.diseaseIdx[a], soa.diseaseIdx[b]);
    std::swap(soa.diseaseCount[a], soa.diseaseCount[b]);
    std::swap(soa.diseaseInfestationTickCount[a], soa.diseaseInfestationTickCount[b]);
    std::swap(soa.diseaseAccumulatedError[a], soa.diseaseAccumulatedError[b]);
}

// How much pressure a solid tile withstands. strengthInfo: low 7 bits strength in quarters, top
// bit set for tiles whose strength scales with how full they are; the others get a 0.1 margin.
// The margin is returned separately because the original adds it last.
struct Resistance {
    float strength;
    float margin;
};

Resistance TileResistance(const CellSOA &soa, int cell, const ElementPostProcessData &elem) {
    uint8_t info = soa.strengthInfo[cell];
    float scaled = static_cast<float>(static_cast<int>(info) >> 7);
    float margin = 1.0f <= scaled ? 0.0f : 0x1.9999ap-4f;
    float base = static_cast<float>(info & 0x7f) * 0.25f;
    float fill = (1.0f - scaled) + (soa.mass[cell] / elem.maxMass) * scaled;
    return {fill * elem.strength * base, margin};
}

bool IsBreakableSolid(const CellSOA &soa, int cell, const ElementPostProcessData &elem) {
    return StateOf(elem.state) == kSolid && (elem.state & 4) == 0 && (soa.properties[cell] & 8) == 0;
}

void EmitWorldDamage(SimData &sim, SimEvents &events, int target, int source) {
    int game_target = ToGameCell(sim, target);
    if (game_target < 0) {
        return;
    }
    int game_source = ToGameCell(sim, source);
    if (IsValidGameCell(sim, game_source) && game_target < sim.numGameCells) {
        WorldDamageInfo info;
        info.gameCell = game_target;
        info.damageSourceOffset = game_source;
        events.worldDamage.push_back(info);
    }
}

}  // namespace

// Swaps `cell` with the cell below when both hold the same kind of matter and the one below is
// lighter (random chance above `threshold`). For the same element the colder cell sinks instead:
// liquids just even out their temperature, gases trade places.
bool DoDensityDisplacement(SimData &sim, SimEvents &events, int cell, const ElementPostProcessData &elem,
                           float threshold) {
    const Element *elements = gElements.data();
    CellSOA &soa = *sim.updatedCells;
    int below = cell - sim.width;
    uint16_t below_element = soa.elementIdx[below];
    if (elem.state != elements[below_element].state || !(threshold < SimRandom01(sim))) {
        return false;
    }
    uint16_t cell_element = soa.elementIdx[cell];
    if (elements[below_element].molarMass <= elem.molarMass && elem.molarMass != elements[below_element].molarMass) {
        SwapContents(soa, cell, below, true);
        PushChangeAndFlag(sim, events, cell);
        PushChangeAndFlag(sim, events, below);
        return true;
    }
    if (below_element != cell_element) {
        return false;
    }
    float below_temperature = soa.temperature[below];
    float cell_temperature = soa.temperature[cell];
    if (!(cell_temperature < below_temperature)) {
        return false;
    }
    if (StateOf(elem.state) == kLiquid) {
        float cell_mass = soa.mass[cell];
        float below_mass = soa.mass[below];
        float mixed = (cell_temperature * cell_mass + below_temperature * below_mass) / (cell_mass + below_mass);
        float result = !(mixed >= cell_temperature) ? cell_temperature : MinSS(below_temperature, mixed);
        soa.temperature[cell] = result;
        soa.temperature[below] = result;
        return true;
    }
    SwapContents(soa, cell, below, false);
    return true;
}

// Fluid at `pressure_ratio` times its maximum mass tries to break the one or two solid tiles in
// direction `step`; tiles get WorldDamage events when the pressure beats their combined
// resistance.
bool DoPressureBreak(SimData &sim, SimEvents &events, int cell, float pressure_ratio, int step) {
    CellSOA &soa = *sim.updatedCells;
    const uint16_t *elements = soa.elementIdx.data();
    int first = cell + step;
    const ElementPostProcessData &first_elem = gElementPostProcessData[elements[first]];
    if (!IsBreakableSolid(soa, first, first_elem)) {
        return false;
    }
    Resistance r1 = TileResistance(soa, first, first_elem);
    float resistance = (r1.strength + 1.0f) + r1.margin;
    if (pressure_ratio < resistance) {
        return false;
    }
    int blocks = 1;
    int beyond = first + step;
    const ElementPostProcessData *beyond_elem = &gElementPostProcessData[elements[beyond]];
    if (StateOf(beyond_elem->state) == kSolid) {
        if (!IsBreakableSolid(soa, beyond, *beyond_elem)) {
            return false;
        }
        Resistance r2 = TileResistance(soa, beyond, *beyond_elem);
        resistance = (resistance + r2.strength) + r2.margin;
        if (pressure_ratio < resistance) {
            return false;
        }
        beyond = beyond + step;
        beyond_elem = &gElementPostProcessData[elements[beyond]];
        if (StateOf(beyond_elem->state) == kSolid) {
            return false;
        }
        blocks = 2;
    }
    if (pressure_ratio <= resistance) {
        return false;
    }
    if (StateOf(beyond_elem->state) == kLiquid && 0.0f < soa.mass[beyond] &&
        pressure_ratio - soa.mass[beyond] / beyond_elem->maxMass <= resistance) {
        return false;  // the liquid on the other side pushes back
    }
    EmitWorldDamage(sim, events, first, cell);
    if (blocks != 1) {
        EmitWorldDamage(sim, events, first + step, cell);
    }
    return true;
}

// Tries the four directions: down, left, right, up.
bool DoPressureBreak(SimData &sim, SimEvents &events, int cell, float pressure_ratio) {
    return DoPressureBreak(sim, events, cell, pressure_ratio, -sim.width) ||
           DoPressureBreak(sim, events, cell, pressure_ratio, -1) ||
           DoPressureBreak(sim, events, cell, pressure_ratio, 1) ||
           DoPressureBreak(sim, events, cell, pressure_ratio, sim.width);
}
