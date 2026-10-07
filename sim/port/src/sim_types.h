// Generated from sim/layout/layout.txt. Do not edit.
// The original SimDLL structures, laid out exactly like the original.
#pragma once
#include <cstdint>
#include <queue>
#include <vector>

// Enums used in member function signatures (they shape the mangled names).
namespace SpawnFXHashes { enum Type : int32_t {}; }

// The original's small vector templates (their names appear in mangled signatures).
template <typename T>
struct Vector2 {
    T x, y;
};
template <typename T>
struct Vector4 {
    T x, y, z, w;
};

// Component handle: low 24 bits index, top 8 bits version; 0xf00000 set means invalid.
// Its copy constructor is user-provided, as in the original: that makes it non-trivial for
// the ABI, so it is returned through a hidden pointer and passed by invisible reference.
struct Handle {
    uint32_t value;
    Handle() = default;
    explicit Handle(uint32_t v) : value(v) {}
    Handle(const Handle &other) : value(other.value) {}
    Handle &operator=(const Handle &other) { value = other.value; return *this; }
};

struct CellSOA;
struct ElementConsumer;
struct ElementEmitter;
struct RadiationEmitter;
struct ElementChunk;
struct BuildingHeatExchange;
struct BuildingToBuildingHeatExchange;
struct DiseaseEmitter;
struct DiseaseConsumer;
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
struct SolidInfo;
struct LiquidChangeInfo;
struct SolidSubstanceChangeInfo;
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
struct WorldRect;
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
struct DiseaseEmitterData;
struct DiseaseEmittedInfo;
struct DiseaseConsumerData;
struct ElementChunkData;
struct ElementChunkInfo;
struct RadiationEmitterData;
struct GameData;
struct DigPoint;
struct CellModification;
struct CellEnergyModification;
struct SetCellFloatValue;
struct CellPropertiesChange;
struct ConsumeDisease;
struct CellDiseaseModification;
struct CellRadiationModification;
struct RadiationParamsModification;
struct CellWorldZoneModification;
struct MassConsumption;
struct MassEmission;
struct AddBuildingHeatExchangeMessage;
struct ModifyBuildingHeatExchangeMessage;
struct RemoveBuildingHeatExchangeMessage;
struct ModifyBuildingEnergyMessage;
struct RegisterBuildingToBuildingHeatExchangeMessage;
struct RemoveBuildingInContactFromBuildingToBuildingHeatExchangeMessage;
struct RemoveBuildingToBuildingHeatExchangeMessage;
struct AddInContactBuildingToBuildingToBuildingHeatExchangeMessage;
struct AddElementChunkMessage;
struct ModifyElementChunkMessage;
struct RemoveElementChunkMessage;
struct MoveElementChunkMessage;
struct ModifyElementChunkEnergyMessage;
struct ModifyElementChunkAdjusterMessage;
struct ModifyBackwallDataMessage;
struct AddElementConsumerMessage;
struct ModifyElementConsumerMessage;
struct RemoveElementConsumerMessage;
struct AddElementEmitterMessage;
struct ModifyElementEmitterMessage;
struct RemoveElementEmitterMessage;
struct AddDiseaseEmitterMessage;
struct ModifyDiseaseEmitterMessage;
struct RemoveDiseaseEmitterMessage;
struct AddDiseaseConsumerMessage;
struct ModifyDiseaseConsumerMessage;
struct RemoveDiseaseConsumerMessage;
struct AddRadiationEmitterMessage;
struct ModifyRadiationEmitterMessage;
struct RemoveRadiationEmitterMessage;
struct DebugProperties;
struct NewGameFrame;
struct SimFrameInfo;
struct SimFrameManager;
struct SimBase;

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
    Handle Register(const SimData &sim, const AddElementConsumerMessage &msg);
    void Modify(const SimData &sim, const ModifyElementConsumerMessage &msg);
    void Unregister(Handle handle);
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
    void UpdateDataListOnly(SimData &sim);
    Handle Register(const SimData &sim, const AddElementEmitterMessage &msg);
    void Modify(const SimData &sim, const ModifyElementEmitterMessage &msg);
    void Unregister(Handle handle);
};

struct RadiationEmitter {
    void * vtable;
    std::vector<RadiationEmitterData> data;
    std::vector<uint32_t> denseToHandle;
    std::vector<uint32_t> freeHandles;
    std::vector<int32_t> handleToDense;
    std::vector<uint8_t> versions;
    void * componentVtable;
    void Update(float dt, SimData &sim, const Region &region);
    Handle Register(const SimData &sim, const AddRadiationEmitterMessage &msg);
    void Modify(const SimData &sim, const ModifyRadiationEmitterMessage &msg);
    void Unregister(Handle handle);
};

