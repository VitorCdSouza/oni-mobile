// Heat exchange between buildings and the cells under them. Written from the disassembly: the
// decompile shows the long product in the wrong association.
#include "sim_common.h"

void BuildingHeatExchange::Update(float dt, SimData &sim, const Region &region) {
    size_t handles = handleToDense.size();
    temperatures.resize(handles);
    const float dt_rate = dt * 0x1.4a414ep-24f;  // 7.689351e-08
    for (size_t i = 0; i < data.size(); i++) {
        BuildingHeatExchangeData &b = data[i];
        const Region &a = b.area;
        if (!(region.minX <= a.minX && region.minY <= a.minY && a.maxX <= region.maxX && a.maxY <= region.maxY)) {
            continue;
        }
        float capacity_per_cell = b.heatCapacityPerCell;
        float start = b.temperature;
        uint32_t handle = denseToHandle[i];
        float result = start;
        if (0.0f < capacity_per_cell && 0.0f <= start) {
            int w = a.maxX - a.minX;
            int h = a.maxY - a.minY;
            float lowest = start;
            float highest = start;
            float exchanged = 0.0f;
            for (int y = a.minY; y < a.maxY; y++) {
                int row = y * sim.width + a.minX;
                for (int c = row; c < row + w; c++) {
                    CellSOA &upd = *sim.updatedCells;
                    float mass = upd.mass[c];
                    if (!(mass > 0.0f)) {
                        continue;
                    }
                    const ElementTemperatureData &e = gElementTemperatureData[upd.elementIdx[c]];
                    float capacity = mass * e.specificHeatCapacity;
                    float ins = static_cast<float>(upd.insulation[c]);
                    float t = upd.temperature[c];
                    float ins2 = ins * ins;
                    float diff = t - start;
                    // The colder side's capacity limits the exchange.
                    float limiting = diff < 0.0f ? capacity_per_cell : capacity;
                    float energy = (ins2 * (dt_rate * sim.unk_0xa0)) *
                                   ((limiting * e.thermalConductivity) * (b.conductivity * diff));
                    float hi, lo;
                    if (t > start) {
                        hi = t;
                        lo = start;
                    } else {
                        hi = start;
                        lo = t;
                    }
                    lowest = MinSS(lowest, lo);
                    highest = MaxSS(highest, hi);
                    float new_t = MaxSS(MinSS(t - energy / capacity, hi), lo);
                    float new_b = MaxSS(MinSS(energy / capacity_per_cell + start, hi), lo);
                    if (0.0f > (new_b - new_t) * (start - t)) {
                        // Overshoot: settle both at the weighted equilibrium.
                        float sum = capacity + capacity_per_cell;
                        new_t = MaxSS(MinSS((capacity / sum) * t + (capacity_per_cell / sum) * start, hi), lo);
                        energy = (new_t - start) * capacity_per_cell;
                    }
                    if (!(hi >= new_t) || !(new_t >= lo)) {
                        KCrashReporterReportMessage(
                            "Components/BuildingHeatExchange.cpp", 0xe9,
                            "BuildingHeatExchange has invalid values Cell T=%f, HC=%f, Bldg T=%f, HC=%f",
                            static_cast<double>(t), static_cast<double>(capacity), static_cast<double>(b.temperature),
                            static_cast<double>(capacity_per_cell));
                    } else {
                        if (0.0f > new_t || new_t > 10000.0f) {
                            AssertFunc("BREAKPT:", 0xe0, "Components/BuildingHeatExchange.cpp");
                            KCrashReporterReportMessage(
                                "Components/BuildingHeatExchange.cpp", 0xe0,
                                "BuildingHeatExchange has generated invalid final temperature of %f. Src Values: "
                                "Cell T=%f, HC=%f, Bldg T=%f, HC=%f",
                                static_cast<double>(new_t), static_cast<double>(upd.temperature[c]),
                                static_cast<double>(capacity), static_cast<double>(b.temperature),
                                static_cast<double>(b.heatCapacityPerCell));
                        }
                        upd.temperature[c] = new_t;
                        DoStateTransition(sim, *sim.events, c, e);
                        exchanged = energy + exchanged;
                    }
                    capacity_per_cell = b.heatCapacityPerCell;
                }
            }
            float total = capacity_per_cell * static_cast<float>(h * w);
            float mixed = MinSS(exchanged / total + b.temperature, highest);
            float settled = MaxSS(lowest, mixed);
            result = (dt * b.operatingKilowatts) / total + settled;
            if (0.0f > result || result > 10000.0f) {
                AssertFunc("BREAKPT:", 0xf5, "Components/BuildingHeatExchange.cpp");
                KCrashReporterReportMessage(
                    "Components/BuildingHeatExchange.cpp", 0xf5,
                    "Assert failed: 0 <= final_temperature && final_temperature <= SIM_MAX_TEMPERATURE");
            }
            float overheat = b.overheatTemperature;
            if (result >= overheat) {
                sim.events->buildingOverheat.push_back(MeltedInfo{static_cast<int32_t>(handle)});
            }
            if (start >= overheat && overheat > result) {
                sim.events->buildingNoLongerOverheated.push_back(MeltedInfo{static_cast<int32_t>(handle)});
            }
            if (result >= b.meltTemperature) {
                sim.events->buildingMelted.push_back(MeltedInfo{static_cast<int32_t>(handle)});
            }
            b.temperature = result;
        }
        BuildingTemperatureInfo &info = temperatures[handle & 0xffffff];
        info.handle = handle;
        info.temperature = result;
    }
}

