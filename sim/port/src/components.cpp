// Register / Modify / Unregister of the components (the messages SimFrameManager::ProcessFrame
// applies). Every component is a CompactedVector: dense data plus handle <-> dense index maps.
#include <cfloat>

#include "sim_common.h"

namespace {

int SimCell(const SimData &sim, int game_cell) {
    int w = sim.width - 2;
    return game_cell % w + 1 + (game_cell / w + 1) * sim.width;
}

// GameCellToXY: sim coordinates of a game cell.
void GameCellToXY(const SimData &sim, int game_cell, uint16_t &x, uint16_t &y) {
    int c = SimCell(sim, game_cell);
    x = static_cast<uint16_t>(c % sim.width);
    y = static_cast<uint16_t>(c / sim.width);
}

// Takes a free handle (its version was bumped when it was freed) or makes a new one with
// version 0, and appends the data.
template <typename Component, typename T>
Handle CvAdd(Component &c, const T &value) {
    int32_t dense = static_cast<int32_t>(c.data.size());
    uint32_t handle;
    if (c.freeHandles.empty()) {
        c.versions.push_back(0);
        handle = static_cast<uint32_t>(c.handleToDense.size()) & 0xffffff;
        c.handleToDense.push_back(dense);
    } else {
        handle = c.freeHandles.back();
        c.freeHandles.pop_back();
        c.handleToDense[handle & 0xffffff] = dense;
    }
    c.denseToHandle.push_back(handle);
    c.data.push_back(value);
    return Handle(handle);
}

// Frees the handle (bumping its version) and moves the last element into the hole. Like the
// original, only the invalid bit is checked, not the version.
template <typename Component>
void CvRemove(Component &c, Handle handle) {
    uint32_t h = handle.value;
    if ((h & 0xf00000) != 0) {
        return;
    }
    uint32_t index = h & 0xffffff;
    int version = (static_cast<int32_t>(h) >> 24) + 1;
    c.versions[index] = static_cast<uint8_t>(version);
    c.freeHandles.push_back(static_cast<uint32_t>(version) << 24 | index);
    int32_t dense = c.handleToDense[index];
    c.handleToDense[index] = 0;
    int last = static_cast<int>(c.data.size()) - 1;
    if (dense < last) {
        c.data[static_cast<size_t>(dense)] = c.data[static_cast<size_t>(last)];
        uint32_t moved = c.denseToHandle[static_cast<size_t>(last)];
        c.handleToDense[moved & 0xffffff] = dense;
        c.denseToHandle[static_cast<size_t>(dense)] = moved;
    }
    c.denseToHandle.pop_back();
    c.data.pop_back();
}

template <typename Component>
bool IsLive(const Component &c, uint32_t h) {
    if ((h & 0xf00000) != 0) {
        return false;
    }
    uint32_t index = h & 0xffffff;
    return index < c.versions.size() && c.versions[index] == static_cast<uint8_t>(h >> 24);
}

template <typename Component>
auto &DataOf(Component &c, uint32_t h) {
    return c.data[static_cast<size_t>(c.handleToDense[h & 0xffffff])];
}

// HandleVector.h's check: asserts, then uses the handle anyway.
template <typename Component>
void AssertLive(const Component &c, uint32_t h) {
    if (!IsLive(c, h) && !gAssertsDisabled) {
        AssertFunc("BREAKPT:", 0xcf, "../util/HandleVector.h");
    }
}

}  // namespace

// --- ElementConsumer ---

Handle ElementConsumer::Register(const SimData &sim, const AddElementConsumerMessage &msg) {
    ElementConsumerData d{};
    d.consumptionRate = 0.0f;
    d.radius = msg.radius;
    d.configuration = msg.configuration;
    d.elementIdx = msg.elementIdx;
    d.roundRobin = 0;
    GameCellToXY(sim, msg.cellIdx, d.x, d.y);
    return CvAdd(*this, d);
}

void ElementConsumer::Modify(const SimData &sim, const ModifyElementConsumerMessage &msg) {
    uint32_t h = static_cast<uint32_t>(msg.handle);
    if (!IsLive(*this, h)) {
        return;
    }
    ElementConsumerData &d = DataOf(*this, h);
    GameCellToXY(sim, msg.cell, d.x, d.y);
    d.consumptionRate = msg.consumptionRate;
}

void ElementConsumer::Unregister(Handle handle) {
    CvRemove(*this, handle);
}

// --- ElementEmitter ---

