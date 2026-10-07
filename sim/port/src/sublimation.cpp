// Solids next to a cell that give off gas (bleach stone, polluted dirt, oxylite...).
#include "sim_common.h"

// Lets the four neighbours of `cell` sublimate into it. Returns 1 when any of them did.
char DoSublimation(SimData &sim, SimEvents &events, int cell, const ElementPostProcessData &self) {
    CellSOA &home = *sim.updatedCells;
    float home_mass = home.mass[cell];
    if (1.8f < home_mass || home_mass == 1.8f) {
        return 0;
    }
    char result = 0;
    const int neighbours[4] = {sim.width + cell, cell + 1, cell - 1, cell - sim.width};
    const uint16_t home_element = home.elementIdx[cell];
    for (int n : neighbours) {
        CellSOA &soa = *sim.updatedCells;
        const ElementPostProcessData &solid = gElementPostProcessData[soa.elementIdx[n]];
        if (StateOf(solid.state) != kSolid) {
            continue;
        }
        uint16_t gas = solid.sublimateIndex;
        if (gas == kInvalidElement) {
            continue;
        }
        float roll = SimRandom01(sim);
        if (!(roll < solid.sublimateProbability || roll == solid.sublimateProbability)) {
            continue;
        }
        float rate = solid.sublimateRate * 0.2f;
        float solid_mass = soa.mass[n];
        float amount = solid_mass <= rate ? solid_mass : rate;

        if (solid_mass - amount < rate) {
            // Too little left: the whole tile turns into gas in place.
            soa.elementIdx[n] = gas;
            soa.mass[n] = solid_mass * solid.sublimateEfficiency;
            int game_cell = ToGameCell(sim, n);
            sim.unk_0x68[n] |= 0x1f;
            if (game_cell >= 0) {
                if (game_cell < sim.numGameCells) {
                    PushSubstanceChange(events, game_cell);
                    PushSpawnFX(events, game_cell, solid.sublimateFX, 0.0f);
                }
            }
            result = 1;
            continue;
        }

        float solid_disease = static_cast<float>(soa.diseaseCount[n]);
        int disease = static_cast<int>((amount / solid_mass) * solid_disease);
        if (gas == home_element) {
            float added = solid.sublimateEfficiency * amount;
            float room = 1.8f - home.mass[cell];
            if (room < added) {
                amount = amount * (room / added);
                disease = static_cast<int>(solid_disease * (amount / solid_mass));
                added = room;
            }
            AddMassAndUpdateTemperature(&soa, &soa, cell, added, soa.temperature[n], soa.diseaseIdx[n], disease);
        } else {
            bool replace = true;
            if (StateOf(self.state) != kVacuum) {
                if (!DisplaceGas(sim, events, &soa, cell, home.elementIdx[cell])) {
                    // The original still consumes the solid when an earlier neighbour succeeded.
                    if (!result) {
                        continue;
                    }
                    replace = false;
                } else {
                    gas = solid.sublimateIndex;
                }
            }
            if (replace) {
                home.elementIdx[cell] = gas;
                home.mass[cell] = solid.sublimateEfficiency * amount + home.mass[cell];
                home.temperature[cell] = soa.temperature[n];
                home.diseaseIdx[cell] = soa.diseaseIdx[n];
                home.diseaseCount[cell] = disease;
                int game_cell = ToGameCell(sim, cell);
                if (IsValidGameCell(sim, game_cell)) {
                    PushSubstanceChange(events, game_cell);
                }
                sim.unk_0x68[cell] |= 0x1f;
            }
        }

        soa.mass[n] -= amount;
        soa.diseaseCount[n] -= disease;
        if (soa.diseaseCount[n] < 1) {
            ClearDisease(soa, n);
        }
        sim.unk_0x58[n] = amount + sim.unk_0x58[n];
        int offset = n - cell;
        float rotation = -90.0f;
        if (offset != -1) {
            rotation = 180.0f;
            if (offset != sim.width) {
                rotation = offset == 1 ? 90.0f : 0.0f;
            }
        }
        int game_cell = ToGameCell(sim, n);
        if (IsValidGameCell(sim, game_cell)) {
            PushSpawnFX(events, game_cell, solid.sublimateFX, rotation);
        }
        if (soa.mass[n] <= 1e-09f) {
            Evaporate(n, sim, events);
        }
        result = 1;
    }
    return result;
}