struct ElementChunk {
    void * vtable;
    std::vector<ElementChunkData> data;
    std::vector<uint32_t> denseToHandle;
    std::vector<uint32_t> freeHandles;
    std::vector<int32_t> handleToDense;
    std::vector<uint8_t> versions;
    void * componentVtable;
    std::vector<ElementChunkInfo> info;
    void Update(float dt, SimData &sim, const Region &region);
    void UpdateDataListOnly(SimData &sim);
    Handle Register(const SimData &sim, const AddElementChunkMessage &msg);
    void Modify(const SimData &sim, const ModifyElementChunkMessage &msg);
    void ModifyEnergy(const SimData &sim, const ModifyElementChunkEnergyMessage &msg);
    void ModifyAdjuster(const SimData &sim, const ModifyElementChunkAdjusterMessage &msg);
    void Unregister(Handle handle);
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
    void UpdateDataListOnly(SimData &sim);
    Handle Register(const SimData &sim, const AddBuildingHeatExchangeMessage &msg);
    void Modify(const SimData &sim, const ModifyBuildingHeatExchangeMessage &msg);
    void Unregister(Handle handle);
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
    Handle Register(const SimData &sim, const RegisterBuildingToBuildingHeatExchangeMessage &msg);
    void Add(const SimData &sim, const AddInContactBuildingToBuildingToBuildingHeatExchangeMessage &msg);
    void Remove(const SimData &sim, const RemoveBuildingInContactFromBuildingToBuildingHeatExchangeMessage &msg);
    void Unregister(Handle handle);
};

struct DiseaseEmitter {
    void * vtable;
    std::vector<DiseaseEmitterData> data;
    std::vector<uint32_t> denseToHandle;
    std::vector<uint32_t> freeHandles;
    std::vector<int32_t> handleToDense;
    std::vector<uint8_t> versions;
    void * componentVtable;
    std::vector<DiseaseEmittedInfo> emitted;
    std::queue<FloodFillInfo> queue;
    std::vector<int32_t> visited;
    std::vector<int32_t> reachable;
    void Update(float dt, SimData &sim, const Region &region);
    void UpdateDataListOnly(SimData &sim);
    Handle Register(const SimData &sim, const AddDiseaseEmitterMessage &msg);
    void Modify(const SimData &sim, const ModifyDiseaseEmitterMessage &msg);
    void Unregister(Handle handle);
};

