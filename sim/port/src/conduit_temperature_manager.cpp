// Heat exchange between pipe contents and the pipe structures (ConduitTemperatureManager_*).
#include <pthread.h>

#include "sim_common.h"

namespace {

float AbsSS(float x) {
    uint32_t bits;
    __builtin_memcpy(&bits, &x, 4);
    bits &= 0x7fffffffu;
    __builtin_memcpy(&x, &bits, 4);
    return x;
}

constexpr int kModifyBuildingEnergy = static_cast<int>(0xaf9b1296u);

}  // namespace

// SimFrameManager::HandleMessage queues a message for the next frame; the frame manager lives at
// offset 0x40 of the object gSim points to.
extern char *gSim;

// Energy flowing between the contents and the conduit in one step.
// insulated: use the smaller of the two conductivities instead of their average.
float ConduitTemperatureManager::CalculateEnergyFlow(bool insulated, float contents_temperature,
                                                     float contents_conductivity, float conduit_temperature,
                                                     float conduit_conductivity, float scale) {
    float flow = (contents_temperature - conduit_temperature) * scale;
    if (insulated) {
        // minss semantics: the second operand wins unless the first is strictly smaller.
        float conductivity = contents_conductivity < conduit_conductivity ? contents_conductivity
                                                                          : conduit_conductivity;
        return flow * conductivity;
    }
    return flow * (contents_conductivity + conduit_conductivity) * 0.5f;
}

// Limits the energy so that neither side overshoots the other's temperature.
float ConduitTemperatureManager::ClampEnergyTransfer(float dt, float src_temperature, float src_capacity,
                                                     float dst_temperature, float dst_capacity, float energy) {
    float e = (dt * 0.001f) * energy;
    float hi = MaxSS(src_temperature, dst_temperature);
    float lo = MinSS(src_temperature, dst_temperature);
    float src_limit = AbsSS(MaxSS(MinSS(src_temperature - e / src_capacity, hi), lo) - src_temperature) * src_capacity;
    float dst_limit = AbsSS(MaxSS(MinSS(e / dst_capacity + dst_temperature, hi), lo) - dst_temperature) * dst_capacity;
    float sign = 0.0f > energy ? -1.0f : 1.0f;
    return MinSS(dst_limit, src_limit) * sign;
}

float ConduitTemperatureManager::ModifyTemperature(float temperature, float capacity, float other_temperature,
                                                   float other_capacity, float energy) {
    float mine = MaxSS(energy / capacity + temperature, 0.0f);
    float other = MaxSS(other_temperature - energy / other_capacity, 0.0f);
    if (0.0f > (mine - other) * (temperature - other_temperature)) {
        float sum = capacity + other_capacity;
        return (other_capacity / sum) * other_temperature + (capacity / sum) * temperature;
    }
    return mine;
}

