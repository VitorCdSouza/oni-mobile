// SimFrameManager::ProcessFrame: applies one frame's messages to the simulation, in the
// original's order, and empties the frame's message lists.
#include <cstdio>
#include <queue>
#include <vector>

#include "sim_common.h"

void FloodRemoved(SimData &sim, float &amount, unsigned short element, ConsumedMassInfo &info, int x, int y, int radius,
                  std::vector<int> &visited, std::queue<FloodFillInfo> &queue);
void RectangularRemoved(SimData &sim, float &amount, unsigned short element, ConsumedMassInfo &info, int x, int y,
                        int radius, int height);
void ProcessCellModifications(SimFrameInfo &frame, SimData &sim);

namespace LogType {
enum Type : int32_t {};
}
struct Logger {
    enum eLogLevel : int32_t {};
    void Log(eLogLevel level, LogType::Type type, const char *format, ...);
};
namespace Util {
template <typename T>
class cSingleton {
public:
    static T *mInstance;
};
}  // namespace Util

namespace {

// Game cell index to sim cell index (the sim grid has a one-cell border).
int SimCell(const SimData &sim, int game_cell) {
    int w = sim.width - 2;
    return game_cell % w + 1 + (game_cell / w + 1) * sim.width;
}

void PushCallback(SimEvents &events, int callback) {
    if (callback != -1) {
        events.callbacks.push_back(CallbackInfo{callback});
    }
}

void PushStateChanged(SimEvents &events, int callback, uint32_t handle) {
    if (callback != -1) {
        events.componentStateChanged.push_back(
            ComponentStateChangedMessage{callback, static_cast<int32_t>(handle)});
    }
}

void ClearCellDisease(CellSOA &cells, int c) {
    cells.diseaseIdx[c] = kNoDisease;
    cells.diseaseCount[c] = 0;
    cells.diseaseInfestationTickCount[c] = 0;
    cells.diseaseAccumulatedError[c] = 0.0f;
}

// A handle is used only when it is valid and its version matches the slot's.
bool IsLive(const std::vector<uint8_t> &versions, uint32_t handle) {
    if ((handle & 0xf00000) != 0) {
        return false;
    }
    uint32_t index = handle & 0xffffff;
    return index < versions.size() && versions[index] == static_cast<uint8_t>(handle >> 24);
}

void ApplyCellEnergy(SimData &sim, const CellEnergyModification &m) {
    CellSOA &upd = *sim.updatedCells;
    int c = SimCell(sim, m.cellIdx);
    const ElementTemperatureData &e = gElementTemperatureData[upd.elementIdx[c]];
    if ((e.state & 3) == 0) {
        return;
    }
    float mass = upd.mass[c];
    if (!(mass > 0.001f)) {
        return;
    }
    float max_t = m.maxTemperature;
    if (!(max_t > 0.0f)) {
        return;
    }
    float delta = m.kilojoules / (e.specificHeatCapacity * mass);
    float t = upd.temperature[c];
    float hi = MaxSS(max_t, t);
    float nt = MinSS(delta + t, hi);
    if (0.0f >= nt || nt > 10000.0f) {
        fprintf(stderr,
                "INVALID TEMPERATURE FOR CELL ENERGY MODIFICATION:\nID(%d), kj(%f), maxT(%f), cell(%d), mass(%f), "
                "deg. chg(%f), temp(%f)\n",
                m.id, static_cast<double>(m.kilojoules), static_cast<double>(max_t), c, static_cast<double>(mass),
                static_cast<double>(delta), static_cast<double>(t));
        nt = MaxSS(MinSS(nt, 10000.0f), 1.0f);
    }
    if (!(nt > 0.0f)) {
        return;
    }
    upd.temperature[c] = nt;
    DoStateTransition(sim, *sim.events, c, e);
}

void ApplyProperties(SimData &sim, const CellPropertiesChange &m) {
    CellSOA &upd = *sim.updatedCells;
    int c = SimCell(sim, m.cellIdx);
    uint8_t &p = upd.properties[c];
    if (m.set == 0) {
        p &= static_cast<uint8_t>(~m.properties);
    } else {
        p |= m.properties;
    }
    uint8_t props = upd.properties[c];
    if (props != 0 && !(0.0f >= upd.mass[c])) {
        if ((props & 1) != 0) {
            DisplaceGas(sim, *sim.events, sim.updatedCells, c, upd.elementIdx[c]);
        } else if ((props & 2) != 0) {
            DisplaceLiquid(sim, *sim.events, sim.updatedCells, c, upd.elementIdx[c]);
        }
    }
    PushCallback(*sim.events, m.callbackIdx);
}

// Mass-weighted temperature of two masses, clamped between the two temperatures.
float MixTemperature(float mass_a, float t_a, float mass_b, float t_b, float total) {
    float mixed = (mass_a * t_a + mass_b * t_b) / total;
    if (!(t_a > t_b)) {
        return t_a <= mixed ? MinSS(t_b, mixed) : t_a;
    }
    if (!(mixed >= t_b)) {
        return t_b;
    }
    return MinSS(t_a, mixed);
}

void ApplyMassEmission(SimData &sim, const MassEmission &m) {
    CellSOA &upd = *sim.updatedCells;
    int c = SimCell(sim, m.cellIdx);
    uint16_t current = upd.elementIdx[c];
    if (current != m.elementIdx && current != sim.vacuumElementIdx) {
        uint8_t state = gElements[m.elementIdx].state & 3;
        bool displaced = false;
        if (state == kGas) {
            displaced = DisplaceGas(sim, *sim.events, &upd, c, current);
        } else if (state == kLiquid) {
            displaced = DisplaceLiquid(sim, *sim.events, &upd, c, current);
        }
        if (!displaced) {
            if (m.callbackIdx != -1) {
                MassEmittedCallback cb{};
                cb.callbackIdx = m.callbackIdx;
                cb.elemIdx = kInvalidElement;
                cb.suceeded = 0;
                cb.diseaseIdx = kNoDisease;
                sim.events->massEmitted.push_back(cb);
            }
            return;
        }
    }
    float cell_mass = upd.mass[c];
    float total = m.mass + cell_mass;
    upd.temperature[c] = 0.0f >= total ? 0.0f : MixTemperature(cell_mass, upd.temperature[c], m.mass, m.temperature, total);
    upd.mass[c] = upd.mass[c] + m.mass;
    if (m.diseaseIdx != kNoDisease) {
        gDisease->AddDiseaseToCell(sim.updatedCells, c, m.diseaseIdx, m.diseaseCount);
    }
    if (current != m.elementIdx) {
        upd.elementIdx[c] = m.elementIdx;
        int game_cell = ToGameCell(sim, c);
        if (IsValidGameCell(sim, game_cell)) {
            PushSubstanceChange(*sim.events, game_cell);
        }
        sim.unk_0x68[c] |= 0x1f;
    }
    if (m.callbackIdx != -1) {
        MassEmittedCallback cb;
        cb.callbackIdx = m.callbackIdx;
        cb.elemIdx = m.elementIdx;
        cb.suceeded = 1;
        cb.diseaseIdx = m.diseaseIdx;
        cb.mass = m.mass;
        cb.temperature = m.temperature;
        cb.diseaseCount = m.diseaseCount;
        sim.events->massEmitted.push_back(cb);
    }
}

void ApplyConsumeDisease(SimData &sim, const ConsumeDisease &m) {
    CellSOA &upd = *sim.updatedCells;
    int c = SimCell(sim, m.gameCell);
    int consumed = 0;
    uint8_t idx = upd.diseaseIdx[c];
    if (idx != kNoDisease) {
        int count = upd.diseaseCount[c];
        int wanted = static_cast<int>(static_cast<float>(count) * m.percentToConsume + 0.5f);
        consumed = wanted <= m.maxToConsume ? wanted : m.maxToConsume;
        count -= consumed;
        upd.diseaseCount[c] = count;
        if (count < 1) {
            ClearCellDisease(upd, c);
        }
        idx = upd.diseaseIdx[c];
    }
    if (m.callbackIdx != -1) {
        DiseaseConsumptionCallback cb{};
        cb.callbackIdx = m.callbackIdx;
        cb.diseaseIdx = idx;
        cb.diseaseCount = consumed;
        sim.events->diseaseConsumed.push_back(cb);
    }
}

void ApplyDiseaseModification(SimData &sim, const CellDiseaseModification &m) {
    CellSOA &upd = *sim.updatedCells;
    int c = SimCell(sim, m.cellIdx);
    if (m.diseaseIdx != kNoDisease) {
        gDisease->AddDiseaseToCell(sim.updatedCells, c, m.diseaseIdx, m.diseaseCount);
        return;
    }
    int count = m.diseaseCount + upd.diseaseCount[c];
    upd.diseaseCount[c] = count;
    if (count < 1) {
        ClearCellDisease(upd, c);
    }
}

void ApplyRadiation(SimData &sim, const CellRadiationModification &m) {
    float &radiation = sim.updatedCells->unk_0xf0[SimCell(sim, m.cellIdx)];
    float before = radiation;
    float after = m.radiationDelta + before;
    float reported = m.radiationDelta;
    if (!(0.0f < after)) {
        reported = before;
        after = 0.0f;
    }
    radiation = after;
    if (m.callbackIdx != -1) {
        sim.events->radiationConsumed.push_back(ConsumedRadiationCallback{m.callbackIdx, m.cellIdx, reported});
    }
}

void ApplyRadiationParams(SimData &sim, const RadiationParamsModification &m) {
    switch (m.RadiationParamsType) {
    case 0: sim.unk_0x71c = m.value; break;
    case 2: sim.unk_0x724 = m.value; break;
    case 3: sim.unk_0x728 = m.value; break;
    case 4: sim.unk_0x72c = m.value; break;
    case 5: sim.unk_0x720 = m.value; break;
    default: break;
    }
}

// Digging a solid cell (or backwall) clears it and reports what falls out as dig ore.
void ApplyDig(SimData &sim, const DigPoint &m) {
    SimEvents &events = *sim.events;
    int c = SimCell(sim, m.cellIdx);
    CellAccessor at;
    at.soa = sim.updatedCells;
    at.cell = c;
    uint16_t element = m.backwall ? sim.backwall->elementIdx[c] : sim.updatedCells->elementIdx[c];
    PushCallback(events, m.callbackIdx);
    const Element &e = gElements[element];
    if ((e.state & 3) != kSolid) {
        return;
    }
    if (!m.skipEvent) {
        float mass;
        float temperature;
        uint8_t disease_idx;
        int disease_count;
        if (!m.backwall) {
            mass = sim.updatedCells->mass[c];
            temperature = sim.updatedCells->temperature[c];
            disease_idx = sim.updatedCells->diseaseIdx[c];
            disease_count = sim.updatedCells->diseaseCount[c];
        } else {
            disease_count = 0;
            disease_idx = kNoDisease;
            mass = sim.backwall->mass[c];
            temperature = sim.backwall->temperature[c];
        }
        if (mass > 0.0f) {
            SIM_ASSERT(!(temperature <= 0.0f), 0x8b, "SimEvents.h");
            int game_cell = ToGameCell(sim, c);
            if (IsValidGameCell(sim, game_cell)) {
                SpawnOreInfo ore{};
                ore.cellIdx = game_cell;
                ore.elemIdx = e.elementsTableIdx;
                ore.diseaseIdx = disease_idx;
                ore.mass = mass;
                ore.temperature = temperature;
                ore.diseaseCount = disease_count;
                events.digInfo.push_back(ore);
            }
        }
    }
    int game_cell;
    if (m.backwall) {
        sim.ClearBackwall(c);
        game_cell = ToGameCell(sim, c);
        if (game_cell >= 0 && game_cell < sim.numGameCells) {
            events.backwallElementChanged.push_back(BackwallElementChangedInfo{game_cell});
        }
    } else {
        sim.ClearCell(at);
        game_cell = ToGameCell(sim, c);
    }
    if (IsValidGameCell(sim, game_cell)) {
        PushSubstanceChange(events, game_cell);
    }
    sim.unk_0x68[c] |= 0x1f;
}

void ApplyBuildingEnergy(SimData &sim, const ModifyBuildingEnergyMessage &m) {
    BuildingHeatExchange &bhe = sim.buildingHeatExchange;
    uint32_t handle = static_cast<uint32_t>(m.handle);
    if (!IsLive(bhe.versions, handle)) {
        return;
    }
    BuildingHeatExchangeData &b = bhe.data[static_cast<size_t>(bhe.handleToDense[handle & 0xffffff])];
    float t = b.temperature;
    float lo = MinSS(m.minTemperature, t);
    float hi = MaxSS(m.maxTemperature, t);
    if (0.0f > lo || hi > 10000.0f) {
        return;
    }
    float capacity = static_cast<float>((b.area.maxX - b.area.minX) * (b.area.maxY - b.area.minY)) * b.heatCapacityPerCell;
    float delta = m.deltaKJ / capacity;
    float nt = MaxSS(MinSS(t + delta, hi), lo);
    if (!(0.0f >= nt) && !(nt >= 10000.0f)) {
        b.temperature = nt;
        return;
    }
    Logger *log = Util::cSingleton<Logger>::mInstance;
    if (log == nullptr) {
        return;
    }
    const auto level = static_cast<Logger::eLogLevel>(1);
    const auto type = static_cast<LogType::Type>(1);
    log->Log(level, type, "Invalid final temperature in ProcessBuildingHeatExchangeMessages");
    if ((log = Util::cSingleton<Logger>::mInstance) == nullptr) {
        return;
    }
    log->Log(level, type, "delta: %f hc: %f ft: %f [%f - %f]", static_cast<double>(delta), static_cast<double>(capacity),
             static_cast<double>(nt), static_cast<double>(lo), static_cast<double>(hi));
    if ((log = Util::cSingleton<Logger>::mInstance) == nullptr) {
        return;
    }
    log->Log(level, type, "msg dkj: %f, %f - %f", static_cast<double>(m.deltaKJ), static_cast<double>(m.minTemperature),
             static_cast<double>(m.maxTemperature));
    if ((log = Util::cSingleton<Logger>::mInstance) == nullptr) {
        return;
    }
    log->Log(level, type, "building %i [%f @(%i,%i)-(%i,%i) %f %f]", m.handle, static_cast<double>(b.meltTemperature),
             b.area.minX, b.area.minY, b.area.maxX, b.area.maxY, static_cast<double>(b.heatCapacityPerCell),
             static_cast<double>(b.temperature));
}

void ApplyBackwall(SimData &sim, const ModifyBackwallDataMessage &m) {
    BackwallSOA &bw = *sim.backwall;
    int c = SimCell(sim, m.gameCell);
    uint16_t before = bw.elementIdx[c];
    bw.elementIdx[c] = m.elementIdx;
    bw.mass[c] = m.mass;
    bw.temperature[c] = m.temperature;
    if (before != m.elementIdx) {
        int game_cell = ToGameCell(sim, c);
        if (game_cell >= 0 && game_cell < sim.numGameCells) {
            sim.events->backwallElementChanged.push_back(BackwallElementChangedInfo{game_cell});
        }
    }
}

// Register / Unregister report the new handle (or 0xffffffff) to the message's callback.
template <typename Component, typename Message>
void RegisterAll(SimData &sim, Component &component, std::vector<Message> &messages, int Message::*callback) {
    for (const Message &m : messages) {
        Handle handle = component.Register(sim, m);
        PushStateChanged(*sim.events, m.*callback, handle.value);
    }
    messages.clear();
}

template <typename Component, typename Message>
void UnregisterAll(SimData &sim, Component &component, std::vector<Message> &messages, int Message::*handle,
                   int Message::*callback) {
    for (const Message &m : messages) {
        component.Unregister(Handle(static_cast<uint32_t>(m.*handle)));
        PushStateChanged(*sim.events, m.*callback, 0xffffffffu);
    }
    messages.clear();
}

template <typename Message, typename Apply>
void ForEach(std::vector<Message> &messages, Apply apply) {
    for (const Message &m : messages) {
        apply(m);
    }
    messages.clear();
}

}  // namespace

