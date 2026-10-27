// Liquid flow, one cell at a time: down, left, right, up, then the diagonal swap that lets a gas
// bubble trapped under a liquid escape. Reads the previous state from sim.cells and writes the
// new state into sim.updatedCells.
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

void SwapAll(CellSOA &soa, uint64_t a, uint64_t b) {
    std::swap(soa.elementIdx[a], soa.elementIdx[b]);
    std::swap(soa.mass[a], soa.mass[b]);
    std::swap(soa.temperature[a], soa.temperature[b]);
    std::swap(soa.diseaseIdx[a], soa.diseaseIdx[b]);
    std::swap(soa.diseaseCount[a], soa.diseaseCount[b]);
    std::swap(soa.diseaseInfestationTickCount[a], soa.diseaseInfestationTickCount[b]);
    std::swap(soa.diseaseAccumulatedError[a], soa.diseaseAccumulatedError[b]);
}

// Empties `cell` in updatedCells and reports it, used when the liquid left the cell entirely.
void VacateCell(SimData &sim, SimEvents &events, uint64_t cell) {
    CellSOA &upd = *sim.updatedCells;
    upd.elementIdx[cell] = sim.vacuumElementIdx;
    upd.mass[cell] = 0.0f;
    upd.temperature[cell] = 0.0f;
    ClearDisease(upd, static_cast<int>(cell));
    PushChangeAndFlag(sim, events, static_cast<int>(cell));
}

// FUN_0016d1a0 in the original: moves `mass` of `element` (at the source cell's previous
// temperature) into `dst`, pushing a gas out first. Returns false when `dst` holds a different
// liquid or the gas cannot move. The unk_0x1a element swallows whatever flows into it.
bool MoveLiquid(float mass, SimData &sim, SimEvents &events, int src, uint16_t element, int dst,
                uint8_t disease_idx, int disease_count) {
    uint64_t d = static_cast<uint64_t>(static_cast<int64_t>(dst));
    CellSOA *upd = sim.updatedCells;
    float temperature = sim.cells->temperature[src];
    uint16_t current = upd->elementIdx[d];
    if (current == element) {
        AddMassAndUpdateTemperature(upd, upd, dst, mass, temperature, disease_idx, disease_count);
        return true;
    }
    if (current == sim.unk_0x1a) {
        upd->mass[d] = 0.0f;
        return true;
    }
    uint8_t state = StateOf(gElementLiquidData[current].state);
    if (state == kLiquid) {
        return false;
    }
    if (state == kGas) {
        if (!DisplaceGas(sim, events, upd, dst, current)) {
            return false;
        }
        upd = sim.updatedCells;
    }
    upd->elementIdx[d] = element;
    AddMassAndUpdateTemperature(upd, upd, dst, mass, temperature, disease_idx, disease_count);
    PushChangeAndFlag(sim, events, dst);
    return true;
}

// After a successful move: takes the moved mass and germs out of the source cell.
void TakeFromSource(SimData &sim, const CellSOA &old, uint64_t cell, float flow, int moved_disease,
                    float &mass_left, float Vector4f::*flow_slot) {
    CellSOA *upd = sim.updatedCells;
    if (upd->diseaseIdx[cell] == old.diseaseIdx[cell]) {
        upd->diseaseCount[cell] -= moved_disease;
        if (upd->diseaseCount[cell] < 1) {
            ClearDisease(*upd, static_cast<int>(cell));
            upd = sim.updatedCells;
        }
    }
    mass_left = mass_left - flow;
    upd->mass[cell] -= flow;
    sim.unk_0x60[cell].*flow_slot = flow + sim.unk_0x60[cell].*flow_slot;
}

// Whether the liquid in `cell` can visibly drip as a droplet spawned at `drop_cell`.
bool CanSpawnDroplet(const SimData &sim, int drop_cell, int temperature_cell, int &game_cell, float &temperature,
                     uint16_t &element) {
    const CellSOA &cells = *sim.cells;
    uint64_t w = static_cast<uint64_t>(static_cast<int64_t>(sim.width));
    if (cells.properties[static_cast<uint64_t>(static_cast<int64_t>(drop_cell)) - w] & 2) {
        return false;
    }
    game_cell = ToGameCell(sim, drop_cell);
    if (!IsValidGameCell(sim, game_cell) || (sim.unk_0x88[game_cell] == 0 && sim.unk_0xa8 == 0)) {
        return false;
    }
    temperature = cells.temperature[temperature_cell];
    element = cells.elementIdx[temperature_cell];
    return !(temperature < gElements[element].lowTemp - 3.0f || gElements[element].highTemp + 3.0f < temperature);
}

void PushDroplet(SimEvents &events, int game_cell, uint16_t element, uint8_t disease_idx, float mass,
                 float temperature, int disease_count) {
    SpawnFallingLiquidInfo info{};
    info.cellIdx = game_cell;
    info.elemIdx = element;
    info.diseaseIdx = disease_idx;
    info.mass = mass;
    info.temperature = temperature;
    info.diseaseCount = disease_count;
    events.spawnFallingLiquid.push_back(info);
}

