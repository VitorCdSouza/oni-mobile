// Unstable solids that can also slide down diagonally, and the headless (world generation)
// shortcut that drops them straight down without spawning falling objects.
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

struct FallDirection {
    int offset;
    float fraction;         // share of the cell that falls this way
    float min_flow_factor;  // times minHorizontalFlow that must stay behind
    bool diagonal;          // also needs the cell above the target to be open
};

}  // namespace

// Swaps the cell above `start` down through every non-solid cell below it.
void HeadlessUnstableFallSimpleSwap(SimData &sim, SimEvents &events, CellAccessor &start) {
    int width = sim.width;
    for (int below = start.cell - width; below >= 0; below -= width) {
        CellSOA &soa = *sim.updatedCells;
        int above = below + width;
        if (StateOf(gElements[soa.elementIdx[below]].state) == kSolid) {
            PushChangeAndFlag(sim, events, above);
            return;
        }
        std::swap(soa.elementIdx[above], soa.elementIdx[below]);
        std::swap(soa.mass[above], soa.mass[below]);
        std::swap(soa.temperature[above], soa.temperature[below]);
        std::swap(soa.diseaseIdx[above], soa.diseaseIdx[below]);
        std::swap(soa.diseaseCount[above], soa.diseaseCount[below]);
        std::swap(soa.diseaseInfestationTickCount[above], soa.diseaseInfestationTickCount[below]);
        std::swap(soa.diseaseAccumulatedError[above], soa.diseaseAccumulatedError[below]);
        PushChangeAndFlag(sim, events, above);
        width = sim.width;
    }
}

void DoUnstableCheckWithDiagonals(SimData &sim, SimEvents &events, CellAccessor &self) {
    const ElementPostProcessData *pp = gElementPostProcessData.data();
    const int w = sim.width;
    const FallDirection directions[3] = {
        {-w, 1.0f, 0.0f, false},
        {-w - 1, 0.5f, 1.0f, true},
        {1 - w, 0.5f, 1.0f, true},
    };
    int cell = self.cell;
    float start_mass = self.soa->mass[cell];
    int start_disease = self.soa->diseaseCount[cell];
    uint16_t element = self.soa->elementIdx[cell];
    float mass = start_mass;
    uint8_t checked = 0;
    uint8_t ticks = 0xff;
    for (const FallDirection &d : directions) {
        float fraction = d.fraction;
        if (0.0f < (mass - d.min_flow_factor * pp[element].minHorizontalFlow) * fraction) {
            int target = cell + d.offset;
            const uint16_t *elements = sim.updatedCells->elementIdx.data();
            const uint8_t *properties = sim.updatedCells->properties.data();
            uint16_t target_element = elements[target];
            bool open = StateOf(gElementPostProcessData[target_element].state) != kSolid &&
                        sim.unk_0x1a != target_element && (properties[target] & 4) == 0;
            if (open && d.diagonal) {
                int above = sim.width + target;
                open = StateOf(gElementPostProcessData[elements[above]].state) != kSolid && (properties[above] & 4) == 0;
            }
            if (open) {
                if (!checked) {
                    ticks = sim.GetStableTicksRemaining(static_cast<unsigned long>(static_cast<long>(cell)));
                }
                checked = 1;
                if (ticks == 0) {
                    sim.unk_0x68[self.cell] |= 0x1f;
                    checked = sim.headless;
                    CellSOA &src = *self.soa;
                    int c = self.cell;
                    if (!sim.headless) {
                        // The game spawns a falling object with this share of the cell.
                        float fall_mass = fraction * start_mass;
                        int fall_disease = static_cast<int>(static_cast<float>(start_disease) * fraction);
                        UnstableCellInfo info{};
                        info.cellIdx = ToGameCell(sim, target);
                        info.elemIdx = src.elementIdx[c];
                        info.fallingInfo = 0;
                        info.diseaseIdx = src.diseaseIdx[c];
                        info.mass = fall_mass;
                        info.temperature = src.temperature[c];
                        info.diseaseCount = fall_disease;
                        events.unstableCells.push_back(info);
                        src.mass[c] -= fall_mass;
                        src.diseaseCount[c] -= fall_disease;
                        if (src.diseaseCount[self.cell] < 1) {
                            ClearDisease(*self.soa, self.cell);
                        }
                        PushChangeAndFlag(sim, events, self.cell);
                        if (self.soa->mass[self.cell] <= 0.0f) {
                            Evaporate(self.cell, sim, events);
                        }
                        checked = 1;
                    } else {
                        // Headless: move the share into the target cell directly.
                        CellAccessor dst = Accessor(sim.updatedCells, target);
                        uint8_t state = StateOf(gElementPostProcessData[target_element].state);
                        if (state == kGas) {
                            DisplaceGas(sim, events, dst.soa, target, dst.soa->elementIdx[target]);
                        } else if (state == kLiquid) {
                            DisplaceLiquid(sim, events, dst.soa, target, dst.soa->elementIdx[target]);
                        }
                        CellSOA &from = *self.soa;
                        int disease = from.diseaseCount[c];
                        float moved_mass = from.mass[c] * fraction;
                        dst.soa->elementIdx[target] = from.elementIdx[c];
                        dst.soa->mass[target] = moved_mass;
                        int moved_disease = static_cast<int>(static_cast<float>(disease) * fraction);
                        dst.soa->temperature[target] = from.temperature[c];
                        dst.soa->diseaseIdx[target] = from.diseaseIdx[c];
                        dst.soa->diseaseCount[target] = moved_disease;
                        PushChangeAndFlag(sim, events, target);
                        from.mass[self.cell] -= moved_mass;
                        from.diseaseCount[self.cell] -= moved_disease;
                        if (from.diseaseCount[self.cell] < 1) {
                            ClearDisease(*self.soa, self.cell);
                        }
                        PushChangeAndFlag(sim, events, self.cell);
                        if (self.soa->mass[self.cell] <= 0x1p-126f) {
                            Evaporate(self.cell, sim, events);
                        }
                        HeadlessUnstableFallSimpleSwap(sim, events, dst);
                    }
                }
            }
        }
        cell = self.cell;
        mass = self.soa->mass[cell];
    }
    if (!checked) {
        sim.unk_0x68[self.cell] |= 0x1f;
    }
}
