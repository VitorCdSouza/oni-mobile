// Generated from sim/layout/layout.txt. Do not edit.
typedef unsigned char uint8_t;
typedef unsigned short uint16_t;
typedef short int16_t;
typedef unsigned int uint32_t;
typedef int int32_t;
typedef unsigned long long uint64_t;
typedef long long int64_t;

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

struct Vec_ActiveRegion { struct ActiveRegion *begin; struct ActiveRegion *end; struct ActiveRegion *cap; };
struct Vec_BackwallElementChangedInfo { struct BackwallElementChangedInfo *begin; struct BackwallElementChangedInfo *end; struct BackwallElementChangedInfo *cap; };
struct Vec_BackwallShouldTransitionInfo { struct BackwallShouldTransitionInfo *begin; struct BackwallShouldTransitionInfo *end; struct BackwallShouldTransitionInfo *cap; };
struct Vec_BuildingContact { struct BuildingContact *begin; struct BuildingContact *end; struct BuildingContact *cap; };
struct Vec_BuildingHeatExchangeData { struct BuildingHeatExchangeData *begin; struct BuildingHeatExchangeData *end; struct BuildingHeatExchangeData *cap; };
struct Vec_BuildingTemperatureInfo { struct BuildingTemperatureInfo *begin; struct BuildingTemperatureInfo *end; struct BuildingTemperatureInfo *cap; };
struct Vec_BuildingToBuildingHeatExchangeData { struct BuildingToBuildingHeatExchangeData *begin; struct BuildingToBuildingHeatExchangeData *end; struct BuildingToBuildingHeatExchangeData *cap; };
struct Vec_CallbackInfo { struct CallbackInfo *begin; struct CallbackInfo *end; struct CallbackInfo *cap; };
struct Vec_CellMeltedInfo { struct CellMeltedInfo *begin; struct CellMeltedInfo *end; struct CellMeltedInfo *cap; };
struct Vec_ComponentStateChangedMessage { struct ComponentStateChangedMessage *begin; struct ComponentStateChangedMessage *end; struct ComponentStateChangedMessage *cap; };
struct Vec_ConduitTemperatureData { struct ConduitTemperatureData *begin; struct ConduitTemperatureData *end; struct ConduitTemperatureData *cap; };
struct Vec_ConsumedMassInfo { struct ConsumedMassInfo *begin; struct ConsumedMassInfo *end; struct ConsumedMassInfo *cap; };
struct Vec_ConsumedRadiationCallback { struct ConsumedRadiationCallback *begin; struct ConsumedRadiationCallback *end; struct ConsumedRadiationCallback *cap; };
struct Vec_DiseaseConsumptionCallback { struct DiseaseConsumptionCallback *begin; struct DiseaseConsumptionCallback *end; struct DiseaseConsumptionCallback *cap; };
struct Vec_Element { struct Element *begin; struct Element *end; struct Element *cap; };
struct Vec_ElementConsumerData { struct ElementConsumerData *begin; struct ElementConsumerData *end; struct ElementConsumerData *cap; };
struct Vec_ElementEmitterData { struct ElementEmitterData *begin; struct ElementEmitterData *end; struct ElementEmitterData *cap; };
struct Vec_ElementLightAbsorptionData { struct ElementLightAbsorptionData *begin; struct ElementLightAbsorptionData *end; struct ElementLightAbsorptionData *cap; };
struct Vec_ElementLiquidData { struct ElementLiquidData *begin; struct ElementLiquidData *end; struct ElementLiquidData *cap; };
struct Vec_ElementPostProcessData { struct ElementPostProcessData *begin; struct ElementPostProcessData *end; struct ElementPostProcessData *cap; };
struct Vec_ElementPressureData { struct ElementPressureData *begin; struct ElementPressureData *end; struct ElementPressureData *cap; };
struct Vec_ElementPropertyTextureData { struct ElementPropertyTextureData *begin; struct ElementPropertyTextureData *end; struct ElementPropertyTextureData *cap; };
struct Vec_ElementRadiationData { struct ElementRadiationData *begin; struct ElementRadiationData *end; struct ElementRadiationData *cap; };
struct Vec_ElementTemperatureData { struct ElementTemperatureData *begin; struct ElementTemperatureData *end; struct ElementTemperatureData *cap; };
struct Vec_EmittedMassInfo { struct EmittedMassInfo *begin; struct EmittedMassInfo *end; struct EmittedMassInfo *cap; };
struct Vec_MassConsumedCallback { struct MassConsumedCallback *begin; struct MassConsumedCallback *end; struct MassConsumedCallback *cap; };
struct Vec_MassEmittedCallback { struct MassEmittedCallback *begin; struct MassEmittedCallback *end; struct MassEmittedCallback *cap; };
struct Vec_MeltedInfo { struct MeltedInfo *begin; struct MeltedInfo *end; struct MeltedInfo *cap; };
struct Vec_PhysicsData { struct PhysicsData *begin; struct PhysicsData *end; struct PhysicsData *cap; };
struct Vec_SpawnFXInfo { struct SpawnFXInfo *begin; struct SpawnFXInfo *end; struct SpawnFXInfo *cap; };
struct Vec_SpawnFallingLiquidInfo { struct SpawnFallingLiquidInfo *begin; struct SpawnFallingLiquidInfo *end; struct SpawnFallingLiquidInfo *cap; };
struct Vec_SpawnOreInfo { struct SpawnOreInfo *begin; struct SpawnOreInfo *end; struct SpawnOreInfo *cap; };
struct Vec_SubstanceChangeInfo { struct SubstanceChangeInfo *begin; struct SubstanceChangeInfo *end; struct SubstanceChangeInfo *cap; };
struct Vec_UnstableCellInfo { struct UnstableCellInfo *begin; struct UnstableCellInfo *end; struct UnstableCellInfo *cap; };
struct Vec_WorldDamageInfo { struct WorldDamageInfo *begin; struct WorldDamageInfo *end; struct WorldDamageInfo *cap; };
struct Vec_f32 { float *begin; float *end; float *cap; };
struct Vec_i32 { int32_t *begin; int32_t *end; int32_t *cap; };
struct Vec_ptr { void * *begin; void * *end; void * *cap; };
struct Vec_u16 { uint16_t *begin; uint16_t *end; uint16_t *cap; };
struct Vec_u32 { uint32_t *begin; uint32_t *end; uint32_t *cap; };
struct Vec_u8 { uint8_t *begin; uint8_t *end; uint8_t *cap; };