Handle ElementEmitter::Register(const SimData &sim, const AddElementEmitterMessage &msg) {
    (void)sim;
    ElementEmitterData d{};
    d.timer = 0.0f;
    d.emitInterval = FLT_MAX;
    d.emitMass = 0.0f;
    d.emitTemperature = -1.0f;
    d.maxPressure = msg.maxPressure;
    d.blocked = 0xff;
    d.onBlockedCallback = msg.onBlockedCB;
    d.onUnblockedCallback = msg.onUnblockedCB;
    Handle handle = CvAdd(*this, d);
    // Indexed by the dense index, not the handle index, as in the original.
    size_t dense = static_cast<size_t>(handleToDense[handle.value & 0xffffff]);
    if (dense < emitted.size()) {
        EmittedMassInfo &e = emitted[dense];
        e.elemIdx = kInvalidElement;
        e.diseaseIdx = kNoDisease;
        e.pad0 = 0;
        e.mass = 0.0f;
        e.temperature = 0.0f;
        e.diseaseCount = 0;
    }
    return handle;
}

void ElementEmitter::Modify(const SimData &sim, const ModifyElementEmitterMessage &msg) {
    uint32_t h = static_cast<uint32_t>(msg.handle);
    if (!IsLive(*this, h)) {
        return;
    }
    ElementEmitterData &d = DataOf(*this, h);
    int c = SimCell(sim, msg.cellIdx);
    d.timer = 0.0f;
    d.range = msg.maxDepth;
    d.elementIdx = msg.elementIdx;
    d.startIndex = 0;
    d.diseaseIdx = msg.diseaseIdx;
    d.emitInterval = msg.emitInterval;
    d.emitMass = msg.emitMass;
    d.emitTemperature = msg.emitTemperature;
    d.maxPressure = msg.maxPressure;
    d.diseaseCount = msg.diseaseCount;
    d.x = static_cast<uint16_t>(c % sim.width);
    d.y = static_cast<uint16_t>(c / sim.width);
    if (!(0.0f < msg.emitMass)) {
        d.blocked = 0xff;
    }
}

void ElementEmitter::Unregister(Handle handle) {
    CvRemove(*this, handle);
}

// --- ElementChunk ---

Handle ElementChunk::Register(const SimData &sim, const AddElementChunkMessage &msg) {
    const ElementTemperatureData &e = gElementTemperatureData[msg.elementIdx];
    ElementChunkData d{};
    d.cell = SimCell(sim, msg.gameCell);
    d.temperature = msg.temperature;
    d.heatCapacity = e.specificHeatCapacity * msg.mass;
    d.thermalConductivity = e.thermalConductivity;
    d.areaOverThickness = msg.surfaceArea / msg.thickness;
    d.groundTransferScale = msg.groundTransferScale;
    d.highTemp = e.highTemp;
    d.lowTemp = e.lowTemp;
    return CvAdd(*this, d);
}

void ElementChunk::Modify(const SimData &sim, const ModifyElementChunkMessage &msg) {
    (void)sim;
    uint32_t h = static_cast<uint32_t>(msg.handle);
    AssertLive(*this, h);
    ElementChunkData &d = DataOf(*this, h);
    d.temperature = msg.temperature;
    d.heatCapacity = msg.heatCapacity;
    if (msg.heatCapacity != 0.0f && msg.temperature <= 0.0f && !gAssertsDisabled) {
        AssertFunc("BREAKPT:", 0xdd, "Components/ElementChunk.cpp");
    }
}

void ElementChunk::ModifyEnergy(const SimData &sim, const ModifyElementChunkEnergyMessage &msg) {
    (void)sim;
    uint32_t h = static_cast<uint32_t>(msg.handle);
    AssertLive(*this, h);
    ElementChunkData &d = DataOf(*this, h);
    if (d.heatCapacity > 0.0f) {
        d.temperature = MaxSS(msg.deltaKJ / d.heatCapacity + d.temperature, 0.0f);
    }
}

void ElementChunk::ModifyAdjuster(const SimData &sim, const ModifyElementChunkAdjusterMessage &msg) {
    (void)sim;
    uint32_t h = static_cast<uint32_t>(msg.handle);
    AssertLive(*this, h);
    ElementChunkData &d = DataOf(*this, h);
    d.adjusterTemperature = msg.temperature;
    d.adjusterHeatCapacity = msg.heatCapacity;
    d.adjusterRate = msg.thermalConductivity;
}

void ElementChunk::Unregister(Handle handle) {
    CvRemove(*this, handle);
}

// --- RadiationEmitter ---

