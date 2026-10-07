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
Handle Add(Component &c, const T &value) {
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
void Remove(Component &c, Handle handle) {
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
    return Add(*this, d);
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
    Remove(*this, handle);
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
    Handle handle = Add(*this, d);
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
    Remove(*this, handle);
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
    return Add(*this, d);
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
    Remove(*this, handle);
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
    return Add(*this, d);
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
    Remove(*this, handle);
}

// --- DiseaseEmitter ---

Handle DiseaseEmitter::Register(const SimData &sim, const AddDiseaseEmitterMessage &msg) {
    (void)sim;
    (void)msg;
    DiseaseEmitterData d{};
    d.cell = -1;
    d.diseaseIdx = kNoDisease;
    return Add(*this, d);
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
    Remove(*this, handle);
}

// --- DiseaseConsumer (never sent by the game) ---


Handle DiseaseConsumer::Register(const SimData &sim, const AddDiseaseConsumerMessage &msg) {
    (void)sim;
    (void)msg;
    return Add(*this, DiseaseConsumerData{});
}

void DiseaseConsumer::Modify(const SimData &sim, const ModifyDiseaseConsumerMessage &msg) {
    (void)sim;
    (void)msg;
}

void DiseaseConsumer::Unregister(Handle handle) {
    Remove(*this, handle);
}