struct CellSOA {
    struct Vec_u16 elementIdx;
    struct Vec_f32 temperature;
    struct Vec_f32 mass;
    struct Vec_u8 properties;
    struct Vec_u8 insulation;
    struct Vec_u8 strengthInfo;
    struct Vec_u8 diseaseIdx;
    struct Vec_i32 diseaseCount;
    struct Vec_u8 diseaseInfestationTickCount;
    struct Vec_f32 diseaseAccumulatedError;
    struct Vec_f32 unk_0xf0;
};

struct ElementConsumer {
    void * vtable;
    struct Vec_ElementConsumerData data;
    struct Vec_u32 denseToHandle;
    struct Vec_u32 freeHandles;
    struct Vec_i32 handleToDense;
    struct Vec_u8 versions;
    void * componentVtable;
    struct Vec_i32 visited;
    uint8_t queue[80];
    struct Vec_i32 reachable;
    struct Vec_ConsumedMassInfo consumed;
};

struct ElementEmitter {
    void * vtable;
    struct Vec_ElementEmitterData data;
    struct Vec_u32 denseToHandle;
    struct Vec_u32 freeHandles;
    struct Vec_i32 handleToDense;
    struct Vec_u8 versions;
    void * componentVtable;
    struct Vec_EmittedMassInfo emitted;
    struct Vec_i32 visited;
    struct Vec_i32 reachable;
    uint8_t queue[80];
};

struct BuildingHeatExchange {
    void * vtable;
    struct Vec_BuildingHeatExchangeData data;
    struct Vec_u32 denseToHandle;
    struct Vec_u32 freeHandles;
    struct Vec_i32 handleToDense;
    struct Vec_u8 versions;
    void * componentVtable;
    struct Vec_BuildingTemperatureInfo temperatures;
};

struct BuildingToBuildingHeatExchange {
    void * vtable;
    struct Vec_BuildingToBuildingHeatExchangeData data;
    struct Vec_u32 denseToHandle;
    struct Vec_u32 freeHandles;
    struct Vec_i32 handleToDense;
    struct Vec_u8 versions;
    void * componentVtable;
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
    struct CellSOA * cells;
    struct CellSOA * updatedCells;
    struct BackwallSOA * backwall;
    uint8_t * unk_0x38;
    struct Vec_f32 radiationLight;
    float * unk_0x58;
    struct Vector4f * unk_0x60;
    uint8_t * unk_0x68;
    struct Vec_ActiveRegion activeRegions;
    uint8_t * unk_0x88;
    uint16_t displacementDirection;
    uint8_t pad_0x92[6];
    struct SimEvents * events;
    float unk_0xa0;
    float unk_0xa4;
    uint8_t unk_0xa8;
    uint8_t pad_0xa9[7];
    struct Vec_ptr components;
    struct ElementConsumer elementConsumer;
    struct ElementEmitter elementEmitter;
    uint8_t pad_0x308[296];
    struct BuildingHeatExchange buildingHeatExchange;
    struct BuildingToBuildingHeatExchange buildingToBuildingHeatExchange;
    uint8_t pad_0x558[448];
    uint8_t unk_0x718;
    uint8_t headless;
    uint8_t radiationEnabled;
    uint8_t pad_0x71b[1];
    float unk_0x71c;
    float unk_0x720;
    float unk_0x724;
    float unk_0x728;
    float unk_0x72c;
    uint8_t pad_0x730[24];
};