// One sideways step (left or right). Returns false when the cell ran dry.
bool FlowSideways(SimData &sim, SimEvents &events, const CellSOA &old, int cell, uint16_t element, int side,
                  float &mass, int &disease, float Vector4f::*flow_slot) {
    const ElementLiquidData &liquid = gElementLiquidData[element];
    const CellSOA &cells = *sim.cells;
    const CellSOA &upd = *sim.updatedCells;
    uint64_t s = static_cast<uint64_t>(static_cast<int64_t>(side));
    uint16_t side_element = cells.elementIdx[s];
    if (!(side_element == element || (gElementLiquidData[side_element].state & 2) == 0) ||
        StateOf(gElementLiquidData[upd.elementIdx[s]].state) == kSolid || (cells.properties[side] & 2) != 0) {
        return true;
    }
    float available = mass;
    if (StateOf(gElementLiquidData[side_element].state) == kLiquid) {
        available = mass - cells.mass[s];
    }
    float viscosity = liquid.viscosity;
    if (mass <= liquid.viscosity) {
        viscosity = mass;
    }
    float flow = available * 0.25f;
    if (viscosity <= available * 0.25f) {
        flow = viscosity;
    }
    if (!(liquid.minHorizontalFlow <= flow && 0.0f < flow)) {
        return true;
    }
    int below = cell - sim.width;
    int moved_disease = TruncToInt(static_cast<float>(disease) * (flow / mass));
    uint8_t disease_idx = old.diseaseIdx[cell];
    bool moved = false;
    if (StateOf(gElementLiquidData[cells.elementIdx[below]].state) == kSolid || (cells.properties[below] & 2) != 0) {
        // Standing on a floor: liquid going over an open edge drips down as a droplet.
        bool open_below = IsLiquidPermeable(sim.updatedCells, static_cast<unsigned long>(
                                                                   static_cast<long>(side - sim.width)));
        int game_cell;
        float temperature;
        uint16_t drop_element;
        if (open_below && !sim.headless && CanSpawnDroplet(sim, side, cell, game_cell, temperature, drop_element)) {
            PushDroplet(events, game_cell, drop_element, disease_idx, flow, temperature, moved_disease);
            moved = true;
        }
    }
    if (!moved && !MoveLiquid(flow, sim, events, cell, element, side, disease_idx, moved_disease)) {
        return true;
    }
    TakeFromSource(sim, old, static_cast<uint64_t>(static_cast<int64_t>(cell)), flow, moved_disease, mass, flow_slot);
    if (mass <= 0.0f) {
        return false;
    }
    disease -= moved_disease;
    return true;
}

}  // namespace

// 1 when the cell can take liquid: not liquid-impermeable and holding a gas or vacuum.
uint8_t IsLiquidPermeable(const CellSOA *soa, unsigned long cell) {
    if (soa->properties[cell] & 2) {
        return 0;
    }
    return static_cast<uint8_t>(((gElementLiquidData[soa->elementIdx[cell]].state >> 1) ^ 1) & 1);
}