Handle RadiationEmitter::Register(const SimData &sim, const AddRadiationEmitterMessage &msg) {
    RadiationEmitterData d{};
    d.cell = SimCell(sim, msg.cell);
    d.radiusX = static_cast<uint16_t>(msg.emitRadiusX);
    d.radiusY = static_cast<uint16_t>(msg.emitRadiusY);
    d.intensity = msg.emitRads;
    d.pulseInterval = msg.emitRate;
    // The original takes the smaller of emitSpeed and emitRads here (Modify uses emitRate).
    d.pulseDuration = MinSS(msg.emitSpeed, msg.emitRads);
    d.direction = msg.emitDirection;
    d.angle = msg.emitAngle;
    d.type = msg.emitType;
    return CvAdd(*this, d);
}

void RadiationEmitter::Modify(const SimData &sim, const ModifyRadiationEmitterMessage &msg) {
    uint32_t h = static_cast<uint32_t>(msg.handle);
    if (!IsLive(*this, h)) {
        return;
    }
    RadiationEmitterData &d = DataOf(*this, h);
    d.direction = msg.emitDirection;
    d.angle = msg.emitAngle;
    d.pulseInterval = msg.emitRate;
    d.radiusX = static_cast<uint16_t>(msg.emitRadiusX);
    d.radiusY = static_cast<uint16_t>(msg.emitRadiusY);
    d.intensity = msg.emitRads;
    d.type = msg.emitType;
    d.pulseDuration = MinSS(msg.emitSpeed, msg.emitRate);
    d.cell = SimCell(sim, msg.cell);
}

void RadiationEmitter::Unregister(Handle handle) {
    CvRemove(*this, handle);
}

// --- DiseaseEmitter ---

Handle DiseaseEmitter::Register(const SimData &sim, const AddDiseaseEmitterMessage &msg) {
    (void)sim;
    (void)msg;
    DiseaseEmitterData d{};
    d.cell = -1;
    d.diseaseIdx = kNoDisease;
    return CvAdd(*this, d);
}

void DiseaseEmitter::Modify(const SimData &sim, const ModifyDiseaseEmitterMessage &msg) {
    uint32_t h = static_cast<uint32_t>(msg.handle);
    if (!IsLive(*this, h)) {
        return;
    }
    DiseaseEmitterData &d = DataOf(*this, h);
    d.emitInterval = msg.emitInterval;
    d.diseaseIdx = msg.diseaseIdx;
    d.range = msg.maxDepth;
    d.count = msg.emitCount;
    d.cell = SimCell(sim, msg.gameCell);
}

void DiseaseEmitter::Unregister(Handle handle) {
    CvRemove(*this, handle);
}

// --- DiseaseConsumer (never sent by the game) ---


Handle DiseaseConsumer::Register(const SimData &sim, const AddDiseaseConsumerMessage &msg) {
    (void)sim;
    (void)msg;
    return CvAdd(*this, DiseaseConsumerData{});
}

void DiseaseConsumer::Modify(const SimData &sim, const ModifyDiseaseConsumerMessage &msg) {
    (void)sim;
    (void)msg;
}

void DiseaseConsumer::Unregister(Handle handle) {
    CvRemove(*this, handle);
}

// --- BuildingHeatExchange ---

namespace {

// InitializeFromMessageData: the building's heat exchange data from an Add/Modify message (the
// message without its leading handle or callback). The asserts here do not check
// gAssertsDisabled, as in the original.
template <typename Message>
BuildingHeatExchangeData FromMessage(const Message &msg) {
    const ElementTemperatureData &e = gElementTemperatureData[msg.elemIdx];
    int cells = (msg.maxX - msg.minX) * (msg.maxY - msg.minY);
    if (cells < 1) {
        AssertFunc("BREAKPT:", 0x23, "Components/BuildingHeatExchange.cpp");
        KCrashReporterReportMessage("Components/BuildingHeatExchange.cpp", 0x23,
                                    "Size zero building passed into BuildingHeatExchange");
    }
    if (msg.temperature > 10000.0f || 0.0f > msg.temperature) {
        AssertFunc("BREAKPT:", 0x24, "Components/BuildingHeatExchange.cpp");
        KCrashReporterReportMessage("Components/BuildingHeatExchange.cpp", 0x24,
                                    "Invalid temperature passed into BuildingHeatExchange");
    }
    BuildingHeatExchangeData d{};
    d.temperature = msg.temperature;
    d.overheatTemperature = msg.overheatTemperature;
    d.operatingKilowatts = msg.operatingKilowatts;
    d.heatCapacity = msg.mass * e.specificHeatCapacity;
    d.conductivity = msg.thermalConductivity * e.thermalConductivity;
    d.meltTemperature = e.highTemp;
    d.area.minX = msg.minX + 1;
    d.area.minY = msg.minY + 1;
    d.area.maxX = msg.maxX + 1;
    d.area.maxY = msg.maxY + 1;
    d.heatCapacityPerCell = d.heatCapacity / static_cast<float>(cells);
    return d;
}

}  // namespace

