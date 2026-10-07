// Buildings that put matter into the cells around them (vents, generators' exhaust, ...).
#include "sim_common.h"

namespace {

// True when some reachable cell is below the emitter's maximum pressure.
bool CanEmit(const ElementEmitterData &d, const std::vector<int> &reachable, SimData &sim) {
    for (int cell : reachable) {
        float mass = sim.updatedCells->mass[cell];
        if (mass <= d.maxPressure && d.maxPressure != mass) {
            return true;
        }
    }
    return false;
}

// ElementEmitter::Emit: puts the emitter's mass into `cell` (which holds vacuum or the same
// element), mixing temperatures; always reports what was meant to be emitted.
EmittedMassInfo Emit(const ElementEmitterData &d, int cell, float temperature, SimData &sim) {
    CellSOA *upd = sim.updatedCells;
    uint16_t current = upd->elementIdx[cell];
    if (d.elementIdx == current || sim.vacuumElementIdx == current) {
        upd->elementIdx[cell] = d.elementIdx;
        float mass = d.emitMass;
        float cell_temperature = upd->temperature[cell];
        float cell_mass = upd->mass[cell];
        float total = mass + cell_mass;
        float result;
        if (0.0f >= total) {
            result = 0.0f;
        } else {
            float mixed = (mass * temperature + cell_mass * cell_temperature) / total;
            if (cell_temperature > temperature) {
                result = temperature <= mixed ? MinSS(cell_temperature, mixed) : temperature;
            } else {
                result = mixed < cell_temperature ? cell_temperature : MinSS(mixed, temperature);
            }
        }
        upd->temperature[cell] = result;
        upd->mass[cell] = upd->mass[cell] + d.emitMass;
        gDisease->AddDiseaseToCell(upd, cell, d.diseaseIdx, d.diseaseCount);
        CellSOA *after = sim.updatedCells;
        if (0.0f >= after->temperature[cell] && IsNonZero(after->mass[cell]) && !gAssertsDisabled) {
            AssertFunc("BREAKPT:", 0xd4, "Components/ElementEmitter.cpp");
        }
        if (sim.vacuumElementIdx == current) {
            int game_cell = ToGameCell(sim, cell);
            if (IsValidGameCell(sim, game_cell)) {
                PushSubstanceChange(*sim.events, game_cell);
            }
            sim.unk_0x68[cell] |= 0x1f;
        }
    } else if (!gAssertsDisabled) {
        AssertFunc("BREAKPT:", 0xdf, "Components/ElementEmitter.cpp");
    }
    EmittedMassInfo info{};
    info.elemIdx = d.elementIdx;
    info.diseaseIdx = kNoDisease;
    info.mass = d.emitMass;
    info.temperature = temperature;
    info.diseaseCount = 0;
    return info;
}

// ElementEmitter::TryEmit: the first reachable cell (round robin from `start`) below the maximum
// pressure that can take the element, pushing other gases or liquids away. Solids are spawned
// as debris instead.
EmittedMassInfo TryEmit(const ElementEmitterData &d, const std::vector<int> &reachable, unsigned long start,
                        SimData &sim) {
    EmittedMassInfo none{};
    none.elemIdx = kInvalidElement;
    none.diseaseIdx = kNoDisease;
    float temperature = d.emitTemperature;
    const Element &element = gElements[d.elementIdx];
    if (!(temperature >= 0.0f)) {
        temperature = element.defaultValues.temperature;
    }
    uint8_t state = StateOf(element.state);
    size_t n = reachable.size();
    for (size_t k = 0; k < n; k++) {
        int cell = reachable[(start + k) % n];
        if (!(d.maxPressure > sim.updatedCells->mass[cell])) {
            continue;
        }
        if (state == kLiquid) {
            uint16_t current = sim.updatedCells->elementIdx[cell];
            if (current == d.elementIdx || current == sim.vacuumElementIdx ||
                DisplaceLiquid(sim, *sim.events, sim.updatedCells, cell, current) ||
                DisplaceGas(sim, *sim.events, sim.updatedCells, cell, sim.updatedCells->elementIdx[cell])) {
                return Emit(d, cell, temperature, sim);
            }
        } else if (state == kSolid) {
            if (!(d.emitMass > 0.0f)) {
                continue;
            }
            if (0.0f >= temperature && !gAssertsDisabled) {
                AssertFunc("BREAKPT:", 0x7b, "SimEvents.h");
            }
            int game_cell = ToGameCell(sim, cell);
            if (IsValidGameCell(sim, game_cell) && (sim.unk_0xa8 != 0 || sim.unk_0x88[game_cell] != 0)) {
                SpawnOreInfo ore{};
                ore.cellIdx = game_cell;
                ore.elemIdx = d.elementIdx;
                ore.diseaseIdx = kNoDisease;
                ore.mass = d.emitMass;
                ore.temperature = temperature;
                ore.diseaseCount = 0;
                sim.events->spawnOre.push_back(ore);
            }
        } else if (state == kGas) {
            uint16_t current = sim.updatedCells->elementIdx[cell];
            if (current == d.elementIdx || current == sim.vacuumElementIdx ||
                DisplaceGas(sim, *sim.events, sim.updatedCells, cell, current)) {
                return Emit(d, cell, temperature, sim);
            }
        }
    }
    return none;
}

}  // namespace