void UpdateLiquid(SimData &sim, SimEvents &events, int cell) {
    const CellSOA &old = *sim.cells;
    const uint64_t c = static_cast<uint64_t>(static_cast<int64_t>(cell));
    const int width = sim.width;
    const uint16_t element = old.elementIdx[c];
    const ElementLiquidData &liquid = gElementLiquidData[element];
    const uint64_t below = static_cast<uint64_t>(static_cast<int64_t>(cell - width));
    float mass = old.mass[c];
    int disease = old.diseaseCount[c];

    // Down.
    if (StateOf(gElementLiquidData[sim.updatedCells->elementIdx[below]].state) != kSolid &&
        (old.properties[below] & 2) == 0) {
        uint16_t below_element = old.elementIdx[below];
        if (below_element == element) {
            float below_mass = 0.0f;
            if (StateOf(gElementLiquidData[below_element].state) == kLiquid) {
                below_mass = old.mass[below];
            }
            float capacity = mass * 1.01f;
            if (mass * 1.01f <= liquid.maxMass) {
                capacity = liquid.maxMass;
            }
            float viscosity = liquid.viscosity;
            if (mass <= liquid.viscosity) {
                viscosity = mass;
            }
            float room = capacity - below_mass;
            if (capacity - below_mass <= 0.0f) {
                room = 0.0f;
            }
            float flow = room * 0.5f;
            if (viscosity <= room * 0.5f) {
                flow = viscosity;
            }
            if ((mass <= liquid.minVerticalFlow || liquid.minVerticalFlow <= flow) && 0.0f < flow) {
                int moved_disease = TruncToInt(static_cast<float>(disease) * (flow / mass));
                if (MoveLiquid(flow, sim, events, cell, element, cell - width, old.diseaseIdx[c], moved_disease)) {
                    disease -= moved_disease;
                    TakeFromSource(sim, old, c, flow, moved_disease, mass, &Vector4f::z);
                }
            }
        } else if ((gElementLiquidData[below_element].state & 2) == 0) {
            // A gas or vacuum below.
            if (below_element == sim.unk_0x1a) {
                VacateCell(sim, events, c);
                return;
            }
            CellSOA *upd = sim.updatedCells;
            bool left_ok = !IsLiquidPermeable(upd, static_cast<unsigned long>(static_cast<long>(cell - 1))) ||
                           IsLiquidPermeable(sim.updatedCells, static_cast<unsigned long>(static_cast<long>(cell - 1 - width)));
            bool right_ok = left_ok &&
                            (!IsLiquidPermeable(sim.updatedCells, static_cast<unsigned long>(static_cast<long>(cell + 1))) ||
                             IsLiquidPermeable(sim.updatedCells, static_cast<unsigned long>(static_cast<long>(cell + 1 - width))));
            if (right_ok) {
                // Nothing to spread over: the liquid and the gas below trade places.
                SwapAll(*sim.updatedCells, c, below);
                PushChangeAndFlag(sim, events, cell);
                PushChangeAndFlag(sim, events, static_cast<int>(below));
                return;
            }
            if (sim.headless) {
                return;
            }
            int game_cell;
            float temperature;
            uint16_t drop_element;
            if (!CanSpawnDroplet(sim, cell, cell, game_cell, temperature, drop_element)) {
                return;
            }
            PushDroplet(events, game_cell, drop_element, old.diseaseIdx[c], mass, temperature, disease);
            VacateCell(sim, events, c);
            return;
        }
    }
    if (mass <= 0.0f) {
        return;
    }

    // Left and right.
    if (!FlowSideways(sim, events, old, cell, element, cell - 1, mass, disease, &Vector4f::x)) {
        return;
    }
    if (!FlowSideways(sim, events, old, cell, element, cell + 1, mass, disease, &Vector4f::y)) {
        return;
    }

    // Up, only for what exceeds what the cell above can hold.
    {
        const CellSOA &cells = *sim.cells;
        int up = sim.width + cell;
        uint16_t up_element = cells.elementIdx[up];
        if ((up_element == element || (gElementLiquidData[up_element].state & 2) == 0) &&
            StateOf(gElementLiquidData[sim.updatedCells->elementIdx[up]].state) != kSolid &&
            (cells.properties[up] & 2) == 0) {
            float up_mass = 0.0f;
            if (StateOf(gElementLiquidData[up_element].state) == kLiquid) {
                up_mass = cells.mass[up] * 1.01f;
            }
            if (up_mass <= liquid.maxMass) {
                up_mass = liquid.maxMass;
            }
            float excess = mass - up_mass;
            if (mass - up_mass <= 0.0f) {
                excess = 0.0f;
            }
            float flow = excess * 0.5f;
            if (mass < up_mass + up_mass) {
                float viscosity = liquid.viscosity;
                if (mass <= liquid.viscosity) {
                    viscosity = mass;
                }
                if (viscosity <= flow) {
                    flow = viscosity;
                }
            }
            if (0.01f < flow) {
                int moved_disease = TruncToInt(static_cast<float>(disease) * (flow / mass));
                if (MoveLiquid(flow, sim, events, cell, element, up, old.diseaseIdx[c], moved_disease)) {
                    TakeFromSource(sim, old, c, flow, moved_disease, mass, &Vector4f::w);
                }
            }
            if (mass <= 0.0f) {
                return;
            }
        }
    }

    // A gas cell diagonally below, next to a blocked side, swaps up through the liquid.
    CellSOA &upd = *sim.updatedCells;
    const uint16_t *elements = upd.elementIdx.data();
    const uint8_t *properties = upd.properties.data();
    int down = cell - sim.width;
    auto state_of = [&](uint64_t i) { return StateOf(gElementLiquidData[elements[i]].state); };
    uint64_t other = static_cast<uint64_t>(static_cast<int64_t>(down + 1));
    bool right_swap = state_of(other) == kGas &&
                      (state_of(c + 1) == kSolid || (properties[cell + 1] & 1) != 0) &&
                      state_of(static_cast<uint64_t>(static_cast<int64_t>(down))) != kSolid &&
                      (properties[c] & 1) == 0 && (properties[other] & 2) == 0 && (properties[down] & 3) == 0;
    if (!right_swap) {
        other = static_cast<uint64_t>(static_cast<int64_t>(down - 1));
        if (state_of(other) != kGas) {
            return;
        }
        if (state_of(c - 1) != kSolid && (properties[cell - 1] & 1) == 0) {
            return;
        }
        if (state_of(static_cast<uint64_t>(static_cast<int64_t>(down))) == kSolid || (properties[c] & 1) != 0 ||
            (properties[other] & 2) != 0 || (properties[down] & 3) != 0) {
            return;
        }
    }
    SwapAll(upd, c, other);
    PushChangeAndFlag(sim, events, cell);
    PushChangeAndFlag(sim, events, static_cast<int>(other));
}