struct DiseaseConsumer {
    void * vtable;
    std::vector<DiseaseConsumerData> data;
    std::vector<uint32_t> denseToHandle;
    std::vector<uint32_t> freeHandles;
    std::vector<int32_t> handleToDense;
    std::vector<uint8_t> versions;
    void * componentVtable;
    std::vector<DiseaseEmittedInfo> consumed;
    Handle Register(const SimData &sim, const AddDiseaseConsumerMessage &msg);
    void Modify(const SimData &sim, const ModifyDiseaseConsumerMessage &msg);
    void Unregister(Handle handle);
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
    RadiationEmitter radiationEmitter;
    ElementChunk elementChunk;
    BuildingHeatExchange buildingHeatExchange;
    BuildingToBuildingHeatExchange buildingToBuildingHeatExchange;
    DiseaseEmitter diseaseEmitter;
    DiseaseConsumer diseaseConsumer;
    bool unk_0x718;
    bool headless;
    bool radiationEnabled;
    uint8_t pad_0x71b[1];
    float unk_0x71c;
    float unk_0x720;
    float unk_0x724;
    float unk_0x728;
    float unk_0x72c;
    std::vector<WorldRect> worldRects;
    void ClearCell(CellAccessor &cell);
    uint8_t GetStableTicksRemaining(unsigned long cell);
    void UpdateComponents(float dt, const Region &region);
    void UpdateComponentsDataListOnly();
    void ClearBackwall(int cell);
    SimData(int width, int height, unsigned int seed, bool radiation_enabled, bool headless);
    ~SimData();
    void InitializeBoundary();
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
    std::vector<MeltedInfo> elementChunkMelted;
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
    std::vector<SpawnOreInfo> digInfo;
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

struct SolidInfo {
    int32_t cellIdx;
    int32_t isSolid;
};

struct LiquidChangeInfo {
    int32_t cellIdx;
};

struct SolidSubstanceChangeInfo {
    int32_t cellIdx;
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

struct WorldRect {
    int32_t x;
    int32_t y;
    int32_t width;
    int32_t height;
};

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
    int32_t cellsInContact;
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
    int32_t pendingIndex;
    std::vector<uint32_t> pendingRemovals0;
    std::vector<uint32_t> pendingRemovals1;
    uint8_t mutex[40];
    float CalculateEnergyFlow(bool insulated, float contents_temperature, float contents_conductivity, float conduit_temperature, float conduit_conductivity, float scale);
    float ClampEnergyTransfer(float dt, float src_temperature, float src_capacity, float dst_temperature, float dst_capacity, float energy);
    float ModifyTemperature(float temperature, float capacity, float other_temperature, float other_capacity, float energy);
    ConduitTemperatureUpdateData *Update(float dt, const BuildingTemperatureInfo *buildings);
    uint32_t Add(float temperature, float mass, int element_hash, int structure_handle, float conduit_heat_capacity, float conduit_conductivity, bool insulated);
    void Set(int handle, float temperature, float mass, int element_hash);
    void Remove(int handle);
    void Clear();
    void ReleaseQueuedHandles();
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

struct DiseaseEmitterData {
    int32_t cell;
    uint8_t diseaseIdx;
    uint8_t range;
    uint8_t pad_0x6[2];
    int32_t count;
    float emitInterval;
    float timer;
};

struct DiseaseEmittedInfo {
    uint8_t diseaseIdx;
    uint8_t pad_0x1[3];
    int32_t count;
};

struct DiseaseConsumerData {
    uint8_t unk_0x0[12];
};

struct ElementChunkData {
    int32_t cell;
    float temperature;
    float heatCapacity;
    float thermalConductivity;
    float areaOverThickness;
    float groundTransferScale;
    float highTemp;
    float lowTemp;
    float adjusterTemperature;
    float adjusterHeatCapacity;
    float adjusterRate;
};

struct ElementChunkInfo {
    float temperature;
    float deltaKJ;
};

struct RadiationEmitterData {
    int32_t cell;
    uint16_t radiusX;
    uint16_t radiusY;
    float intensity;
    float pulseInterval;
    float pulseDuration;
    float direction;
    float angle;
    float timer;
    float pulseTimer;
    int32_t type;
    int32_t step;
};

struct GameData {
    int32_t width;
    int32_t height;
    int32_t numFramesProcessed;
    uint8_t pad_0xc[4];
    std::vector<SubstanceChangeInfo> substanceChanges;
    std::vector<CallbackInfo> callbacks;
    std::vector<SpawnFallingLiquidInfo> spawnFallingLiquid;
    std::vector<SpawnOreInfo> spawnOre;
    std::vector<SpawnOreInfo> digInfo;
    std::vector<UnstableCellInfo> unstableCells;
    std::vector<WorldDamageInfo> worldDamage;
    std::vector<BuildingTemperatureInfo> buildingTemperatures;
    std::vector<MassConsumedCallback> massConsumed;
    std::vector<MassEmittedCallback> massEmitted;
    std::vector<DiseaseConsumptionCallback> diseaseConsumed;
    std::vector<ConsumedRadiationCallback> radiationConsumed;
    std::vector<SpawnFXInfo> spawnFX;
    std::vector<SolidInfo> solidInfo;
    std::vector<LiquidChangeInfo> liquidChanges;
    std::vector<SolidSubstanceChangeInfo> solidSubstanceChanges;
    std::vector<ConsumedMassInfo> removedMass;
    std::vector<EmittedMassInfo> emittedMass;
    std::vector<ElementChunkInfo> elementChunkInfos;
    std::vector<DiseaseEmittedInfo> diseaseEmitted;
    std::vector<DiseaseEmittedInfo> diseaseConsumedInfos;
    std::vector<MeltedInfo> elementChunkMelted;
    std::vector<MeltedInfo> buildingMelted;
    std::vector<MeltedInfo> buildingOverheat;
    std::vector<MeltedInfo> buildingNoLongerOverheated;
    std::vector<ComponentStateChangedMessage> componentStateChanged;
    std::vector<CellMeltedInfo> cellMelted;
    std::vector<BackwallElementChangedInfo> backwallElementChanged;
    std::vector<BackwallShouldTransitionInfo> backwallShouldTransition;
    CellSOA * cells;
    BackwallSOA * backwall;
    Vector4f * flow;
    float * accumulatedFlow;
    uint8_t * visibleGrid;
    void * propertyTextureFlow;
    void * propertyTextureLiquid;
    void * propertyTextureLiquidData;
    void * propertyTextureMaterialData;
    uint8_t * propertyTextureExposedToSunlight;
    GameData(int width, int height);
    ~GameData();
};

struct DigPoint {
    int32_t cellIdx;
    int32_t callbackIdx;
    bool skipEvent;
    bool backwall;
    uint8_t pad_0xa[2];
};

struct CellModification {
    int32_t cellIdx;
    int32_t callbackIdx;
    float temperature;
    float mass;
    int32_t diseaseCount;
    uint16_t elementIdx;
    uint8_t replaceType;
    uint8_t diseaseIdx;
    uint8_t addSubType;
    uint8_t pad_0x19[3];
};

struct CellEnergyModification {
    int32_t cellIdx;
    float kilojoules;
    float maxTemperature;
    int32_t id;
};

struct SetCellFloatValue {
    int32_t cellIdx;
    float value;
};

struct CellPropertiesChange {
    int32_t cellIdx;
    int32_t callbackIdx;
    uint8_t properties;
    uint8_t set;
    uint8_t pad0;
    uint8_t pad1;
};

struct ConsumeDisease {
    int32_t gameCell;
    int32_t callbackIdx;
    float percentToConsume;
    int32_t maxToConsume;
};

struct CellDiseaseModification {
    int32_t cellIdx;
    uint8_t diseaseIdx;
    uint8_t pad0;
    uint8_t pad1;
    uint8_t pad2;
    int32_t diseaseCount;
};

struct CellRadiationModification {
    int32_t cellIdx;
    float radiationDelta;
    int32_t callbackIdx;
};

struct RadiationParamsModification {
    int32_t RadiationParamsType;
    float value;
};

struct CellWorldZoneModification {
    int32_t cell;
    uint8_t zoneID;
    uint8_t pad_0x5[3];
};

struct MassConsumption {
    int32_t cellIdx;
    int32_t callbackIdx;
    float mass;
    uint16_t elementIdx;
    uint8_t radius;
    uint8_t height;
};

struct MassEmission {
    int32_t cellIdx;
    int32_t callbackIdx;
    float mass;
    float temperature;
    int32_t diseaseCount;
    uint16_t elementIdx;
    uint8_t diseaseIdx;
    uint8_t pad_0x17[1];
};

struct AddBuildingHeatExchangeMessage {
    int32_t callbackIdx;
    uint16_t elemIdx;
    uint8_t pad0;
    uint8_t pad1;
    float mass;
    float temperature;
    float thermalConductivity;
    float overheatTemperature;
    float operatingKilowatts;
    int32_t minX;
    int32_t minY;
    int32_t maxX;
    int32_t maxY;
};

struct ModifyBuildingHeatExchangeMessage {
    int32_t callbackIdx;
    uint16_t elemIdx;
    uint8_t pad0;
    uint8_t pad1;
    float mass;
    float temperature;
    float thermalConductivity;
    float overheatTemperature;
    float operatingKilowatts;
    int32_t minX;
    int32_t minY;
    int32_t maxX;
    int32_t maxY;
};

struct RemoveBuildingHeatExchangeMessage {
    int32_t handle;
    int32_t callbackIdx;
};

struct ModifyBuildingEnergyMessage {
    int32_t handle;
    float deltaKJ;
    float minTemperature;
    float maxTemperature;
};

struct RegisterBuildingToBuildingHeatExchangeMessage {
    int32_t callbackIdx;
    int32_t structureTemperatureHandler;
};

struct RemoveBuildingInContactFromBuildingToBuildingHeatExchangeMessage {
    int32_t selfHandler;
    int32_t buildingNoLongerInContactHandler;
};

struct RemoveBuildingToBuildingHeatExchangeMessage {
    int32_t callbackIdx;
    int32_t selfHandler;
};

struct AddInContactBuildingToBuildingToBuildingHeatExchangeMessage {
    int32_t selfHandler;
    int32_t buildingInContactHandle;
    int32_t cellsInContact;
};

struct AddElementChunkMessage {
    int32_t gameCell;
    int32_t callbackIdx;
    float mass;
    float temperature;
    float surfaceArea;
    float thickness;
    float groundTransferScale;
    uint16_t elementIdx;
    uint8_t pad0;
    uint8_t pad1;
};

struct ModifyElementChunkMessage {
    int32_t handle;
    float temperature;
    float heatCapacity;
};

struct RemoveElementChunkMessage {
    int32_t handle;
    int32_t callbackIdx;
};

struct MoveElementChunkMessage {
    int32_t handle;
    int32_t gameCell;
};

struct ModifyElementChunkEnergyMessage {
    int32_t handle;
    float deltaKJ;
};

struct ModifyElementChunkAdjusterMessage {
    int32_t handle;
    float temperature;
    float heatCapacity;
    float thermalConductivity;
};

struct ModifyBackwallDataMessage {
    int32_t gameCell;
    uint16_t elementIdx;
    uint16_t pad;
    float mass;
    float temperature;
};

struct AddElementConsumerMessage {
    int32_t cellIdx;
    int32_t callbackIdx;
    uint8_t radius;
    uint8_t configuration;
    uint16_t elementIdx;
};

struct ModifyElementConsumerMessage {
    int32_t handle;
    int32_t cell;
    float consumptionRate;
};

struct RemoveElementConsumerMessage {
    int32_t handle;
    int32_t callbackIdx;
};

struct AddElementEmitterMessage {
    float maxPressure;
    int32_t callbackIdx;
    int32_t onBlockedCB;
    int32_t onUnblockedCB;
};

struct ModifyElementEmitterMessage {
    int32_t handle;
    int32_t cellIdx;
    float emitInterval;
    float emitMass;
    float emitTemperature;
    float maxPressure;
    int32_t diseaseCount;
    uint16_t elementIdx;
    uint8_t maxDepth;
    uint8_t diseaseIdx;
};

struct RemoveElementEmitterMessage {
    int32_t handle;
    int32_t callbackIdx;
};

struct AddDiseaseEmitterMessage {
    int32_t callbackIdx;
};

struct ModifyDiseaseEmitterMessage {
    int32_t handle;
    int32_t gameCell;
    uint8_t diseaseIdx;
    uint8_t maxDepth;
    uint8_t pad_0xa[2];
    float emitInterval;
    int32_t emitCount;
};

struct RemoveDiseaseEmitterMessage {
    int32_t handle;
    int32_t callbackIdx;
};

struct AddDiseaseConsumerMessage {
    int32_t unk_0x0;
    int32_t unk_0x4;
    int32_t unk_0x8;
};

struct ModifyDiseaseConsumerMessage {
    int32_t unk_0x0;
    int32_t unk_0x4;
    int32_t unk_0x8;
    int32_t unk_0xc;
    int32_t unk_0x10;
};

struct RemoveDiseaseConsumerMessage {
    int32_t handle;
    int32_t callbackIdx;
};

struct AddRadiationEmitterMessage {
    int32_t callbackIdx;
    int32_t cell;
    int16_t emitRadiusX;
    int16_t emitRadiusY;
    float emitRads;
    float emitRate;
    float emitSpeed;
    float emitDirection;
    float emitAngle;
    int32_t emitType;
};

struct ModifyRadiationEmitterMessage {
    int32_t handle;
    int32_t cell;
    int32_t callbackIdx;
    int16_t emitRadiusX;
    int16_t emitRadiusY;
    float emitRads;
    float emitRate;
    float emitSpeed;
    float emitDirection;
    float emitAngle;
    int32_t emitType;
};

struct RemoveRadiationEmitterMessage {
    int32_t handle;
    int32_t callbackIdx;
};

struct DebugProperties {
    float buildingTemperatureScale;
    float buildingToBuildingTemperatureScale;
    uint8_t isDebugEditing;
    uint8_t pad0;
    uint8_t pad1;
    uint8_t pad2;
};

struct NewGameFrame {
    float elapsedSeconds;
    int32_t minX;
    int32_t minY;
    int32_t maxX;
    int32_t maxY;
    float currentSunlightIntensity;
    float currentCosmicRadiationIntensity;
};

struct SimFrameInfo {
    float dt;
    uint8_t pad_0x4[4];
    std::vector<DigPoint> digs;
    std::vector<CellModification> cellModifications;
    std::vector<CellEnergyModification> cellEnergyModifications;
    std::vector<uint8_t> unk_0x50;
    std::vector<SetCellFloatValue> insulationChanges;
    std::vector<SetCellFloatValue> strengthChanges;
    std::vector<CellPropertiesChange> propertyChanges;
    std::vector<uint8_t> unk_0xb0;
    std::vector<ConsumeDisease> diseaseConsumptions;
    std::vector<CellDiseaseModification> diseaseModifications;
    std::vector<CellRadiationModification> radiationModifications;
    std::vector<RadiationParamsModification> radiationParams;
    std::vector<CellWorldZoneModification> worldZoneModifications;
    std::vector<MassConsumption> massConsumptions;
    std::vector<MassEmission> massEmissions;
    std::vector<AddBuildingHeatExchangeMessage> addBuildingHeatExchange;
    std::vector<ModifyBuildingHeatExchangeMessage> modifyBuildingHeatExchange;
    std::vector<RemoveBuildingHeatExchangeMessage> removeBuildingHeatExchange;
    std::vector<ModifyBuildingEnergyMessage> modifyBuildingEnergy;
    std::vector<RegisterBuildingToBuildingHeatExchangeMessage> registerBuildingToBuilding;
    std::vector<RemoveBuildingInContactFromBuildingToBuildingHeatExchangeMessage> removeBuildingInContact;
    std::vector<RemoveBuildingToBuildingHeatExchangeMessage> removeBuildingToBuilding;
    std::vector<AddInContactBuildingToBuildingToBuildingHeatExchangeMessage> addBuildingInContact;
    std::vector<AddElementChunkMessage> addElementChunk;
    std::vector<ModifyElementChunkMessage> modifyElementChunk;
    std::vector<RemoveElementChunkMessage> removeElementChunk;
    std::vector<MoveElementChunkMessage> moveElementChunk;
    std::vector<ModifyElementChunkEnergyMessage> modifyElementChunkEnergy;
    std::vector<ModifyElementChunkAdjusterMessage> modifyElementChunkAdjuster;
    std::vector<ModifyBackwallDataMessage> modifyBackwall;
    std::vector<AddElementConsumerMessage> addElementConsumer;
    std::vector<ModifyElementConsumerMessage> modifyElementConsumer;
    std::vector<RemoveElementConsumerMessage> removeElementConsumer;
    std::vector<AddElementEmitterMessage> addElementEmitter;
    std::vector<ModifyElementEmitterMessage> modifyElementEmitter;
    std::vector<RemoveElementEmitterMessage> removeElementEmitter;
    std::vector<AddDiseaseEmitterMessage> addDiseaseEmitter;
    std::vector<ModifyDiseaseEmitterMessage> modifyDiseaseEmitter;
    std::vector<RemoveDiseaseEmitterMessage> removeDiseaseEmitter;
    std::vector<AddDiseaseConsumerMessage> addDiseaseConsumer;
    std::vector<ModifyDiseaseConsumerMessage> modifyDiseaseConsumer;
    std::vector<RemoveDiseaseConsumerMessage> removeDiseaseConsumer;
    std::vector<AddRadiationEmitterMessage> addRadiationEmitter;
    std::vector<ModifyRadiationEmitterMessage> modifyRadiationEmitter;
    std::vector<RemoveRadiationEmitterMessage> removeRadiationEmitter;
    DebugProperties debugProperties;
    uint8_t pad_0x44c[4];
};

struct SimFrameManager {
    uint8_t mutex[40];
    SimFrameInfo * current;
    std::vector<void *> freeFrames;
    std::vector<void *> queuedFrames;
    std::vector<void *> processingFrames;
    std::vector<void *> processedFrames;
    uint8_t pad_0x90[32];
    std::vector<ActiveRegion> activeRegions;
    int32_t nextFrame;
    uint8_t pad_0xcc[4];
    bool HandleMessage(int msg_id, int length, void *data, void *&out);
    int BeginFrameProcessing();
    float ProcessNextFrame(SimData &sim);
    void EndFrameProcessing();
    void NewFrame();
    static void ProcessFrame(SimFrameInfo &frame, SimData &sim);
};

struct SimBase {
    void * vtable;
    SimFrameManager frames;
    void * taskQueue;
    std::vector<void *> updateTasks;
    std::vector<void *> taskEvents;
    std::vector<void *> flowTasks;
    std::vector<void *> cellTasks;
    std::vector<void *> backwallTasks;
    float remainder;
    uint8_t pad_0x15c[4];
    void UpdatedCellsToCells(SimData &sim, Vector2<int> min, Vector2<int> max);
    void CopyUpdatedCellsToCells(SimData &sim);
    void ConsolidateEvents(SimData &sim);
    void UpdateData(SimData &sim);
    void Step(SimData &sim, GameData &game, const GameData &previous);
    void CopySimDataToGame(SimData &sim, GameData &game, const GameData &previous, int frames);
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
