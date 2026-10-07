// Debris items exchanging heat with the cell they lie in. From the disassembly: the exchanges
// run in double precision.
#include "sim_common.h"

namespace {

// Heat exchange between debris `d` and the cell `at` (FUN_00161520). `scale` multiplies the
// conductance (1 for the item's own cell, groundTransferScale for the floor below). Returns the
// energy the cell received (negative when it gave heat to the item).
float ExchangeWithCell(const CellAccessor &at, ElementChunkData &d, float dt, float scale) {
    CellSOA &soa = *at.soa;
    int cell = at.cell;
    float s = scale * 0.001f;
    float mass = soa.mass[cell];
    if (0.0f >= mass) {
        return 0.0f;
    }
    float item_t = d.temperature;
    float *cell_temperature = &soa.temperature[cell];
    float cell_t = *cell_temperature;
    if (item_t > 10000.0f || 0.0f > item_t) {
        KCrashReporterReportMessage("Components/ElementChunk.cpp", 0x54,
                                    "ElementChunk bad item temperature: %f. Cell T=%f", static_cast<double>(item_t),
                                    static_cast<double>(cell_t));
        return 0.0f;
    }
    const ElementTemperatureData &e = gElementTemperatureData[soa.elementIdx[cell]];
    float cell_capacity = e.specificHeatCapacity * mass;
    if (0.0f > cell_t || cell_t > 10000.0f) {
        KCrashReporterReportMessage("Components/ElementChunk.cpp", 0x5d,
                                    "ElementChunk bad src cell T=%f, HC=%f, Item T=%f, HC=%f",
                                    static_cast<double>(cell_t), static_cast<double>(cell_capacity),
                                    static_cast<double>(item_t), static_cast<double>(d.heatCapacity));
        return 0.0f;
    }
    if (e.state & 0x10) {
        return 0.0f;
    }
    float ins = static_cast<float>(soa.insulation[cell]);
    float conductance = (0x1.020306p-16f * e.thermalConductivity) * (ins * ins);
    conductance = MinSS(conductance, d.thermalConductivity) * d.areaOverThickness;
    double rate = s * conductance;
    double cc = cell_capacity;
    double ic = d.heatCapacity;
    double it = item_t;
    double ct = cell_t;
    double step = dt;
    double equilibrium = (cc * ct + ic * it) / (ic + cc);
    float new_item, new_cell, energy;
    if (cell_t > item_t) {
        double diff = ct - it;
        double e1 = MinSD(diff * ic, (step * diff) * rate);
        double transfer = MinSD(diff * cc, e1);
        new_item = static_cast<float>(MinSD(transfer / ic + it, equilibrium));
        new_cell = static_cast<float>(MaxSD(ct + -transfer / cc, equilibrium));
        energy = -static_cast<float>((it - static_cast<double>(new_item)) * ic);
    } else {
        double diff = it - ct;
        double e1 = MinSD((step * diff) * rate, diff * cc);
        double transfer = MinSD(diff * ic, e1);
        new_cell = static_cast<float>(MinSD(transfer / cc + ct, equilibrium));
        energy = static_cast<float>((ct - static_cast<double>(new_cell)) * cc);
        new_item = static_cast<float>(MaxSD(-transfer / ic + it, equilibrium));
    }
    *cell_temperature = new_cell;
    d.temperature = new_item;
    return energy;
}

}  // namespace

void ElementChunk::Update(float dt, SimData &sim, const Region &region) {
    info.resize(handleToDense.size());
    const double step = dt;
    for (size_t i = 0; i < data.size(); i++) {
        uint32_t handle = denseToHandle[i];
        ElementChunkData &d = data[i];
        int y = d.cell / sim.width;
        int x = d.cell % sim.width;
        float temperature;
        float energy = 0.0f;
        if (!(region.minX <= x && region.minY <= y && x <= region.maxX && y <= region.maxY) ||
            !(d.heatCapacity > 0.0f)) {
            temperature = d.temperature;
        } else if (0.0f >= d.adjusterHeatCapacity) {
            CellAccessor own = Accessor(sim.updatedCells, d.cell);
            energy = ExchangeWithCell(own, d, dt, 1.0f);
            int below = d.cell - sim.width;
            if (StateOf(gElementStateData[sim.updatedCells->elementIdx[below]]) == kSolid) {
                CellAccessor floor = Accessor(sim.updatedCells, below);
                float e2 = ExchangeWithCell(floor, d, dt, d.groundTransferScale);
                energy = e2 + energy;
            }
            temperature = d.temperature;
        } else {
            // A temperature adjuster (e.g. a fridge) pulls the item towards its temperature.
            double cap = d.heatCapacity;
            double adj_cap = d.adjusterHeatCapacity;
            double it = d.temperature;
            double at = d.adjusterTemperature;
            double equilibrium = (cap * it + adj_cap * at) / (cap + adj_cap);
            double rate = static_cast<double>(d.adjusterRate) * step;
            if (d.adjusterTemperature > d.temperature) {
                double diff = at - it;
                double e1 = MinSD(diff * cap, rate * diff);
                double transfer = MinSD(diff * adj_cap, e1);
                temperature = static_cast<float>(MinSD(transfer / cap + it, equilibrium));
            } else {
                double diff = it - at;
                double e1 = MinSD(rate * diff, adj_cap * diff);
                double transfer = MinSD(e1, diff * cap);
                temperature = static_cast<float>(MaxSD(-transfer / cap + it, equilibrium));
            }
            d.temperature = temperature;
        }
        ElementChunkInfo &slot = info[handle & 0xffffff];
        float melt = d.highTemp + 3.0f;
        float delta = energy + slot.deltaKJ;
        slot.temperature = temperature;
        slot.deltaKJ = delta;
        if (temperature >= melt || d.lowTemp - 3.0f > temperature) {
            sim.events->elementChunkMelted.push_back(MeltedInfo{static_cast<int32_t>(handle)});
        }
    }
}