namespace {

// HandleVector lookups on the BuildingHeatExchange component.
bool IsValidHandle(const BuildingHeatExchange &bhe, uint32_t handle) {
    uint64_t index = handle & 0xffffff;
    return (handle & 0xf00000) == 0 && index < bhe.versions.size() &&
           static_cast<int8_t>(bhe.versions[index]) == static_cast<int8_t>(static_cast<int32_t>(handle) >> 24);
}

BuildingHeatExchangeData &DataFor(BuildingHeatExchange &bhe, uint32_t handle) {
    return bhe.data[static_cast<size_t>(bhe.handleToDense[handle & 0xffffff])];
}

// BuildingHeatExchange::GetSimHandleForBuildingHandle + ChangeBuildingTemperature.
void ChangeBuildingTemperature(BuildingHeatExchange &bhe, uint32_t handle, float temperature) {
    if (!IsValidHandle(bhe, handle) && !gAssertsDisabled) {
        AssertFunc("BREAKPT:", 0xcf, "../util/HandleVector.h");
    }
    uint32_t sim_handle = bhe.denseToHandle[static_cast<size_t>(bhe.handleToDense[handle & 0xffffff])];
    BuildingTemperatureInfo &info = bhe.temperatures[sim_handle & 0xffffff];
    info.handle = sim_handle;
    info.temperature = temperature;
}

uint32_t SimHandleFor(BuildingHeatExchange &bhe, uint32_t handle) {
    if (!IsValidHandle(bhe, handle) && !gAssertsDisabled) {
        AssertFunc("BREAKPT:", 0xcf, "../util/HandleVector.h");
    }
    return bhe.denseToHandle[static_cast<size_t>(bhe.handleToDense[handle & 0xffffff])];
}

float AbsSS(float x) {
    uint32_t bits;
    __builtin_memcpy(&bits, &x, 4);
    bits &= 0x7fffffffu;
    __builtin_memcpy(&x, &bits, 4);
    return x;
}

}  // namespace

// Heat exchange between buildings that touch each other (from the disassembly).
void BuildingToBuildingHeatExchange::Update(float dt, SimData &sim, const Region &region) {
    BuildingHeatExchange &bhe = sim.buildingHeatExchange;
    const float dt_rate = 0x1.47ae14p-8f * dt;  // 0.005 * dt
    for (size_t i = 0; i < data.size(); i++) {
        BuildingToBuildingHeatExchangeData &entry = data[i];
        if (!IsValidHandle(bhe, entry.handle)) {
            continue;
        }
        BuildingHeatExchangeData &self = DataFor(bhe, entry.handle);
        const Region &a = self.area;
        if (!(a.minX >= region.minX && a.minY >= region.minY && a.maxX <= region.maxX && a.maxY <= region.maxY)) {
            continue;
        }
        float capacity = self.heatCapacity;
        float lo = self.temperature;
        float hi = self.temperature;
        if (entry.contacts.empty()) {
            continue;
        }
        for (const BuildingContact &c : entry.contacts) {
            if (IsValidHandle(bhe, c.handle)) {
                float t = DataFor(bhe, c.handle).temperature;
                lo = MinSS(lo, t);
                hi = MaxSS(hi, t);
            }
        }
        const float inverse_capacity = 1.0f / capacity;
        for (size_t k = 0; k < entry.contacts.size(); k++) {
            uint32_t contact = entry.contacts[k].handle;
            if (!IsValidHandle(bhe, contact)) {
                continue;
            }
            BuildingHeatExchangeData &other = DataFor(bhe, contact);
            float other_capacity = other.heatCapacity;
            if (0.0f >= other_capacity) {
                continue;
            }
            float t_other = other.temperature;
            float t_self = self.temperature;
            float diff = t_other - t_self;
            float limiting = diff < 0.0f ? capacity : other_capacity;
            float energy = ((dt_rate * sim.unk_0xa4) * (self.conductivity * other.conductivity)) * (limiting * diff);
            float self_limit = AbsSS(MaxSS(MinSS(inverse_capacity * energy + t_self, hi), lo) - t_self) * capacity;
            float other_limit =
                AbsSS(MaxSS(MinSS(t_other - energy / other_capacity, hi), lo) - t_other) * other_capacity;
            float applied = MinSS(other_limit, self_limit);
            if (0.0f > energy) {
                applied = -applied;
            }
            float new_self = inverse_capacity * applied + t_self;
            float new_other;
            float other_delta = applied / other_capacity;
            if (0.0f > ((other_delta - t_other) + new_self) * (t_self - t_other)) {
                float mixed = (t_other * other_capacity + t_self * capacity) / (other_capacity + capacity);
                new_self = MaxSS(MinSS(mixed, hi), lo);
                new_other = new_self;
            } else {
                new_other = t_other - other_delta;
            }
            uint32_t sim_handle = SimHandleFor(bhe, contact);
            float overheat = other.overheatTemperature;
            if (new_other >= overheat) {
                sim.events->buildingOverheat.push_back(MeltedInfo{static_cast<int32_t>(sim_handle)});
            }
            if (!(overheat > other.temperature) && overheat > new_other) {
                sim.events->buildingNoLongerOverheated.push_back(MeltedInfo{static_cast<int32_t>(sim_handle)});
            }
            other.temperature = new_other;
            ChangeBuildingTemperature(bhe, contact, new_other);
            self.temperature = new_self;
            ChangeBuildingTemperature(bhe, entry.handle, new_self);
        }
    }
}