void ElementEmitter::Update(float dt, SimData &sim, const Region &region) {
    emitted.resize(handleToDense.size());
    for (size_t i = 0; i < data.size(); i++) {
        ElementEmitterData &d = data[i];
        int x = d.x, y = d.y;
        if (!(region.minX <= x && region.minY <= y && x <= region.maxX && y <= region.maxY)) {
            continue;
        }
        float timer = d.timer;
        float interval = d.emitInterval;
        if (interval <= timer) {
            EmittedMassInfo &slot = emitted[denseToHandle[i] & 0xffffff];
            uint16_t slot_element = slot.elemIdx;
            if (slot_element == kInvalidElement || slot_element == sim.vacuumElementIdx ||
                slot_element == d.elementIdx) {
                GetReachableCells(sim, x, y, d.range, visited, queue, reachable, true);
                int callback = -1;
                if (!CanEmit(d, reachable, sim)) {
                    if (d.blocked != 1) {
                        d.blocked = 1;
                        callback = d.onBlockedCallback;
                    }
                } else if (d.blocked != 0) {
                    d.blocked = 0;
                    callback = d.onUnblockedCallback;
                }
                if (callback != -1) {
                    sim.events->callbacks.push_back(CallbackInfo{callback});
                }
                EmittedMassInfo result = TryEmit(d, reachable, d.startIndex, sim);
                if (0.0f < result.mass) {
                    float total = result.mass + slot.mass;
                    float t;
                    if (0.0f >= total) {
                        t = 0.0f;
                    } else {
                        float mixed = (slot.mass * slot.temperature + result.mass * result.temperature) / total;
                        if (slot.temperature > result.temperature) {
                            t = mixed < result.temperature ? result.temperature : MinSS(slot.temperature, mixed);
                        } else {
                            t = mixed < slot.temperature ? slot.temperature : MinSS(result.temperature, mixed);
                        }
                    }
                    slot.elemIdx = result.elemIdx;
                    slot.temperature = t;
                    slot.mass = total;
                }
                visited.clear();
                reachable.clear();
                timer = d.timer;
                interval = d.emitInterval;
            }
            timer = timer - interval;
        }
        d.timer = timer + dt;
    }
}

// The SimComponent interface entry: in the original this thunk is a full copy of Update.
extern "C" __attribute__((visibility("default"))) void ElementEmitterUpdateThunk(char *component, float dt,
                                                                                 SimData &sim, const Region &region)
    __asm__("_ZThn128_N14ElementEmitter6UpdateEfR7SimDataRK6Region");
void ElementEmitterUpdateThunk(char *component, float dt, SimData &sim, const Region &region) {
    reinterpret_cast<ElementEmitter *>(component - 0x80)->Update(dt, sim, region);
}

// Resets what the game reads back for every emitter, without emitting.
void ElementEmitter::UpdateDataListOnly(SimData &) {
    emitted.resize(handleToDense.size());
    size_t count = data.size();
    for (size_t i = 0; i < count; ++i) {
        EmittedMassInfo &slot = emitted[denseToHandle[i] & 0xffffff];
        slot.elemIdx = 0xffff;
        slot.diseaseIdx = 0xff;
        slot.pad0 = 0;
        slot.mass = 0.0f;
        slot.temperature = 0.0f;
        slot.diseaseCount = 0;
    }
}
