// Heat exchange between two neighbouring cells.
//
// Written from the disassembly rather than the decompile: the original switches to double
// precision for the equilibrium and the exchanged energy, and the decompiler shows those as
// floats. Every min/max keeps the operand order of the minsd/maxsd/minss/maxss it replaces.
#include <cmath>

#include "sim_common.h"

namespace {

// Surface area factor of `self` when touching matter in `other_state`.
float SurfaceArea(const ElementTemperatureData &self, uint8_t other_state, float otherwise) {
    switch (other_state) {
    case kLiquid: return self.liquidSurfaceAreaMultiplier;
    case kSolid: return self.solidSurfaceAreaMultiplier;
    case kGas: return self.gasSurfaceAreaMultiplier;
    default: return otherwise;
    }
}

}  // namespace

void UpdateTemperature(SimData &sim, SimEvents &events, int a, int b) {
    (void)events;
    CellSOA &cells = *sim.cells;
    const float *temperature = cells.temperature.data();
    if (0.0f >= temperature[a] && IsNonZero(cells.mass[a])) {
        AssertFunc("BREAKPT:", 0x2a, "UpdateTemperature.cpp");
        KCrashReporterReportMessage("UpdateTemperature.cpp", 0x2a, "Invalid temperature T=%f, M=%f, cell=%d",
                                    static_cast<double>(cells.temperature[a]), static_cast<double>(cells.mass[a]), a);
        temperature = cells.temperature.data();
    }
    if (0.0f >= temperature[b] && IsNonZero(cells.mass[b])) {
        AssertFunc("BREAKPT:", 0x2b, "UpdateTemperature.cpp");
        KCrashReporterReportMessage("UpdateTemperature.cpp", 0x2b, "Invalid temperature T=%f, M=%f, cell=%d",
                                    static_cast<double>(cells.temperature[b]), static_cast<double>(cells.mass[b]), b);
        temperature = cells.temperature.data();
    }
    if (temperature[a] > 10000.0f) {
        AssertFunc("BREAKPT:", 0x2d, "UpdateTemperature.cpp");
        KCrashReporterReportMessage("UpdateTemperature.cpp", 0x2d, "Invalid temperature T=%f, cell=%d",
                                    static_cast<double>(cells.temperature[a]), a);
        temperature = cells.temperature.data();
    }
    float tb = temperature[b];
    if (tb > 10000.0f) {
        AssertFunc("BREAKPT:", 0x2e, "UpdateTemperature.cpp");
        KCrashReporterReportMessage("UpdateTemperature.cpp", 0x2e, "Invalid temperature T=%f, cell=%d",
                                    static_cast<double>(cells.temperature[b]), a);
        temperature = cells.temperature.data();
        tb = temperature[b];
    }

    const ElementTemperatureData &ea = gElementTemperatureData[cells.elementIdx[a]];
    const ElementTemperatureData &eb = gElementTemperatureData[cells.elementIdx[b]];
    const uint8_t *insulation = sim.cells->insulation.data();
    float ia = static_cast<float>(insulation[a]);
    float ib = static_cast<float>(insulation[b]);
    float ia2 = ia * ia;
    float ib2 = ib * ib;
    const float k = 0x1.020306p-16f;  // 1 / 255^2
    float conductivity_a = (ia2 * k) * ea.thermalConductivity;
    float conductivity_b = eb.thermalConductivity * (k * ib2);
    float area = SurfaceArea(ea, StateOf(eb.state), 1.0f);
    switch (StateOf(ea.state)) {
    case kLiquid: area *= eb.liquidSurfaceAreaMultiplier; break;
    case kSolid: area *= eb.solidSurfaceAreaMultiplier; break;
    case kGas: area *= eb.gasSurfaceAreaMultiplier; break;
    default: break;
    }
    float conductance;
    if (ia2 < 65024.9921875f || ib2 < 65024.9921875f) {
        conductance = area * MinSS(conductivity_a, conductivity_b);
    } else {
        // Both cells uninsulated: geometric mean of the two conductivities.
        float log_a = logf(conductivity_a);
        float log_b = logf(conductivity_b);
        conductance = area * expf((log_b + log_a) * 0.5f);
    }

    float ta = temperature[a];
    double ta_d = ta;
    double tb_d = tb;
    double capacity_b = static_cast<float>(cells.mass[b] * eb.specificHeatCapacity);
    double capacity_a = static_cast<float>(cells.mass[a] * ea.specificHeatCapacity);
    double cond = conductance;
    double equilibrium = (capacity_b * tb_d + capacity_a * ta_d) / (capacity_b + capacity_a);
    const double kRate = static_cast<double>(0.2f);
    float new_ta, new_tb;
    if (ta > tb) {
        double diff = ta_d - tb_d;
        double quarter = 0.25 * diff;
        double energy = MinSD((diff * kRate) * cond, quarter * capacity_b);
        energy = MinSD(quarter * capacity_a, energy);
        new_tb = static_cast<float>(MinSD(energy / capacity_b + tb_d, equilibrium));
        new_ta = static_cast<float>(MaxSD(-energy / capacity_a + ta_d, equilibrium));
    } else {
        double diff = tb_d - ta_d;
        double quarter = 0.25 * diff;
        double energy = MinSD((diff * kRate) * cond, quarter * capacity_a);
        energy = MinSD(energy, quarter * capacity_b);
        new_ta = static_cast<float>(MinSD(energy / capacity_a + ta_d, equilibrium));
        new_tb = static_cast<float>(MaxSD(tb_d + -energy / capacity_b, equilibrium));
    }

    // Apply as a delta on top of what other exchanges already wrote this frame.
    float *out = sim.updatedCells->temperature.data();
    out[a] = new_ta + (out[a] - ta);
    out[b] = new_tb + (out[b] - cells.temperature[b]);
    out[a] = MaxSS(MinSS(out[a], 10000.0f), 1.0f);
    out[b] = MaxSS(MinSS(10000.0f, out[b]), 1.0f);
}