struct Vector4f {
    float x;
    float y;
    float z;
    float w;
};

struct CellAccessor {
    struct CellSOA * soa;
    int32_t cell;
    uint8_t pad_0xc[4];
};

struct SimEvents {
    struct Vec_SubstanceChangeInfo substanceChanges;
    struct Vec_SpawnFallingLiquidInfo spawnFallingLiquid;
    struct Vec_SpawnOreInfo spawnOre;
    struct Vec_UnstableCellInfo unstableCells;
    struct Vec_MeltedInfo unk_melted_0x60;
    struct Vec_MeltedInfo buildingMelted;
    struct Vec_MeltedInfo buildingOverheat;
    struct Vec_MeltedInfo buildingNoLongerOverheated;
    struct Vec_CellMeltedInfo cellMelted;
    struct Vec_CallbackInfo callbacks;
    struct Vec_WorldDamageInfo worldDamage;
    struct Vec_MassConsumedCallback massConsumed;
    struct Vec_ConsumedRadiationCallback radiationConsumed;
    struct Vec_MassEmittedCallback massEmitted;
    struct Vec_DiseaseConsumptionCallback diseaseConsumed;
    struct Vec_SpawnFXInfo spawnFX;
    struct Vec_ComponentStateChangedMessage componentStateChanged;
    struct Vec_i32 unk_0x198;
    struct Vec_BackwallElementChangedInfo backwallElementChanged;
    struct Vec_BackwallShouldTransitionInfo backwallShouldTransition;
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
    struct PhysicsData defaultValues;
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
    struct Vec_u16 elementIdx;
    struct Vec_f32 mass;
    struct Vec_f32 temperature;
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
    struct SimData * sim;
    struct SimEvents * events;
    struct Region region;
};

struct UpdateCellSOATask {
    void * vtable;
    struct CellSOA * src;
    struct CellSOA * dst;
    int32_t numRows;
    int32_t firstRow;
    int32_t srcStride;
    int32_t dstFirstRow;
    int32_t rowLength;
    uint8_t pad_0x2c[4];
};

struct UpdateBackwallTask {
    void * vtable;
    struct BackwallSOA * src;
    struct BackwallSOA * dst;
    int32_t numRows;
    int32_t firstRow;
    int32_t srcStride;
    int32_t dstFirstRow;
    int32_t rowLength;
    uint8_t pad_0x2c[4];
};

struct BuildingHeatExchangeData {
    float temperature;
    float overheatTemperature;
    float operatingKilowatts;
    float heatCapacity;
    float heatCapacityPerCell;
    float conductivity;
    float meltTemperature;
    struct Region area;
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
    struct Vec_BuildingContact contacts;
};

struct ConduitTemperatureData {
    float temperature;
    float contentsConductivity;
    float contentsHeatCapacity;
    uint32_t structureHandle;
    float conduitHeatCapacity;
    float conduitConductivity;
    uint8_t insulated;
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
    struct Vec_ConduitTemperatureData data;
    struct Vec_u32 denseToHandle;
    struct Vec_u32 freeHandles;
    struct Vec_i32 handleToDense;
    struct Vec_u8 versions;
    struct Vec_i32 melted;
    struct Vec_i32 frozen;
    struct Vec_f32 temperatures;
    struct ConduitTemperatureUpdateData result;
    uint8_t pad_0xec[52];
    uint8_t mutex[40];
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

//@global gElements Vec_Element 0
//@global gElementTemperatureData Vec_ElementTemperatureData 0
//@global gElementPostProcessData Vec_ElementPostProcessData 0
//@global gElementLiquidData Vec_ElementLiquidData 0
//@global gElementPhysicsData Vec_PhysicsData 0
//@global gElementPressureData Vec_ElementPressureData 0
//@global gElementPropertyTextureData Vec_ElementPropertyTextureData 0
//@global gElementLightAbsorptionData Vec_ElementLightAbsorptionData 0
//@global gElementRadiationData Vec_ElementRadiationData 0
//@global gElementStateData Vec_u8 0
//@global gSimData SimData 1
//@global gConduitTemperatureManager ConduitTemperatureManager 1