ConduitTemperatureUpdateData *ConduitTemperatureManager::Update(float dt, const BuildingTemperatureInfo *buildings) {
    pthread_mutex_lock(reinterpret_cast<pthread_mutex_t *>(mutex));
    size_t n = data.size();
    melted.clear();
    frozen.clear();
    temperatures.resize(handleToDense.size());
    if (buildings == nullptr) {
        for (size_t i = 0; i < n; i++) {
            uint64_t index = denseToHandle[i] & 0xffffff;
            if (temperatures.size() <= index) {
                AssertFunc("BREAKPT:", 0x111, "ConduitTemperatureManager.cpp");
                KCrashReporterReportMessage("ConduitTemperatureManager.cpp", 0x111,
                                            "Assert failed: sim_handle.Index() < temperatures.size()");
            }
            temperatures[index] = data[i].temperature;
        }
    } else {
        for (size_t i = 0; i < n; i++) {
            ConduitTemperatureData &e = data[i];
            if (e.structureHandle & 0xf00000) {
                continue;
            }
            float src = e.temperature;
            float building = 0.0f;
            bool exchange = !(e.contentsHeatCapacity <= 0.0001f || e.conduitHeatCapacity <= 0.0001f);
            if (exchange) {
                building = buildings[e.structureHandle & 0xffffff].temperature;
                exchange = !(building <= 0.0f);
            }
            if (!exchange) {
                temperatures[denseToHandle[i] & 0xffffff] = src;
                continue;
            }
            if (src < 0.0f || 10000.0f < src) {
                AssertFunc("BREAKPT:", 0xc1, "ConduitTemperatureManager.cpp");
                KCrashReporterReportMessage(
                    "ConduitTemperatureManager.cpp", 0xc1,
                    "Assert failed: 0.0f <= src_temperature && src_temperature <= SIM_MAX_TEMPERATURE");
            }
            if (10000.0f < building || building < 0.0f) {
                AssertFunc("BREAKPT:", 0xc2, "ConduitTemperatureManager.cpp");
                KCrashReporterReportMessage(
                    "ConduitTemperatureManager.cpp", 0xc2,
                    "Assert failed: 0.0f <= dest_temperature && dest_temperature <= SIM_MAX_TEMPERATURE");
            }
            float flow = CalculateEnergyFlow(e.insulated, src, e.contentsConductivity, building, e.conduitConductivity,
                                             50.0f);
            float clamped = ClampEnergyTransfer(dt, src, e.contentsHeatCapacity, building, e.conduitHeatCapacity, flow);
            float updated = ModifyTemperature(e.temperature, e.contentsHeatCapacity,
                                              buildings[e.structureHandle & 0xffffff].temperature,
                                              e.conduitHeatCapacity, -clamped);
            e.temperature = updated;
            if (e.temperature <= 0.0f) {
                AssertFunc("BREAKPT:", 0xeb, "ConduitTemperatureManager.cpp");
                KCrashReporterReportMessage("ConduitTemperatureManager.cpp", 0xeb,
                                            "Assert failed: 0.0f < entry.temperature");
            }
            if (10000.0f < e.temperature) {
                AssertFunc("BREAKPT:", 0xec, "ConduitTemperatureManager.cpp");
                KCrashReporterReportMessage("ConduitTemperatureManager.cpp", 0xec,
                                            "Assert failed: entry.temperature <= SIM_MAX_TEMPERATURE");
            }
            uint32_t index = denseToHandle[i] & 0xffffff;
            if (temperatures.size() <= index) {
                AssertFunc("BREAKPT:", 0xef, "ConduitTemperatureManager.cpp");
                KCrashReporterReportMessage("ConduitTemperatureManager.cpp", 0xef,
                                            "Assert failed: sim_handle.Index() < temperatures.size()");
            }
            temperatures[index] = updated;
            if (1e-06f < AbsSS(clamped)) {
                // The pipe building gets the energy the contents gave away.
                ModifyBuildingEnergyMessage msg;
                msg.handle = static_cast<int32_t>(e.structureHandle);
                msg.deltaKJ = (src - updated) * e.contentsHeatCapacity;
                msg.minTemperature = MinSS(building, src);
                msg.maxTemperature = MaxSS(building, src);
                void *out;
                reinterpret_cast<SimFrameManager *>(gSim + 0x40)
                    ->HandleMessage(kModifyBuildingEnergy, sizeof msg, &msg, out);
            }
            if (e.lowTemp - 3.0f <= updated) {
                if (e.highTemp + 3.0f < updated) {
                    melted.push_back(static_cast<int32_t>(index));
                }
            } else {
                frozen.push_back(static_cast<int32_t>(index));
            }
        }
    }
    result.numEntries = static_cast<int32_t>(temperatures.size());
    result.temperatures = temperatures.empty() ? nullptr : temperatures.data();
    result.numFrozenHandles = static_cast<int32_t>(frozen.size());
    result.frozenHandles = frozen.empty() ? nullptr : frozen.data();
    result.numMeltedHandles = static_cast<int32_t>(melted.size());
    result.meltedHandles = melted.empty() ? nullptr : melted.data();
    pthread_mutex_unlock(reinterpret_cast<pthread_mutex_t *>(mutex));
    return &result;
}

namespace {

struct MutexLock {
    explicit MutexLock(uint8_t *m) : mutex(reinterpret_cast<pthread_mutex_t *>(m)) { pthread_mutex_lock(mutex); }
    ~MutexLock() { pthread_mutex_unlock(mutex); }
    pthread_mutex_t *mutex;
};

// The element's low/high transition temperatures, or 0 / FLT_MAX when it has none.
void SetContents(ConduitTemperatureData &d, const Element &e, float temperature, float mass) {
    d.temperature = temperature;
    d.contentsConductivity = e.thermalConductivity;
    d.contentsHeatCapacity = mass * e.specificHeatCapacity;
    d.lowTemp = e.lowTempTransitionIdx != kInvalidElement ? e.lowTemp : 0.0f;
    d.highTemp = e.highTempTransitionIdx != kInvalidElement ? e.highTemp : 0x1.fffffep+127f;
}

void CheckContents(const Element &e, float temperature, float mass, int line_temperature, int line_conductivity) {
    if (mass == 0.0f) {
        return;
    }
    if (temperature < 0.0f) {
        AssertFunc("BREAKPT:", line_temperature, "ConduitTemperatureManager.cpp");
        KCrashReporterReportMessage("ConduitTemperatureManager.cpp", line_temperature,
                                    "Assert failed: contents_mass == 0.0f || 0.0f <= contents_temperature");
    }
    if (e.thermalConductivity < 0.0f) {
        AssertFunc("BREAKPT:", line_conductivity, "ConduitTemperatureManager.cpp");
        KCrashReporterReportMessage("ConduitTemperatureManager.cpp", line_conductivity,
                                    "Assert failed: contents_mass == 0.0f || 0.0f <= "
                                    "contents_elem.thermalConductivity");
    }
}

}  // namespace

