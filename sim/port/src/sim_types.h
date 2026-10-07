// Generated from sim/layout/layout.txt. Do not edit.
// The original SimDLL structures, laid out exactly like the original.
#pragma once
#include <cstdint>
#include <vector>

// Enums used in member function signatures (they shape the mangled names).
namespace SpawnFXHashes { enum Type : int32_t {}; }

struct CellSOA;
struct SimData;
struct Vector4f;
struct CellAccessor;
struct SimEvents;
struct SubstanceChangeInfo;
struct SpawnFallingLiquidInfo;
struct SpawnOreInfo;
struct UnstableCellInfo;
struct MeltedInfo;
struct CellMeltedInfo;
struct CallbackInfo;
struct WorldDamageInfo;
struct MassConsumedCallback;
struct MassEmittedCallback;
struct DiseaseConsumptionCallback;
struct ConsumedRadiationCallback;
struct SpawnFXInfo;
struct ComponentStateChangedMessage;
struct BackwallElementChangedInfo;
struct BackwallShouldTransitionInfo;
struct PhysicsData;
struct Element;
struct ElementTemperatureData;
struct ElementPostProcessData;
struct ElementLiquidData;
struct ElementPressureData;
struct ElementPropertyTextureData;
struct ElementLightAbsorptionData;
struct ElementRadiationData;

struct CellSOA {
    std::vector<uint16_t> elementIdx;
    std::vector<float> temperature;
    std::vector<float> mass;
    std::vector<uint8_t> properties;
    std::vector<uint8_t> insulation;
    std::vector<uint8_t> strengthInfo;
    std::vector<uint8_t> diseaseIdx;
    std::vector<int32_t> diseaseCount;
    std::vector<uint8_t> diseaseInfestationTickCount;
    std::vector<float> diseaseAccumulatedError;
    std::vector<float> unk_0xf0;
};

struct SimData {
    int32_t width;
    int32_t height;
    int32_t numGameCells;
    uint32_t rngState;
    uint8_t pad_0x10[4];
    int32_t gasDirection;
    uint8_t unk_0x18;
    uint8_t pad_0x19[3];
    uint16_t vacuumElementIdx;
    uint8_t pad_0x1e[2];
    CellSOA * cells;
    CellSOA * updatedCells;
    uint8_t pad_0x30[40];
    float * unk_0x58;
    Vector4f * unk_0x60;
    uint8_t * unk_0x68;
    uint8_t pad_0x70[24];
    uint8_t * unk_0x88;
    uint16_t displacementDirection;
    uint8_t pad_0x92[6];
    SimEvents * events;
    float unk_0xa0;
    float unk_0xa4;
    uint8_t unk_0xa8;
    uint8_t pad_0xa9[7];
    std::vector<void *> components;
    uint8_t pad_0xc8[1616];
    bool unk_0x718;
    bool headless;
    bool radiationEnabled;
    uint8_t pad_0x71b[1];
    float unk_0x71c;
    float unk_0x720;
    float unk_0x724;
    float unk_0x728;
    float unk_0x72c;
    uint8_t pad_0x730[24];
    void ClearCell(CellAccessor &cell);
};

struct Vector4f {
    float x;
    float y;
    float z;
    float w;
};

struct CellAccessor {
    CellSOA * soa;
    int32_t cell;
    uint8_t pad_0xc[4];
};

struct SimEvents {
    std::vector<SubstanceChangeInfo> substanceChanges;
    std::vector<SpawnFallingLiquidInfo> spawnFallingLiquid;
    std::vector<SpawnOreInfo> spawnOre;
    std::vector<UnstableCellInfo> unstableCells;
    std::vector<MeltedInfo> unk_melted_0x60;
    std::vector<MeltedInfo> unk_melted_0x78;
    std::vector<MeltedInfo> unk_melted_0x90;
    std::vector<int32_t> unk_0xa8;
    std::vector<CellMeltedInfo> cellMelted;
    std::vector<CallbackInfo> callbacks;
    std::vector<WorldDamageInfo> worldDamage;
    std::vector<MassConsumedCallback> massConsumed;
    std::vector<ConsumedRadiationCallback> radiationConsumed;
    std::vector<MassEmittedCallback> massEmitted;
    std::vector<DiseaseConsumptionCallback> diseaseConsumed;
    std::vector<SpawnFXInfo> spawnFX;
    std::vector<ComponentStateChangedMessage> componentStateChanged;
    std::vector<int32_t> unk_0x198;
    std::vector<BackwallElementChangedInfo> backwallElementChanged;
    std::vector<BackwallShouldTransitionInfo> backwallShouldTransition;
    void ChangeSubstance(const SimData &sim, unsigned long cell);
    void SpawnFX(const SimData &sim, unsigned long cell, SpawnFXHashes::Type fx, float rotation);
};

struct SubstanceChangeInfo {
    int32_t cellIdx;
    uint16_t oldElemIdx;
    uint16_t newElemIdx;
};

struct SpawnFallingLiquidInfo {
    int32_t cellIdx;
    uint16_t elemIdx;
    uint8_t diseaseIdx;
    uint8_t pad_0x7[1];
    float mass;
    float temperature;
    int32_t diseaseCount;
};

struct SpawnOreInfo {
    int32_t cellIdx;
    uint16_t elemIdx;
    uint8_t diseaseIdx;
    uint8_t pad_0x7[1];
    float mass;
    float temperature;
    int32_t diseaseCount;
};

struct UnstableCellInfo {
    int32_t cellIdx;
    uint16_t elemIdx;
    uint8_t fallingInfo;
    uint8_t diseaseIdx;
    float mass;
    float temperature;
    int32_t diseaseCount;
};

