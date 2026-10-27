// A liquid boiling 5 kg at a time when a much hotter neighbour heats it past its boiling point,
// instead of waiting for the whole cell to reach the transition temperature.
#include "sim_common.h"

namespace {

constexpr float kChunk = 5.0f;

void SetCell(CellSOA &soa, int cell, float mass, float temperature, uint16_t element) {
    soa.mass[cell] = mass;
    soa.temperature[cell] = temperature;
    soa.elementIdx[cell] = element;
}

}  // namespace

// Heats `cell` from the neighbour `source`. The return value is what the caller tests: non-zero
// when a chunk transitioned.
uint64_t DoPartialHeatTransition(SimData &sim, SimEvents &events, int cell, int source) {
    const Element *elements = gElements.data();
    CellSOA &soa = *sim.updatedCells;
    float mass = soa.mass[cell];
    if (mass < kChunk) {
        return 0;
    }
    uint16_t element = soa.elementIdx[cell];
    float transition_temperature = elements[element].highTemp + 3.0f;
    if (elements[element].highTempTransitionIdx == kInvalidElement) {
        return 0;
    }
    float source_temperature = soa.temperature[source];
    if (source_temperature <= transition_temperature) {
        return 0;
    }
    if (source_temperature < elements[soa.elementIdx[source]].lowTemp + 13.0f) {
        return 0;
    }
    float temperature = soa.temperature[cell];
    if (elements[element].highTemp - 3.0f <= temperature) {
        return 0;
    }
    float energy_needed = elements[element].specificHeatCapacity * kChunk * (transition_temperature - temperature);
    float source_capacity = soa.mass[source] * elements[soa.elementIdx[source]].specificHeatCapacity;
    if (source_capacity * 10.0f < energy_needed) {
        return 0;
    }
    int disease = soa.diseaseCount[cell];
    uint8_t target_state = StateOf(elements[elements[element].highTempTransitionIdx].state);
    float new_source_temperature = (source_capacity * source_temperature - energy_needed) / source_capacity;
    int moved_disease = TruncToInt(static_cast<float>(disease) * (kChunk / mass));

    uint64_t result;
    int game_cell;
    if (target_state == kGas) {
        soa.mass[cell] = mass - kChunk;
        soa.diseaseCount[cell] = disease - moved_disease;
        CellSOA *current = &soa;
        if (disease - moved_disease < 1) {
            ClearDisease(soa, cell);
            current = sim.updatedCells;
        }
        int above = sim.width + cell;
        soa.temperature[source] = new_source_temperature;
        uint16_t above_element = current->elementIdx[above];
        if (StateOf(elements[above_element].state) == kGas) {
            // The steam goes into the gas cell above when that gas can be pushed away.
            bool displaced = DisplaceGas(sim, events, current, above, above_element);
            if (displaced) {
                SetCell(*current, above, kChunk, transition_temperature, elements[element].highTempTransitionIdx);
                int above_game_cell = ToGameCell(sim, above);
                if (IsValidGameCell(sim, above_game_cell)) {
                    PushSubstanceChange(events, above_game_cell);
                }
                sim.unk_0x68[above] |= 0x1f;
                return displaced;
            }
            current = sim.updatedCells;
        }
        // Otherwise the remaining liquid moves out and the cell itself becomes the steam.
        result = DisplaceLiquid(sim, events, current, cell, soa.elementIdx[cell]);
        if (!result) {
            goto restore;
        }
        SetCell(soa, cell, kChunk, transition_temperature, elements[element].highTempTransitionIdx);
        game_cell = ToGameCell(sim, cell);
        if (!IsValidGameCell(sim, game_cell)) {
            goto done;
        }
    } else {
        if (target_state != kLiquid) {
            SIM_ASSERT(false, 0x2da, "PostProcessCell.cpp");
            return 0;
        }
        soa.mass[cell] = mass - kChunk;
        soa.diseaseCount[cell] = disease - moved_disease;
        CellSOA *current = &soa;
        uint16_t displaced_element = element;
        if (disease - moved_disease < 1) {
            ClearDisease(soa, cell);
            current = sim.updatedCells;
            displaced_element = soa.elementIdx[cell];
        }
        soa.temperature[source] = new_source_temperature;
        result = DisplaceLiquid(sim, events, current, cell, displaced_element);
        if (!result) {
            goto restore;
        }
        uint16_t target = elements[element].highTempTransitionIdx;
        uint64_t w = static_cast<uint64_t>(static_cast<int64_t>(sim.width));
        game_cell = ToGameCell(sim, cell);
        if (!sim.headless &&
            (sim.cells->properties[static_cast<uint64_t>(static_cast<int64_t>(cell)) - w] & 2) == 0) {
            if (game_cell < 0) {
                SetCell(soa, cell, kChunk, transition_temperature, target);
                goto done;
            }
            if (game_cell < sim.numGameCells) {
                uint8_t visible = static_cast<uint8_t>(sim.unk_0x88[game_cell] != 0) | sim.unk_0xa8;
                if (visible != 0 && gElements[target].lowTemp - 3.0f <= transition_temperature &&
                    transition_temperature <= gElements[target].highTemp + 3.0f) {
                    // A liquid product falls as a droplet; the cell keeps what DisplaceLiquid left.
                    SpawnFallingLiquidInfo info{};
                    info.cellIdx = game_cell;
                    info.elemIdx = target;
                    info.diseaseIdx = soa.diseaseIdx[cell];
                    info.mass = kChunk;
                    info.temperature = transition_temperature;
                    info.diseaseCount = moved_disease;
                    events.spawnFallingLiquid.push_back(info);
                    return visible;
                }
            }
            SetCell(soa, cell, kChunk, transition_temperature, target);
        } else {
            SetCell(soa, cell, kChunk, transition_temperature, target);
            if (game_cell < 0) {
                goto done;
            }
        }
        if (sim.numGameCells <= game_cell) {
            goto done;
        }
    }
    PushSubstanceChange(events, game_cell);
done:
    sim.unk_0x68[cell] |= 0x1f;
    return result;

restore:
    // Nowhere to put the displaced liquid: undo the chunk.
    soa.mass[cell] += kChunk;
    soa.diseaseCount[cell] = moved_disease + soa.diseaseCount[cell];
    if (soa.diseaseCount[cell] < 1) {
        ClearDisease(soa, cell);
    }
    soa.temperature[source] = source_temperature;
    return result;
}

// Tries the four neighbours as heat sources: below, left, right, above.
uint64_t DoPartialHeatTransition(SimData &sim, SimEvents &events, int cell) {
    if (static_cast<char>(DoPartialHeatTransition(sim, events, cell, cell - sim.width)) ||
        static_cast<char>(DoPartialHeatTransition(sim, events, cell, cell - 1)) ||
        static_cast<char>(DoPartialHeatTransition(sim, events, cell, cell + 1))) {
        return 1;
    }
    return DoPartialHeatTransition(sim, events, cell, sim.width + cell);
}
