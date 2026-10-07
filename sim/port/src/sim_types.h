// Generated from sim/layout/layout.txt. Do not edit.
// The original SimDLL structures, laid out exactly like the original.
#pragma once
#include <cstdint>
#include <queue>
#include <vector>

// Enums used in member function signatures (they shape the mangled names).
namespace SpawnFXHashes { enum Type : int32_t {}; }

struct CellSOA;
struct ElementConsumer;
struct ElementEmitter;
struct BuildingHeatExchange;
struct BuildingToBuildingHeatExchange;
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
struct BackwallSOA;
struct ActiveRegion;
struct Region;
struct SimUpdateTemperatureTask;
struct UpdateCellSOATask;
struct UpdateBackwallTask;
struct BuildingHeatExchangeData;
struct BuildingTemperatureInfo;
struct BuildingContact;
struct BuildingToBuildingHeatExchangeData;
struct ConduitTemperatureData;
struct ConduitTemperatureUpdateData;
struct ConduitTemperatureManager;
struct FloodFillInfo;
struct ConsumedMassInfo;
struct ElementConsumerData;
struct ElementEmitterData;
struct EmittedMassInfo;

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

struct ElementConsumer {
    void * vtable;
    std::vector<ElementConsumerData> data;
    std::vector<uint32_t> denseToHandle;
    std::vector<uint32_t> freeHandles;
    std::vector<int32_t> handleToDense;
    std::vector<uint8_t> versions;
    void * componentVtable;
    std::vector<int32_t> visited;
    std::queue<FloodFillInfo> queue;
    std::vector<int32_t> reachable;
    std::vector<ConsumedMassInfo> consumed;
    void Update(float dt, SimData &sim, const Region &region);
};

struct ElementEmitter {
    void * vtable;
    std::vector<ElementEmitterData> data;
    std::vector<uint32_t> denseToHandle;
    std::vector<uint32_t> freeHandles;
    std::vector<int32_t> handleToDense;
    std::vector<uint8_t> versions;
    void * componentVtable;
    std::vector<EmittedMassInfo> emitted;
    std::vector<int32_t> visited;
    std::vector<int32_t> reachable;
    std::queue<FloodFillInfo> queue;
    void Update(float dt, SimData &sim, const Region &region);
};

struct BuildingHeatExchange {
    void * vtable;
    std::vector<BuildingHeatExchangeData> data;
    std::vector<uint32_t> denseToHandle;
    std::vector<uint32_t> freeHandles;
    std::vector<int32_t> handleToDense;
    std::vector<uint8_t> versions;
    void * componentVtable;
    std::vector<BuildingTemperatureInfo> temperatures;
    void Update(float dt, SimData &sim, const Region &region);
};

struct BuildingToBuildingHeatExchange {
    void * vtable;
    std::vector<BuildingToBuildingHeatExchangeData> data;
    std::vector<uint32_t> denseToHandle;
    std::vector<uint32_t> freeHandles;
    std::vector<int32_t> handleToDense;
    std::vector<uint8_t> versions;
    void * componentVtable;
    void Update(float dt, SimData &sim, const Region &region);
};