struct MeltedInfo {
    int32_t handle;
};

struct CellMeltedInfo {
    int32_t gameCell;
};

struct CallbackInfo {
    int32_t callbackIdx;
};

struct WorldDamageInfo {
    int32_t gameCell;
    int32_t damageSourceOffset;
};

struct MassConsumedCallback {
    int32_t callbackIdx;
    uint16_t elemIdx;
    uint8_t diseaseIdx;
    uint8_t pad_0x7[1];
    float mass;
    float temperature;
    int32_t diseaseCount;
};

struct MassEmittedCallback {
    int32_t callbackIdx;
    uint16_t elemIdx;
    uint8_t suceeded;
    uint8_t diseaseIdx;
    float mass;
    float temperature;
    int32_t diseaseCount;
};

struct DiseaseConsumptionCallback {
    int32_t callbackIdx;
    uint8_t diseaseIdx;
    uint8_t pad_0x5[3];
    int32_t diseaseCount;
};

struct ConsumedRadiationCallback {
    int32_t callbackIdx;
    int32_t gameCell;
    float radiation;
};

struct SpawnFXInfo {
    int32_t cellIdx;
    int32_t fxHash;
    float rotation;
};

struct ComponentStateChangedMessage {
    int32_t callbackIdx;
    int32_t simHandle;
};

struct BackwallElementChangedInfo {
    int32_t gameCell;
};

struct BackwallShouldTransitionInfo {
    int32_t gameCell;
};

struct PhysicsData {
    float temperature;
    float mass;
    float pressure;
};

struct Element {
    int32_t id;
    uint16_t lowTempTransitionIdx;
    uint16_t highTempTransitionIdx;
    uint16_t elementsTableIdx;
    uint8_t state;
    uint8_t numberOfGradientColors;
    float specificHeatCapacity;
    float thermalConductivity;
    float molarMass;
    float solidSurfaceAreaMultiplier;
    float liquidSurfaceAreaMultiplier;
    float gasSurfaceAreaMultiplier;
    float flow;
    float viscosity;
    float minHorizontalFlow;
    float minVerticalFlow;
    float maxMass;
    float lowTemp;
    float highTemp;
    float strength;
    int32_t lowTempTransitionOreID;
    float lowTempTransitionOreMassConversion;
    int32_t highTempTransitionOreID;
    float highTempTransitionOreMassConversion;
    uint16_t sublimateIndex;
    uint16_t convertIndex;
    uint32_t materialProperties;
    uint32_t colour;
    uint32_t gradientColours[6];
    int32_t sublimateFX;
    float sublimateRate;
    float sublimateEfficiency;
    float sublimateProbability;
    float offGasProbability;
    float lightAbsorptionFactor;
    float radiationAbsorptionFactor;
    float radiationPer1000Mass;
    PhysicsData defaultValues;
};

struct ElementTemperatureData {
    uint8_t state;
    uint8_t pad_0x1[1];
    uint16_t lowTempTransitionIdx;
    uint16_t highTempTransitionIdx;
    uint16_t lowTempTransitionOreIdx;
    uint16_t highTempTransitionOreIdx;
    uint8_t pad_0xa[2];
    float specificHeatCapacity;
    float thermalConductivity;
    float defaultMass;
    float minMassForTemperature;
    float gasSurfaceAreaMultiplier;
    float liquidSurfaceAreaMultiplier;
    float solidSurfaceAreaMultiplier;
    float lowTemp;
    float highTemp;
    float lowTempTransitionOreMassConversion;
    float highTempTransitionOreMassConversion;
};

struct ElementPostProcessData {
    uint16_t sublimateIndex;
    uint16_t convertIndex;
    uint8_t state;
    uint8_t pad_0x5[3];
    float sublimateRate;
    float sublimateEfficiency;
    float sublimateProbability;
    float offGasProbability;
    float molarMass;
    float strength;
    float maxMass;
    float minHorizontalFlow;
    int32_t sublimateFX;
};

struct ElementLiquidData {
    uint8_t state;
    uint8_t pad_0x1[3];
    float flow;
    float viscosity;
    float minHorizontalFlow;
    float minVerticalFlow;
    float maxMass;
};

struct ElementPressureData {
    uint8_t state;
    uint8_t pad_0x1[3];
    float flow;
};

struct ElementPropertyTextureData {
    uint32_t colour;
    uint32_t gradientColours[6];
    uint32_t materialProperties;
    uint8_t state;
    uint8_t numberOfGradientColors;
    uint8_t pad_0x22[2];
};

struct ElementLightAbsorptionData {
    float lightAbsorptionFactor;
    float inverseDefaultMass;
};

struct ElementRadiationData {
    float radiationAbsorptionFactor;
    float radiationPer1000Mass;
};

extern std::vector<Element> gElements;
extern std::vector<ElementTemperatureData> gElementTemperatureData;
extern std::vector<ElementPostProcessData> gElementPostProcessData;
extern std::vector<ElementLiquidData> gElementLiquidData;
extern std::vector<PhysicsData> gElementPhysicsData;
extern std::vector<ElementPressureData> gElementPressureData;
extern std::vector<ElementPropertyTextureData> gElementPropertyTextureData;
extern std::vector<ElementLightAbsorptionData> gElementLightAbsorptionData;
extern std::vector<ElementRadiationData> gElementRadiationData;
extern std::vector<uint8_t> gElementStateData;
extern SimData * gSimData;

#include "layout_checks.h"
