// Per-cell work after the main simulation step: evaporation, displacement by density, pressure,
// boiling, off-gassing, gas mixing, unstable solids and sublimation.
#include <cstring>
#include <utility>

#include "sim_common.h"

namespace {

// Moves `amount` kg from `src` into `dst` as the off-gas element of `elem` (scaled by its
// efficiency), mixing temperatures and optionally carrying disease along. This is a lambda in the
// original (FUN_0016f980); `transfer_disease` is false when the two cells carry different germs.
struct OffGas {
    const ElementPostProcessData &elem;
    SimEvents &events;
    SimData &sim;

    void Transfer(CellAccessor &src, CellAccessor &dst, float amount, bool transfer_disease) const {
        CellSOA &s = *src.soa;
        float src_mass = s.mass[src.cell];
        s.mass[src.cell] = src_mass - amount;
        int moved_disease = 0;
        if (transfer_disease) {
            int disease = s.diseaseCount[src.cell];
            moved_disease = static_cast<int>(static_cast<float>(disease) * (amount / src_mass));
            s.diseaseCount[src.cell] = disease - moved_disease;
            if (s.diseaseCount[src.cell] < 1) {
                ClearDisease(s, src.cell);
            }
            CellSOA &d = *dst.soa;
            if (d.diseaseIdx[dst.cell] != s.diseaseIdx[src.cell]) {
                ClearDisease(d, dst.cell);
                d.diseaseIdx[dst.cell] = s.diseaseIdx[src.cell];
            }
        }
        CellSOA &d = *dst.soa;
        float dst_mass;
        if (d.elementIdx[dst.cell] == elem.sublimateIndex) {
            dst_mass = d.mass[dst.cell];
        } else {
            d.elementIdx[dst.cell] = elem.sublimateIndex;
            int game_cell = ToGameCell(sim, dst.cell);
            if (IsValidGameCell(sim, game_cell)) {
                PushSubstanceChange(events, game_cell);
            }
            dst_mass = 0.0f;
            sim.unk_0x68[dst.cell] |= 0x1f;
            ClearDisease(d, dst.cell);
            d.mass[dst.cell] = 0.0f;
        }
        float added = amount * elem.sublimateEfficiency;
        float src_temperature = s.temperature[src.cell];
        float dst_temperature = d.temperature[dst.cell];
        float temperature;
        if (added + dst_mass <= 0.0f) {
            temperature = 0.0f;
        } else {
            // Mass-weighted mix, kept between the two input temperatures.
            float mixed = (dst_mass * dst_temperature + added * src_temperature) / (added + dst_mass);
            temperature = src_temperature;
            if (src_temperature < dst_temperature) {
                if (src_temperature <= mixed) {
                    temperature = dst_temperature;
                    if (mixed <= dst_temperature) {
                        temperature = mixed;
                    }
                }
            } else {
                if (mixed <= src_temperature) {
                    temperature = mixed;
                }
                if (!(dst_temperature <= mixed)) {
                    temperature = dst_temperature;
                }
            }
        }
        d.temperature[dst.cell] = temperature;
        d.mass[dst.cell] = added + d.mass[dst.cell];
        d.diseaseCount[dst.cell] += moved_disease;
        if (d.diseaseCount[dst.cell] < 1) {
            ClearDisease(d, dst.cell);
        }
    }
};

bool CarriesDisease(uint8_t src_disease, uint8_t dst_disease) {
    return src_disease == dst_disease || dst_disease == kNoDisease;
}

void EvaporateCell(SimData &sim, SimEvents &events, int cell) {
    std::memset(&sim.unk_0x60[cell], 0, sizeof(Vector4f));
    Evaporate(cell, sim, events);
}

bool IsHeavyGas(const float *mass, const uint16_t *elements, int cell) {
    return !(mass[cell] < 1.0f) && StateOf(gElementPostProcessData[elements[cell]].state) == kGas;
}

void SwapCells(CellSOA &soa, int a, int b) {
    std::swap(soa.elementIdx[a], soa.elementIdx[b]);
    std::swap(soa.mass[a], soa.mass[b]);
    std::swap(soa.temperature[a], soa.temperature[b]);
    std::swap(soa.diseaseIdx[a], soa.diseaseIdx[b]);
    std::swap(soa.diseaseCount[a], soa.diseaseCount[b]);
    std::swap(soa.diseaseInfestationTickCount[a], soa.diseaseInfestationTickCount[b]);
    std::swap(soa.diseaseAccumulatedError[a], soa.diseaseAccumulatedError[b]);
}

float NextRoll(SimData &sim) {
    sim.rngState = sim.rngState * 0x343fdu + 0x269ec3u;
    return static_cast<float>(static_cast<int>((sim.rngState >> 16) & 0x7fff));
}

// Liquid cells. Returns after any step that moved the liquid.
void PostProcessLiquid(SimData &sim, SimEvents &events, CellAccessor &self, const ElementPostProcessData &elem) {
    int cell = self.cell;
    CellSOA &soa = *self.soa;
    float mass = soa.mass[cell];
    if (mass <= 0.01f && mass != 0.01f) {
        EvaporateCell(sim, events, cell);
        return;
    }
    if (DoDensityDisplacement(sim, events, cell, elem, 0.3f)) {
        return;
    }
    float current = soa.mass[cell];
    if (elem.maxMass < current) {
        if (DoPressureBreak(sim, events, cell, current / elem.maxMass)) {
            return;
        }
        float max_mass = elem.maxMass;
        current = soa.mass[cell];
        if (max_mass * 1.5f < current) {
            // Overpressured: push half of it into a lighter liquid cell above.
            CellSOA *cur = sim.updatedCells;
            int above = sim.width + cell;
            float above_mass = cur->mass[above];
            uint16_t above_element;
            if (above_mass < current &&
                (above_element = cur->elementIdx[above], StateOf(gElementLiquidData[above_element].state) == kLiquid)) {
                above_mass = above_mass * 1.01f;
                float limit = max_mass;
                if (limit <= above_mass) {
                    limit = above_mass;
                }
                if (limit < current && DisplaceLiquid(sim, events, cur, above, above_element)) {
                    float total = soa.mass[cell];
                    float moved = total * 0x1.fd73e6p-2f;  // 1 / 2.01
                    cur->mass[above] = moved;
                    cur->temperature[above] = soa.temperature[cell];
                    cur->elementIdx[above] = soa.elementIdx[cell];
                    int disease = soa.diseaseCount[cell];
                    cur->diseaseIdx[above] = soa.diseaseIdx[cell];
                    int moved_disease = static_cast<int>(static_cast<float>(disease) * (moved / total));
                    cur->diseaseCount[above] = moved_disease;
                    cur->diseaseInfestationTickCount[above] = 0;
                    cur->diseaseAccumulatedError[above] = 0.0f;
                    events.ChangeSubstance(sim, static_cast<unsigned long>(static_cast<long>(above)));
                    soa.mass[cell] -= moved;
                    soa.diseaseCount[cell] -= moved_disease;
                    if (soa.diseaseCount[cell] > 0) {
                        return;
                    }
                    ClearDisease(soa, cell);
                    return;
                }
            }
        }
    }
    if (static_cast<char>(DoPartialHeatTransition(sim, events, cell))) {
        return;
    }

    // Off-gassing (polluted water into polluted oxygen and the like) into the cell above.
    CellSOA *cur = sim.updatedCells;
    const float *masses = cur->mass.data();
    int above = sim.width + cell;
    if (1.8f < masses[above] || masses[above] == 1.8f) {
        return;
    }
    uint16_t gas = elem.sublimateIndex;
    if (gas == kInvalidElement) {
        return;
    }
    const uint16_t *elements = cur->elementIdx.data();
    uint16_t above_element = elements[above];
    if (gElementPostProcessData[above_element].state & 2) {
        return;  // liquid or solid above
    }
    float roll = NextRoll(sim);
    if (cur->properties[above] & 1) {
        return;  // gas impermeable
    }
    if (elem.sublimateProbability <= roll * 0x1.0002p-15f) {
        return;
    }
    int below = cell - sim.width;
    mass = soa.mass[cell];
    float amount = MinSS(elem.offGasProbability * mass, 1.0f);
    OffGas off_gas{elem, events, sim};
    if (elements[below] != soa.elementIdx[cell] || masses[below] <= mass) {
        if (mass - amount < 0.01f) {
            CellAccessor src = self, dst = self;
            off_gas.Transfer(src, dst, mass, true);
            events.SpawnFX(sim, static_cast<unsigned long>(static_cast<long>(cell)),
                           static_cast<SpawnFXHashes::Type>(elem.sublimateFX), 0.0f);
            return;
        }
    } else {
        // The same liquid continues below: the gas comes out of the whole column.
        amount = MinSS(elem.offGasProbability * masses[below], 1.0f);
        if (mass - amount < 0.01f) {
            CellAccessor src = self, dst = self;
            off_gas.Transfer(src, dst, mass, true);
            if (0.0f < amount - mass) {
                uint8_t dst_disease = soa.diseaseIdx[cell];
                CellAccessor from = Accessor(cur, below), to = self;
                off_gas.Transfer(from, to, amount - mass, CarriesDisease(cur->diseaseIdx[below], dst_disease));
            }
            events.SpawnFX(sim, static_cast<unsigned long>(static_cast<long>(cell)),
                           static_cast<SpawnFXHashes::Type>(elem.sublimateFX), 0.0f);
            return;
        }
    }
    if (amount <= 0.0f) {
        return;
    }
    if (gas != above_element && above_element != sim.vacuumElementIdx &&
        !DisplaceGas(sim, events, cur, above, above_element)) {
        return;
    }
    float added = elem.sublimateEfficiency * amount;
    float room = 1.8f - cur->mass[above];
    if (room < added) {
        amount = amount * (room / added);
    }
    uint8_t dst_disease = cur->diseaseIdx[above];
    CellAccessor src = self, dst = Accessor(cur, above);
    off_gas.Transfer(src, dst, amount, CarriesDisease(soa.diseaseIdx[cell], dst_disease));
    sim.unk_0x58[cell] = amount + sim.unk_0x58[cell];
    events.SpawnFX(sim, static_cast<unsigned long>(static_cast<long>(cell)),
                   static_cast<SpawnFXHashes::Type>(elem.sublimateFX), 0.0f);
}

// Gas cells: evaporation of traces, density displacement, partial melting of neighbours and a
// random swap with a neighbouring gas that mixes gases over time.
void PostProcessGas(SimData &sim, SimEvents &events, CellAccessor &self, const ElementPostProcessData &elem) {
    int cell = self.cell;
    int direction = sim.gasDirection;
    sim.gasDirection = -direction;
    const float *mass = self.soa->mass.data();
    const uint16_t *elements = self.soa->elementIdx.data();
    if (1e-09f <= mass[cell]) {
        if (mass[cell] < 0.001f) {
            bool neighbour_heavy = IsHeavyGas(mass, elements, direction + cell) ||
                                   IsHeavyGas(mass, elements, cell - direction) ||
                                   IsHeavyGas(mass, elements, cell - sim.width) ||
                                   IsHeavyGas(mass, elements, sim.width + cell);
            if (neighbour_heavy) {
                EvaporateCell(sim, events, cell);
            }
        }
    } else {
        EvaporateCell(sim, events, cell);
    }

    if (DoDensityDisplacement(sim, events, cell, elem, 0.99f)) {
        return;
    }
    if ((self.soa->properties[cell] & 2) == 0 && static_cast<char>(DoPartialMelt(sim, events, cell))) {
        return;
    }
    if (!(29490.298828125f < NextRoll(sim))) {
        return;
    }
    const Element *e = gElements.data();
    CellSOA &soa = *sim.updatedCells;
    const float *temperature = soa.temperature.data();
    int side_a = sim.gasDirection + cell;
    int side_b = cell - sim.gasDirection;
    int below = cell - sim.width;
    // Swap with the coldest gas neighbour among those that pass a coin flip.
    int target = side_a;
    float best = temperature[side_a];
    if (3.4028235e+38f < best || NextRoll(sim) <= 16383.5f || StateOf(e[soa.elementIdx[side_a]].state) != kGas) {
        target = -1;
        best = 3.4028235e+38f;
    }
    if (temperature[side_b] <= best && 16383.5f < NextRoll(sim) &&
        StateOf(e[soa.elementIdx[side_b]].state) == kGas) {
        target = side_b;
        best = temperature[side_b];
    }
    if (temperature[below] <= best && 16383.5f < NextRoll(sim)) {
        uint16_t below_element = soa.elementIdx[below];
        if (StateOf(e[below_element].state) == kGas && e[below_element].molarMass <= elem.molarMass &&
            elem.molarMass != e[below_element].molarMass) {
            target = below;  // lighter gas below rises
        }
    }
    if (target == -1) {
        return;
    }
    SwapCells(soa, cell, target);
    int game_cell = ToGameCell(sim, target);
    if (IsValidGameCell(sim, game_cell)) {
        PushSubstanceChange(events, game_cell);
    }
    sim.unk_0x68[target] |= 0x1f;
    game_cell = ToGameCell(sim, cell);
    if (IsValidGameCell(sim, game_cell)) {
        PushSubstanceChange(events, game_cell);
    }
    sim.unk_0x68[cell] |= 0x1f;
}

}  // namespace

void PostProcessCell(SimData &sim, SimEvents &events, int cell) {
    CellAccessor self = Accessor(sim.updatedCells, cell);
    const ElementPostProcessData &elem = gElementPostProcessData[self.soa->elementIdx[cell]];
    switch (StateOf(elem.state)) {
    case kLiquid:
        PostProcessLiquid(sim, events, self, elem);
        return;
    case kSolid:
        if ((elem.state & 0xb) != 0xb) {
            return;  // not an unstable solid
        }
        if (sim.savedOptions & 1) {
            DoUnstableCheckWithDiagonals(sim, events, self);
        } else {
            DoUnstableCheckBasic(sim, events, self);
        }
        return;
    case kGas:
        PostProcessGas(sim, events, self, elem);
        break;
    default:
        if (self.soa->properties[cell] & 1) {
            return;
        }
        break;
    }
    DoSublimation(sim, events, cell, elem);
}
