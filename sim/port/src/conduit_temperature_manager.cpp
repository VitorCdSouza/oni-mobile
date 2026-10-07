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

struct ModifyBuildingEnergyMessage {
    uint32_t handle;
    float deltaKJ;
    float minTemperature;
    float maxTemperature;
};

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
                msg.handle = e.structureHandle;
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
