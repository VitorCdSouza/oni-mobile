// The element table the game sends at load (CreateSimElementsTable in C#) and the per-element
// tables the sim derives from it.
#include <cfloat>
#include <cmath>
#include <string>
#include <unordered_map>
#include <vector>

#include "binary_buffer.h"
#include "sim_common.h"

extern std::unordered_map<uint32_t, uint16_t> gElementIndices;
extern std::vector<std::string> gElementNames;

namespace {

constexpr uint32_t kNoOre = 0x2d39bf75;  // the "no transition ore" element hash

}  // namespace

void DestroyElementsTable() {
    gElementNames.clear();
    gElements.clear();
    gElementTemperatureData.clear();
    gElementPostProcessData.clear();
    gElementLiquidData.clear();
    gElementPhysicsData.clear();
    gElementPressureData.clear();
    gElementPropertyTextureData.clear();
    gElementLightAbsorptionData.clear();
    gElementRadiationData.clear();
    gElementStateData.clear();
    gElementIndices.clear();
}

// Message: int count, count Element records (0xa4 bytes, Pack = 4), then count names (int length
// and the bytes). Returns the count.
long CreateElementsTable(BinaryBufferReader &reader) {
    DestroyElementsTable();
    int count = reader.Read<int32_t>();
    const uint8_t *records = reader.Current();
    if (count > 0xfffe && !gAssertsDisabled) {
        AssertFunc("BREAKPT:", 0xb2, "Elements.cpp");
    }
    size_t n = static_cast<size_t>(static_cast<long>(count));
    gElementNames.resize(n);
    gElements.resize(n);
    gElementTemperatureData.resize(n);
    gElementPostProcessData.resize(n);
    gElementLiquidData.resize(n);
    gElementPhysicsData.resize(n);
    gElementPressureData.resize(n);
    gElementPropertyTextureData.resize(n);
    gElementLightAbsorptionData.resize(n);
    gElementRadiationData.resize(n);
    gElementStateData.resize(n);
    if (n != 0) {
        std::memcpy(gElements.data(), records, n * sizeof(Element));
    }
    reader.SkipBytes(n * sizeof(Element));
    for (uint32_t i = 0; i < gElements.size() && i != 0xffff; i++) {
        gElementIndices[static_cast<uint32_t>(gElements[i].id)] = static_cast<uint16_t>(i);
    }
    for (int i = 0; i < count; i++) {
        int length = reader.Read<int32_t>();
        gElementNames[static_cast<size_t>(i)].assign(reinterpret_cast<const char *>(reader.Current()),
                                                     static_cast<size_t>(length));
        reader.SkipBytes(static_cast<uint64_t>(static_cast<long>(length)));
    }
    for (int i = 0; i < count; i++) {
        Element e;
        std::memcpy(&e, records + static_cast<size_t>(i) * sizeof(Element), sizeof e);
        uint8_t state = e.state;

        ElementTemperatureData &t = gElementTemperatureData[static_cast<size_t>(i)];
        t.specificHeatCapacity = e.specificHeatCapacity;
        t.thermalConductivity = e.thermalConductivity;
        t.state = state;
        t.defaultMass = e.defaultValues.mass;
        t.minMassForTemperature = StateOf(state) == kGas ? 1.0f : 0.001f;
        t.lowTempTransitionIdx = e.lowTempTransitionIdx;
        t.lowTemp = e.lowTemp;
        t.highTempTransitionIdx = e.highTempTransitionIdx;
        t.highTemp = e.highTemp;
        t.lowTempTransitionOreMassConversion = e.lowTempTransitionOreMassConversion;
        t.highTempTransitionOreMassConversion = e.highTempTransitionOreMassConversion;
        t.solidSurfaceAreaMultiplier = e.solidSurfaceAreaMultiplier;
        t.liquidSurfaceAreaMultiplier = e.liquidSurfaceAreaMultiplier;
        t.gasSurfaceAreaMultiplier = e.gasSurfaceAreaMultiplier;

        ElementPostProcessData &p = gElementPostProcessData[static_cast<size_t>(i)];
        p.state = state;
        p.sublimateRate = e.sublimateRate;
        p.sublimateEfficiency = e.sublimateEfficiency;
        p.sublimateProbability = e.sublimateProbability;
        p.offGasProbability = e.offGasProbability;
        p.sublimateIndex = e.sublimateIndex;
        p.convertIndex = e.convertIndex;
        p.molarMass = e.molarMass;
        p.maxMass = e.maxMass;
        p.strength = e.strength;
        p.sublimateFX = e.sublimateFX;
        p.minHorizontalFlow = e.minHorizontalFlow;

        ElementLiquidData &l = gElementLiquidData[static_cast<size_t>(i)];
        l.state = state;
        l.minHorizontalFlow = e.minHorizontalFlow;
        l.flow = e.flow;
        l.viscosity = e.viscosity;
        l.minVerticalFlow = e.minVerticalFlow;
        l.maxMass = e.maxMass;

        gElementPhysicsData[static_cast<size_t>(i)] = e.defaultValues;

        ElementPressureData &pr = gElementPressureData[static_cast<size_t>(i)];
        pr.state = state;
        pr.flow = e.flow;

        ElementPropertyTextureData &tx = gElementPropertyTextureData[static_cast<size_t>(i)];
        tx.state = state;
        tx.colour = e.colour;
        tx.materialProperties = e.materialProperties;
        std::memcpy(tx.gradientColours, e.gradientColours, sizeof tx.gradientColours);
        tx.numberOfGradientColors = e.numberOfGradientColors;

        ElementLightAbsorptionData &a = gElementLightAbsorptionData[static_cast<size_t>(i)];
        a.lightAbsorptionFactor = e.lightAbsorptionFactor;
        a.inverseDefaultMass = StateOf(state) == kSolid ? INFINITY : 1.0f / e.defaultValues.mass;

        ElementRadiationData &r = gElementRadiationData[static_cast<size_t>(i)];
        r.radiationAbsorptionFactor = e.radiationAbsorptionFactor;
        r.radiationPer1000Mass = e.radiationPer1000Mass;

        gElementStateData[static_cast<size_t>(i)] = state;
    }
    for (int i = 0; i < count; i++) {
        const uint8_t *rec = records + static_cast<size_t>(i) * sizeof(Element);
        uint32_t high_ore;
        uint32_t low_ore;
        std::memcpy(&high_ore, rec + 0x4c, 4);
        std::memcpy(&low_ore, rec + 0x44, 4);
        ElementTemperatureData &t = gElementTemperatureData[static_cast<size_t>(i)];
        t.highTempTransitionOreIdx = high_ore == kNoOre ? kInvalidElement : GetElementIndex(high_ore);
        t.lowTempTransitionOreIdx = low_ore == kNoOre ? kInvalidElement : GetElementIndex(low_ore);
    }
    return count;
}
