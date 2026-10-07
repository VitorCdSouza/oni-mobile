// Melting, freezing, condensing and boiling of a single cell.
#include "sim_common.h"

namespace {

// Moves part of the cell's mass and disease into an ore that the game spawns as a debris item.
// Nothing is emitted for border cells outside the game grid.
void EmitTransitionOre(SimData &sim, SimEvents &events, CellSOA &soa, int cell, uint16_t ore_elem,
                       float conversion, float ore_mass, float temperature) {
    int disease_count = soa.diseaseCount[cell];
    uint8_t disease_idx = soa.diseaseIdx[cell];
    SIM_ASSERT(temperature > 0.0f, 0x7b, "SimEvents.h");
    int game_cell = ToGameCell(sim, cell);
    if (!IsValidGameCell(sim, game_cell)) {
        return;
    }
    int ore_disease = static_cast<int>(conversion * static_cast<float>(disease_count));
    SpawnOreInfo info{};
    info.cellIdx = game_cell;
    info.elemIdx = ore_elem;
    info.diseaseIdx = disease_idx;
    info.mass = ore_mass;
    info.temperature = temperature;
    info.diseaseCount = ore_disease;
    events.spawnOre.push_back(info);
    soa.mass[cell] -= ore_mass;
    soa.diseaseCount[cell] -= ore_disease;
    if (soa.diseaseCount[cell] < 1) {
        ClearDisease(soa, cell);
    }
}

}  // namespace

// Returns 1 when the cell changed element.
int DoStateTransition(SimData &sim, SimEvents &events, int cell, const ElementTemperatureData &elem) {
    CellSOA &soa = *sim.updatedCells;
    float mass = soa.mass[cell];
    if (mass <= 0.0f) {
        return 0;
    }
    float temperature = soa.temperature[cell];
    float new_temperature = 0.0f;
    int game_cell;

    if (elem.lowTemp - 3.0f <= temperature || elem.lowTempTransitionIdx == kInvalidElement) {
        // Heating past the high transition point.
        if (temperature <= elem.highTemp + 3.0f) {
            return 0;
        }
        uint16_t target = elem.highTempTransitionIdx;
        if (target == kInvalidElement) {
            return 0;
        }
        if (1.5f < temperature) {
            new_temperature = temperature - 1.5f;
        }
        soa.temperature[cell] = new_temperature;
        soa.elementIdx[cell] = target;
        uint16_t ore = elem.highTempTransitionOreIdx;
        if (ore != kInvalidElement) {
            float conversion = elem.highTempTransitionOreMassConversion;
            float ore_mass = mass * conversion;
            if (!(ore_mass <= 0.001f || ore_mass <= 0.0f)) {
                EmitTransitionOre(sim, events, soa, cell, ore, conversion, ore_mass, new_temperature);
            }
        }
        if (soa.properties[cell] & 0x40) {  // NotifyOnMelt
            game_cell = ToGameCell(sim, cell);
            if (IsValidGameCell(sim, game_cell)) {
                events.cellMelted.push_back(CellMeltedInfo{game_cell});
            }
            sim.unk_0x68[cell] |= 0x1f;
        }
        game_cell = ToGameCell(sim, cell);
    } else {
        // Cooling past the low transition point.
        uint16_t target = elem.lowTempTransitionIdx;
        if (-1.5f < temperature) {
            new_temperature = temperature + 1.5f;
        }
        soa.temperature[cell] = new_temperature;
        uint16_t ore = elem.lowTempTransitionOreIdx;
        if (ore != kInvalidElement) {
            float conversion = elem.lowTempTransitionOreMassConversion;
            float ore_mass = conversion * mass;
            if (0.001f < ore_mass && 0.0f < ore_mass) {
                EmitTransitionOre(sim, events, soa, cell, ore, conversion, ore_mass, new_temperature);
                mass = soa.mass[cell];
            }
        }

        bool cleared = false;
        if (StateOf(gElementTemperatureData[target].state) == kSolid) {
            // Freezing into a solid: small amounts become a debris item instead of a tile.
            if (0.8f < mass / gElementTemperatureData[target].defaultMass || mass <= 0.0f) {
                game_cell = ToGameCell(sim, cell);
            } else {
                uint8_t always_spawn = sim.unk_0xa8;
                int disease_count = soa.diseaseCount[cell];
                uint8_t disease_idx = soa.diseaseIdx[cell];
                float cell_temperature = soa.temperature[cell];
                SIM_ASSERT(cell_temperature > 0.0f, 0x7b, "SimEvents.h");
                game_cell = ToGameCell(sim, cell);
                if (IsValidGameCell(sim, game_cell) && (always_spawn != 0 || sim.unk_0x88[game_cell] != 0)) {
                    SpawnOreInfo info{};
                    info.cellIdx = game_cell;
                    info.elemIdx = target;
                    info.diseaseIdx = disease_idx;
                    info.mass = mass;
                    info.temperature = cell_temperature;
                    info.diseaseCount = disease_count;
                    events.spawnOre.push_back(info);
                    cleared = true;
                }
            }
        } else {
            // Condensing into a liquid: falls as a droplet when the cell above lets liquid through.
            game_cell = ToGameCell(sim, cell);
            uint64_t above = static_cast<uint64_t>(static_cast<int64_t>(cell)) -
                             static_cast<uint64_t>(static_cast<int64_t>(sim.width));
            bool stays = sim.headless || (sim.cells->properties[above] & 2) != 0 ||
                         !IsValidGameCell(sim, game_cell) ||
                         (sim.unk_0x88[game_cell] == 0 && sim.unk_0xa8 == 0);
            float cell_temperature = 0.0f;
            if (!stays) {
                cell_temperature = soa.temperature[cell];
                stays = cell_temperature < gElements[target].lowTemp - 3.0f ||
                        gElements[target].highTemp + 3.0f < cell_temperature;
            }
            if (!stays) {
                SpawnFallingLiquidInfo info{};
                info.cellIdx = game_cell;
                info.elemIdx = target;
                info.diseaseIdx = soa.diseaseIdx[cell];
                info.mass = mass;
                info.temperature = cell_temperature;
                info.diseaseCount = soa.diseaseCount[cell];
                events.spawnFallingLiquid.push_back(info);
                cleared = true;
            }
        }
        if (cleared) {
            CellAccessor accessor = Accessor(sim.updatedCells, cell);
            sim.ClearCell(accessor);
            game_cell = ToGameCell(sim, cell);
        } else {
            soa.elementIdx[cell] = target;
        }
    }

    if (IsValidGameCell(sim, game_cell)) {
        PushSubstanceChange(events, game_cell);
    }
    sim.unk_0x68[cell] |= 0x1f;
    return 1;
}