void SimFrameManager::ProcessFrame(SimFrameInfo &f, SimData &sim) {
    ForEach(f.insulationChanges, [&](const SetCellFloatValue &m) {
        sim.updatedCells->insulation[SimCell(sim, m.cellIdx)] = static_cast<uint8_t>(static_cast<int>(m.value * 255.0f));
    });
    ForEach(f.strengthChanges, [&](const SetCellFloatValue &m) {
        sim.updatedCells->strengthInfo[SimCell(sim, m.cellIdx)] = static_cast<uint8_t>(static_cast<int>(m.value));
    });
    ForEach(f.cellEnergyModifications, [&](const CellEnergyModification &m) { ApplyCellEnergy(sim, m); });
    ForEach(f.propertyChanges, [&](const CellPropertiesChange &m) { ApplyProperties(sim, m); });

    {
        std::vector<int> visited;
        std::queue<FloodFillInfo> queue;
        for (const MassConsumption &m : f.massConsumptions) {
            int c = SimCell(sim, m.cellIdx);
            int y = c / sim.width;
            int x = c % sim.width;
            visited.clear();
            float amount = m.mass;
            ConsumedMassInfo info{};
            info.simHandle = -1;
            info.removedElemIdx = m.elementIdx;
            info.diseaseIdx = kNoDisease;
            if (m.height == 0) {
                FloodRemoved(sim, amount, m.elementIdx, info, x, y, m.radius, visited, queue);
            } else {
                RectangularRemoved(sim, amount, m.elementIdx, info, x, y, m.radius, m.height);
            }
            while (!queue.empty()) {
                queue.pop();
            }
            if (m.callbackIdx != -1) {
                MassConsumedCallback cb{};
                cb.callbackIdx = m.callbackIdx;
                cb.elemIdx = info.removedElemIdx;
                cb.diseaseIdx = info.diseaseIdx;
                cb.mass = info.mass;
                cb.temperature = info.temperature;
                cb.diseaseCount = info.diseaseCount;
                sim.events->massConsumed.push_back(cb);
            }
        }
        f.massConsumptions.clear();
    }

    ForEach(f.massEmissions, [&](const MassEmission &m) { ApplyMassEmission(sim, m); });
    ForEach(f.diseaseConsumptions, [&](const ConsumeDisease &m) { ApplyConsumeDisease(sim, m); });
    ForEach(f.diseaseModifications, [&](const CellDiseaseModification &m) { ApplyDiseaseModification(sim, m); });
    ForEach(f.radiationModifications, [&](const CellRadiationModification &m) { ApplyRadiation(sim, m); });
    ForEach(f.radiationParams, [&](const RadiationParamsModification &m) { ApplyRadiationParams(sim, m); });
    ForEach(f.digs, [&](const DigPoint &m) { ApplyDig(sim, m); });
    ProcessCellModifications(f, sim);
    ForEach(f.worldZoneModifications, [&](const CellWorldZoneModification &m) {
        sim.unk_0x38[SimCell(sim, m.cell)] = m.zoneID;
    });

    BuildingHeatExchange &bhe = sim.buildingHeatExchange;
    RegisterAll(sim, bhe, f.addBuildingHeatExchange, &AddBuildingHeatExchangeMessage::callbackIdx);
    ForEach(f.modifyBuildingHeatExchange, [&](const ModifyBuildingHeatExchangeMessage &m) {
        if (IsLive(bhe.versions, static_cast<uint32_t>(m.callbackIdx))) {
            bhe.Modify(sim, m);
        }
    });
    ForEach(f.modifyBuildingEnergy, [&](const ModifyBuildingEnergyMessage &m) { ApplyBuildingEnergy(sim, m); });
    UnregisterAll(sim, bhe, f.removeBuildingHeatExchange, &RemoveBuildingHeatExchangeMessage::handle,
                  &RemoveBuildingHeatExchangeMessage::callbackIdx);

    BuildingToBuildingHeatExchange &b2b = sim.buildingToBuildingHeatExchange;
    RegisterAll(sim, b2b, f.registerBuildingToBuilding, &RegisterBuildingToBuildingHeatExchangeMessage::callbackIdx);
    ForEach(f.removeBuildingInContact, [&](const RemoveBuildingInContactFromBuildingToBuildingHeatExchangeMessage &m) {
        if (IsLive(b2b.versions, static_cast<uint32_t>(m.selfHandler))) {
            b2b.Remove(sim, m);
        }
    });
    ForEach(f.addBuildingInContact, [&](const AddInContactBuildingToBuildingToBuildingHeatExchangeMessage &m) {
        if (IsLive(b2b.versions, static_cast<uint32_t>(m.selfHandler))) {
            b2b.Add(sim, m);
        }
    });
    UnregisterAll(sim, b2b, f.removeBuildingToBuilding, &RemoveBuildingToBuildingHeatExchangeMessage::selfHandler,
                  &RemoveBuildingToBuildingHeatExchangeMessage::callbackIdx);

    ElementChunk &chunks = sim.elementChunk;
    RegisterAll(sim, chunks, f.addElementChunk, &AddElementChunkMessage::callbackIdx);
    ForEach(f.moveElementChunk, [&](const MoveElementChunkMessage &m) {
        uint32_t handle = static_cast<uint32_t>(m.handle);
        if (IsLive(chunks.versions, handle)) {
            chunks.data[static_cast<size_t>(chunks.handleToDense[handle & 0xffffff])].cell = SimCell(sim, m.gameCell);
        }
    });
    ForEach(f.modifyElementChunk, [&](const ModifyElementChunkMessage &m) {
        if (IsLive(chunks.versions, static_cast<uint32_t>(m.handle))) {
            chunks.Modify(sim, m);
        }
    });
    ForEach(f.modifyElementChunkEnergy, [&](const ModifyElementChunkEnergyMessage &m) {
        if (IsLive(chunks.versions, static_cast<uint32_t>(m.handle))) {
            chunks.ModifyEnergy(sim, m);
        }
    });
    ForEach(f.modifyElementChunkAdjuster, [&](const ModifyElementChunkAdjusterMessage &m) {
        if (IsLive(chunks.versions, static_cast<uint32_t>(m.handle))) {
            chunks.ModifyAdjuster(sim, m);
        }
    });
    ForEach(f.modifyBackwall, [&](const ModifyBackwallDataMessage &m) { ApplyBackwall(sim, m); });
    UnregisterAll(sim, chunks, f.removeElementChunk, &RemoveElementChunkMessage::handle,
                  &RemoveElementChunkMessage::callbackIdx);

    ElementConsumer &consumers = sim.elementConsumer;
    RegisterAll(sim, consumers, f.addElementConsumer, &AddElementConsumerMessage::callbackIdx);
    ForEach(f.modifyElementConsumer, [&](const ModifyElementConsumerMessage &m) { consumers.Modify(sim, m); });
    UnregisterAll(sim, consumers, f.removeElementConsumer, &RemoveElementConsumerMessage::handle,
                  &RemoveElementConsumerMessage::callbackIdx);

    ElementEmitter &emitters = sim.elementEmitter;
    RegisterAll(sim, emitters, f.addElementEmitter, &AddElementEmitterMessage::callbackIdx);
    ForEach(f.modifyElementEmitter, [&](const ModifyElementEmitterMessage &m) { emitters.Modify(sim, m); });
    UnregisterAll(sim, emitters, f.removeElementEmitter, &RemoveElementEmitterMessage::handle,
                  &RemoveElementEmitterMessage::callbackIdx);

    RadiationEmitter &radiation = sim.radiationEmitter;
    RegisterAll(sim, radiation, f.addRadiationEmitter, &AddRadiationEmitterMessage::callbackIdx);
    ForEach(f.modifyRadiationEmitter, [&](const ModifyRadiationEmitterMessage &m) { radiation.Modify(sim, m); });
    UnregisterAll(sim, radiation, f.removeRadiationEmitter, &RemoveRadiationEmitterMessage::handle,
                  &RemoveRadiationEmitterMessage::callbackIdx);

    DiseaseEmitter &disease_emitters = sim.diseaseEmitter;
    RegisterAll(sim, disease_emitters, f.addDiseaseEmitter, &AddDiseaseEmitterMessage::callbackIdx);
    ForEach(f.modifyDiseaseEmitter, [&](const ModifyDiseaseEmitterMessage &m) { disease_emitters.Modify(sim, m); });
    UnregisterAll(sim, disease_emitters, f.removeDiseaseEmitter, &RemoveDiseaseEmitterMessage::handle,
                  &RemoveDiseaseEmitterMessage::callbackIdx);

    DiseaseConsumer &disease_consumers = sim.diseaseConsumer;
    RegisterAll(sim, disease_consumers, f.addDiseaseConsumer, &AddDiseaseConsumerMessage::unk_0x0);
    ForEach(f.modifyDiseaseConsumer, [&](const ModifyDiseaseConsumerMessage &m) { disease_consumers.Modify(sim, m); });
    UnregisterAll(sim, disease_consumers, f.removeDiseaseConsumer, &RemoveDiseaseConsumerMessage::handle,
                  &RemoveDiseaseConsumerMessage::callbackIdx);

    // DebugProperties: the two heat exchange scales, then isDebugEditing and its three padding
    // bytes as one 32-bit copy.
    sim.unk_0xa0 = f.debugProperties.buildingTemperatureScale;
    sim.unk_0xa4 = f.debugProperties.buildingToBuildingTemperatureScale;
    __builtin_memcpy(&sim.unk_0xa8, &f.debugProperties.isDebugEditing, 4);
}