// A pipe segment's contents. Returns its handle.
uint32_t ConduitTemperatureManager::Add(float temperature, float mass, int element_hash, int structure_handle,
                                        float conduit_heat_capacity, float conduit_conductivity, bool insulated) {
    MutexLock lock(mutex);
    const Element &e = gElements[GetElementIndex(static_cast<uint32_t>(element_hash))];
    CheckContents(e, temperature, mass, 0x30, 0x31);
    if (temperature > 10000.0f) {
        KCrashReporterReportMessage("ConduitTemperatureManager.cpp", 0x35,
                                    "Contents have invalid temperature %f. Resetting to default.",
                                    static_cast<double>(temperature));
        temperature = e.defaultValues.temperature;
    }
    ConduitTemperatureData d{};
    SetContents(d, e, temperature, mass);
    d.structureHandle = static_cast<uint32_t>(structure_handle);
    d.insulated = insulated;
    d.conduitHeatCapacity = conduit_heat_capacity;
    d.conduitConductivity = conduit_conductivity;
    int32_t dense = static_cast<int32_t>(data.size());
    uint32_t handle;
    if (freeHandles.empty()) {
        versions.push_back(0);
        handle = static_cast<uint32_t>(handleToDense.size()) & 0xffffff;
        handleToDense.push_back(dense);
    } else {
        handle = freeHandles.back();
        freeHandles.pop_back();
        handleToDense[handle & 0xffffff] = dense;
    }
    denseToHandle.push_back(handle);
    data.push_back(d);
    return handle;
}

// New contents for a pipe segment (bad handles are reported and ignored).
void ConduitTemperatureManager::Set(int handle, float temperature, float mass, int element_hash) {
    MutexLock lock(mutex);
    uint32_t h = static_cast<uint32_t>(handle);
    uint32_t index = h & 0xffffff;
    if ((h & 0xf00000) != 0 || index >= versions.size() || versions[index] != static_cast<uint8_t>(h >> 24)) {
        KCrashReporterReportMessage("ConduitTemperatureManager.cpp", 99,
                                    "ConduitTemperatureManager::Set called with bad handle: V=%d, Idx=%d, ArraySize=%d",
                                    static_cast<int>(index), static_cast<int>(h >> 24), static_cast<int>(data.size()));
        return;
    }
    ConduitTemperatureData &d = data[static_cast<size_t>(handleToDense[index])];
    const Element &e = gElements[GetElementIndex(static_cast<uint32_t>(element_hash))];
    CheckContents(e, temperature, mass, 0x6b, 0x6c);
    if (temperature > 10000.0f) {
        AssertFunc("BREAKPT:", 0x6d, "ConduitTemperatureManager.cpp");
        KCrashReporterReportMessage("ConduitTemperatureManager.cpp", 0x6d,
                                    "Assert failed: contents_temperature <= SIM_MAX_TEMPERATURE");
        KCrashReporterReportMessage("ConduitTemperatureManager.cpp", 0x71,
                                    "Contents have invalid temperature %f. Resetting to default.",
                                    static_cast<double>(temperature));
        temperature = e.defaultValues.temperature;
    }
    SetContents(d, e, temperature, mass);
}

// Marks the segment dead now and queues its handle; ReleaseQueuedHandles frees it a step later.
void ConduitTemperatureManager::Remove(int handle) {
    MutexLock lock(mutex);
    uint32_t h = static_cast<uint32_t>(handle);
    uint32_t index = h & 0xffffff;
    if (((h & 0xf00000) != 0 || index >= versions.size() || versions[index] != static_cast<uint8_t>(h >> 24)) &&
        !gAssertsDisabled) {
        AssertFunc("BREAKPT:", 0xcf, "./util/HandleVector.h");
    }
    ConduitTemperatureData &d = data[static_cast<size_t>(handleToDense[index])];
    d.structureHandle = 0xffffffffu;
    d.conduitHeatCapacity = -1.0f;
    d.conduitConductivity = -1.0f;
    (pendingIndex == 0 ? pendingRemovals0 : pendingRemovals1).push_back(h);
}

void ConduitTemperatureManager::Clear() {
    MutexLock lock(mutex);
    denseToHandle.clear();
    data.clear();
    handleToDense.clear();
    freeHandles.clear();
    versions.clear();
}

// Called by the sim thread before each step: flips the pending list and frees the handles
// queued in the list it lands on (removed during the step before last).
void ConduitTemperatureManager::ReleaseQueuedHandles() {
    MutexLock lock(mutex);
    pendingIndex = (pendingIndex + 1) % 2;
    std::vector<uint32_t> &pending = pendingIndex == 0 ? pendingRemovals0 : pendingRemovals1;
    for (uint32_t h : pending) {
        if ((h & 0xf00000) != 0) {
            continue;
        }
        uint32_t index = h & 0xffffff;
        int version = (static_cast<int32_t>(h) >> 24) + 1;
        versions[index] = static_cast<uint8_t>(version);
        freeHandles.push_back(static_cast<uint32_t>(version) << 24 | index);
        int32_t dense = handleToDense[index];
        handleToDense[index] = 0;
        int last = static_cast<int>(data.size()) - 1;
        if (dense < last) {
            data[static_cast<size_t>(dense)] = data[static_cast<size_t>(last)];
            uint32_t moved = denseToHandle[static_cast<size_t>(last)];
            handleToDense[moved & 0xffffff] = dense;
            denseToHandle[static_cast<size_t>(dense)] = moved;
        }
        denseToHandle.pop_back();
        data.pop_back();
    }
    pending.clear();
}