Handle BuildingHeatExchange::Register(const SimData &sim, const AddBuildingHeatExchangeMessage &msg) {
    (void)sim;
    return CvAdd(*this, FromMessage(msg));
}

// The handle travels in the message's first field (named callbackIdx in the C# struct).
void BuildingHeatExchange::Modify(const SimData &sim, const ModifyBuildingHeatExchangeMessage &msg) {
    BuildingHeatExchangeData updated = FromMessage(msg);
    uint32_t h = static_cast<uint32_t>(msg.callbackIdx);
    AssertLive(*this, h);
    BuildingHeatExchangeData &d = DataOf(*this, h);
    if (d.overheatTemperature <= d.temperature && updated.temperature < d.overheatTemperature) {
        sim.events->buildingNoLongerOverheated.push_back(MeltedInfo{static_cast<int32_t>(h)});
    }
    AssertLive(*this, h);
    DataOf(*this, h) = updated;
}

void BuildingHeatExchange::Unregister(Handle handle) {
    uint32_t h = handle.value;
    AssertLive(*this, h);
    uint32_t index = h & 0xffffff;
    BuildingHeatExchangeData &d = DataOf(*this, h);
    d.temperature = -1.0f;
    d.area.minX = 0;
    d.area.minY = 0;
    d.area.maxX = 0;
    d.area.maxY = 0;
    d.heatCapacityPerCell = -1.0f;
    d.conductivity = -1.0f;
    temperatures[index].temperature = -1.0f;
    CvRemove(*this, handle);
}

// --- BuildingToBuildingHeatExchange ---

// Registers a building (by its BuildingHeatExchange handle) for heat exchange with the buildings
// it touches. Reports the new handle to the callback itself, and ProcessFrame reports it again.
Handle BuildingToBuildingHeatExchange::Register(const SimData &sim,
                                                const RegisterBuildingToBuildingHeatExchangeMessage &msg) {
    uint32_t building = static_cast<uint32_t>(msg.structureTemperatureHandler);
    if (!IsLive(sim.buildingHeatExchange, building)) {
        if (msg.callbackIdx != -1) {
            sim.events->componentStateChanged.push_back(ComponentStateChangedMessage{msg.callbackIdx, -1});
        }
        return Handle(0xffffffffu);
    }
    BuildingToBuildingHeatExchangeData d{};
    d.handle = building;
    Handle handle = CvAdd(*this, d);
    if (msg.callbackIdx != -1) {
        sim.events->componentStateChanged.push_back(
            ComponentStateChangedMessage{msg.callbackIdx, static_cast<int32_t>(handle.value)});
    }
    return handle;
}

void BuildingToBuildingHeatExchange::Add(
    const SimData &sim, const AddInContactBuildingToBuildingToBuildingHeatExchangeMessage &msg) {
    (void)sim;
    uint32_t h = static_cast<uint32_t>(msg.selfHandler);
    AssertLive(*this, h);
    BuildingContact contact;
    contact.handle = static_cast<uint32_t>(msg.buildingInContactHandle);
    contact.cellsInContact = msg.cellsInContact;
    std::vector<BuildingContact> contacts = DataOf(*this, h).contacts;
    contacts.push_back(contact);
    AssertLive(*this, h);
    DataOf(*this, h).contacts = contacts;
}

void BuildingToBuildingHeatExchange::Remove(
    const SimData &sim, const RemoveBuildingInContactFromBuildingToBuildingHeatExchangeMessage &msg) {
    (void)sim;
    uint32_t h = static_cast<uint32_t>(msg.selfHandler);
    AssertLive(*this, h);
    std::vector<BuildingContact> contacts = DataOf(*this, h).contacts;
    uint32_t gone = static_cast<uint32_t>(msg.buildingNoLongerInContactHandler);
    for (auto it = contacts.begin(); it != contacts.end(); ++it) {
        if (it->handle == gone) {
            contacts.erase(it);
            AssertLive(*this, h);
            DataOf(*this, h).contacts = contacts;
            return;
        }
    }
}

void BuildingToBuildingHeatExchange::Unregister(Handle handle) {
    uint32_t h = handle.value;
    AssertLive(*this, h);
    DataOf(*this, h).handle = 0;
    CvRemove(*this, handle);
}
