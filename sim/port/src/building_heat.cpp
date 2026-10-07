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