struct SimData {
    int32_t width;
    int32_t height;
    int32_t numGameCells;
    uint32_t rngState;
    int32_t flowDirection;
    int32_t gasDirection;
    uint8_t savedOptions;
    uint8_t pad_0x19[1];
    uint16_t unk_0x1a;
    uint16_t vacuumElementIdx;
    uint16_t unk_0x1e;
    CellSOA * cells;
    CellSOA * updatedCells;
    BackwallSOA * backwall;
    uint8_t * unk_0x38;
    std::vector<float> radiationLight;
    float * unk_0x58;
    Vector4f * unk_0x60;
    uint8_t * unk_0x68;
    std::vector<ActiveRegion> activeRegions;
    uint8_t * unk_0x88;
    uint16_t displacementDirection;
    uint8_t pad_0x92[6];
    SimEvents * events;
    float unk_0xa0;
    float unk_0xa4;
    uint8_t unk_0xa8;
    uint8_t pad_0xa9[7];
    std::vector<void *> components;
    ElementConsumer elementConsumer;
    ElementEmitter elementEmitter;
    uint8_t pad_0x308[296];
    BuildingHeatExchange buildingHeatExchange;
    BuildingToBuildingHeatExchange buildingToBuildingHeatExchange;
    uint8_t pad_0x558[448];
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
    uint8_t GetStableTicksRemaining(unsigned long cell);
    void UpdateComponents(float dt, const Region &region);
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
    std::vector<MeltedInfo> buildingMelted;
    std::vector<MeltedInfo> buildingOverheat;
    std::vector<MeltedInfo> buildingNoLongerOverheated;
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

#pragma pack(push, 4)
struct BackwallSOA {
    uint16_t noneElementIdx;
    uint8_t pad_0x2[2];
    std::vector<uint16_t> elementIdx;
    std::vector<float> mass;
    std::vector<float> temperature;
};
#pragma pack(pop)

struct ActiveRegion {
    int32_t minX;
    int32_t minY;
    int32_t maxX;
    int32_t maxY;
    float sunlightIntensity;
    float cosmicRadiationIntensity;
};

struct Region {
    int32_t minX;
    int32_t minY;
    int32_t maxX;
    int32_t maxY;
};

struct SimUpdateTemperatureTask {
    void * vtable;
    SimData * sim;
    SimEvents * events;
    Region region;
    void InternalDoTask();
};

struct UpdateCellSOATask {
    void * vtable;
    CellSOA * src;
    CellSOA * dst;
    int32_t numRows;
    int32_t firstRow;
    int32_t srcStride;
    int32_t dstFirstRow;
    int32_t rowLength;
    uint8_t pad_0x2c[4];
    void InternalDoTask();
};

struct UpdateBackwallTask {
    void * vtable;
    BackwallSOA * src;
    BackwallSOA * dst;
    int32_t numRows;
    int32_t firstRow;
    int32_t srcStride;
    int32_t dstFirstRow;
    int32_t rowLength;
    uint8_t pad_0x2c[4];
    void InternalDoTask();
};

struct BuildingHeatExchangeData {
    float temperature;
    float overheatTemperature;
    float operatingKilowatts;
    float heatCapacity;
    float heatCapacityPerCell;
    float conductivity;
    float meltTemperature;
    Region area;
};

struct BuildingTemperatureInfo {
    uint32_t handle;
    float temperature;
};

struct BuildingContact {
    uint32_t handle;
    float unk_0x4;
};

struct BuildingToBuildingHeatExchangeData {
    uint32_t handle;
    uint8_t pad_0x4[4];
    std::vector<BuildingContact> contacts;
};

struct ConduitTemperatureData {
    float temperature;
    float contentsConductivity;
    float contentsHeatCapacity;
    uint32_t structureHandle;
    float conduitHeatCapacity;
    float conduitConductivity;
    bool insulated;
    uint8_t pad_0x19[3];
    float lowTemp;
    float highTemp;
};

#pragma pack(push, 4)
struct ConduitTemperatureUpdateData {
    int32_t numEntries;
    float * temperatures;
    int32_t numFrozenHandles;
    int32_t * frozenHandles;
    int32_t numMeltedHandles;
    int32_t * meltedHandles;
};
#pragma pack(pop)

struct ConduitTemperatureManager {
    void * vtable;
    std::vector<ConduitTemperatureData> data;
    std::vector<uint32_t> denseToHandle;
    std::vector<uint32_t> freeHandles;
    std::vector<int32_t> handleToDense;
    std::vector<uint8_t> versions;
    std::vector<int32_t> melted;
    std::vector<int32_t> frozen;
    std::vector<float> temperatures;
    ConduitTemperatureUpdateData result;
    uint8_t pad_0xec[52];
    uint8_t mutex[40];
    float CalculateEnergyFlow(bool insulated, float contents_temperature, float contents_conductivity, float conduit_temperature, float conduit_conductivity, float scale);
    float ClampEnergyTransfer(float dt, float src_temperature, float src_capacity, float dst_temperature, float dst_capacity, float energy);
    float ModifyTemperature(float temperature, float capacity, float other_temperature, float other_capacity, float energy);
    ConduitTemperatureUpdateData *Update(float dt, const BuildingTemperatureInfo *buildings);
};

struct FloodFillInfo {
    uint16_t x;
    uint16_t y;
    uint16_t distance;
};

struct ConsumedMassInfo {
    int32_t simHandle;
    uint16_t removedElemIdx;
    uint8_t diseaseIdx;
    uint8_t pad_0x7[1];
    float mass;
    float temperature;
    int32_t diseaseCount;
};

struct ElementConsumerData {
    float consumptionRate;
    uint8_t radius;
    uint8_t configuration;
    uint16_t elementIdx;
    uint8_t roundRobin;
    uint8_t pad_0x9[1];
    uint16_t x;
    uint16_t y;
    uint8_t pad_0xe[2];
};

struct ElementEmitterData {
    float timer;
    float emitInterval;
    float emitMass;
    float emitTemperature;
    float maxPressure;
    int32_t diseaseCount;
    uint16_t x;
    uint16_t y;
    uint16_t elementIdx;
    uint8_t range;
    uint8_t startIndex;
    uint8_t blocked;
    uint8_t diseaseIdx;
    uint8_t pad_0x22[2];
    int32_t onBlockedCallback;
    int32_t onUnblockedCallback;
};

struct EmittedMassInfo {
    uint16_t elemIdx;
    uint8_t diseaseIdx;
    uint8_t pad0;
    float mass;
    float temperature;
    int32_t diseaseCount;
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
extern ConduitTemperatureManager * gConduitTemperatureManager;

#include "layout_checks.h"
