// Solids melting 5 kg at a time from a hot gas next to them, and unsupported unstable solids
// (sand, dirt...) starting to fall.
#include "sim_common.h"

// The hot gas or vacuum in `cell` melts a chunk of the solid tile `solid`. Non-zero when it did.
uint64_t DoPartialMelt(SimData &sim, SimEvents &events, int cell, int solid) {
    const Element *e = gElements.data();
    CellSOA &soa = *sim.updatedCells;
    uint16_t element = soa.elementIdx[cell];
    if (e[element].state & 2) {
        return 0;  // only gases and vacuum melt their neighbours
    }
    uint16_t solid_element = soa.elementIdx[solid];
    if (StateOf(e[solid_element].state) != kSolid) {
        return 0;
    }
    if (!(5.0f < soa.mass[solid]) || (soa.properties[solid] & 0x48) != 0) {
        return 0;
    }
    float melt_temperature = e[solid_element].highTemp + 3.0f;
    float temperature = soa.temperature[cell];
    if (!(melt_temperature < temperature && soa.temperature[solid] < e[solid_element].highTemp - 3.0f)) {
        return 0;
    }
    uint16_t liquid = e[solid_element].highTempTransitionIdx;
    if (liquid == kInvalidElement) {
        return 0;
    }
    float capacity = soa.mass[cell] * e[element].specificHeatCapacity;
    float energy_needed = (melt_temperature - soa.temperature[solid]) * 5.0f * e[liquid].specificHeatCapacity;
    if (!(energy_needed <= ((temperature - 6.0f) - e[element].lowTemp) * capacity) ||
        StateOf(e[liquid].state) != kLiquid) {
        return 0;
    }
    if (!sim.headless) {
        uint64_t w = static_cast<uint64_t>(static_cast<int64_t>(sim.width));
        int game_cell;
        if ((sim.cells->properties[static_cast<uint64_t>(static_cast<int64_t>(cell)) - w] & 2) == 0 &&
            (game_cell = ToGameCell(sim, cell), IsValidGameCell(sim, game_cell))) {
            uint8_t visible = static_cast<uint8_t>(sim.unk_0x88[game_cell] != 0) | sim.unk_0xa8;
            if (visible != 0 && e[liquid].lowTemp - 3.0f <= melt_temperature &&
                melt_temperature <= e[liquid].highTemp + 3.0f) {
                // The molten chunk drips out as a droplet.
                int moved_disease =
                    static_cast<int>(static_cast<float>(soa.diseaseCount[solid]) * (5.0f / soa.mass[solid]));
                SpawnFallingLiquidInfo info{};
                info.cellIdx = game_cell;
                info.elemIdx = liquid;
                info.diseaseIdx = soa.diseaseIdx[solid];
                info.mass = 5.0f;
                info.temperature = melt_temperature;
                info.diseaseCount = moved_disease;
                events.spawnFallingLiquid.push_back(info);
                soa.temperature[cell] = (temperature * capacity - energy_needed) / capacity;
                float solid_mass = soa.mass[solid];
                soa.mass[solid] = solid_mass - 5.0f;
                soa.diseaseCount[solid] -= moved_disease;
                if (!(5.0f < solid_mass)) {
                    Evaporate(solid, sim, events);
                }
                return visible;
            }
        }
    }
    if (!DisplaceGas(sim, events, &soa, cell, element)) {
        return 0;
    }
    // The gas moved away: the cell itself becomes the molten chunk.
    soa.mass[cell] = 5.0f;
    soa.temperature[cell] = melt_temperature;
    soa.elementIdx[cell] = e[solid_element].highTempTransitionIdx;
    float solid_mass = soa.mass[solid];
    soa.mass[solid] = solid_mass - 5.0f;
    if (solid_mass <= 5.0f) {
        Evaporate(solid, sim, events);
    }
    events.ChangeSubstance(sim, static_cast<unsigned long>(static_cast<long>(cell)));
    return 1;
}

// Tries the four neighbours: below, left, right, above.
uint64_t DoPartialMelt(SimData &sim, SimEvents &events, int cell) {
    if (static_cast<char>(DoPartialMelt(sim, events, cell, cell - sim.width)) ||
        static_cast<char>(DoPartialMelt(sim, events, cell, cell - 1)) ||
        static_cast<char>(DoPartialMelt(sim, events, cell, cell + 1))) {
        return 1;
    }
    return DoPartialMelt(sim, events, cell, sim.width + cell);
}

// An unstable solid with nothing solid below starts falling once its stable ticks run out.
void DoUnstableCheckBasic(SimData &sim, SimEvents &events, CellAccessor &self) {
    int cell = self.cell;
    int below = cell - sim.width;
    uint16_t below_element = sim.updatedCells->elementIdx[below];
    if (StateOf(gElements[below_element].state) == kSolid || gElements[below_element].id == kSimHashVoid ||
        (sim.updatedCells->properties[below] & 4) != 0) {
        sim.unk_0x68[cell] |= 0x1f;
        return;
    }
    if (sim.GetStableTicksRemaining(static_cast<unsigned long>(static_cast<long>(cell))) != 0) {
        return;
    }
    if (sim.headless) {
        HeadlessUnstableFallSimpleSwap(sim, events, self);
        return;
    }
    cell = self.cell;
    CellSOA &soa = *self.soa;
    UnstableCellInfo info{};
    info.cellIdx = ToGameCell(sim, cell);
    info.elemIdx = soa.elementIdx[cell];
    info.fallingInfo = 0;
    info.diseaseIdx = soa.diseaseIdx[cell];
    info.mass = soa.mass[cell];
    info.temperature = soa.temperature[cell];
    info.diseaseCount = soa.diseaseCount[cell];
    events.unstableCells.push_back(info);
    Evaporate(self.cell, sim, events);
}
