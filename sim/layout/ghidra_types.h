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

struct Vec_ActiveRegion { struct ActiveRegion *begin; struct ActiveRegion *end; struct ActiveRegion *cap; };
struct Vec_AddBuildingHeatExchangeMessage { struct AddBuildingHeatExchangeMessage *begin; struct AddBuildingHeatExchangeMessage *end; struct AddBuildingHeatExchangeMessage *cap; };
struct Vec_AddDiseaseConsumerMessage { struct AddDiseaseConsumerMessage *begin; struct AddDiseaseConsumerMessage *end; struct AddDiseaseConsumerMessage *cap; };
struct Vec_AddDiseaseEmitterMessage { struct AddDiseaseEmitterMessage *begin; struct AddDiseaseEmitterMessage *end; struct AddDiseaseEmitterMessage *cap; };
struct Vec_AddElementChunkMessage { struct AddElementChunkMessage *begin; struct AddElementChunkMessage *end; struct AddElementChunkMessage *cap; };
struct Vec_AddElementConsumerMessage { struct AddElementConsumerMessage *begin; struct AddElementConsumerMessage *end; struct AddElementConsumerMessage *cap; };
struct Vec_AddElementEmitterMessage { struct AddElementEmitterMessage *begin; struct AddElementEmitterMessage *end; struct AddElementEmitterMessage *cap; };
struct Vec_AddInContactBuildingToBuildingToBuildingHeatExchangeMessage { struct AddInContactBuildingToBuildingToBuildingHeatExchangeMessage *begin; struct AddInContactBuildingToBuildingToBuildingHeatExchangeMessage *end; struct AddInContactBuildingToBuildingToBuildingHeatExchangeMessage *cap; };
struct Vec_AddRadiationEmitterMessage { struct AddRadiationEmitterMessage *begin; struct AddRadiationEmitterMessage *end; struct AddRadiationEmitterMessage *cap; };
struct Vec_BackwallElementChangedInfo { struct BackwallElementChangedInfo *begin; struct BackwallElementChangedInfo *end; struct BackwallElementChangedInfo *cap; };
struct Vec_BackwallShouldTransitionInfo { struct BackwallShouldTransitionInfo *begin; struct BackwallShouldTransitionInfo *end; struct BackwallShouldTransitionInfo *cap; };
struct Vec_BuildingContact { struct BuildingContact *begin; struct BuildingContact *end; struct BuildingContact *cap; };
struct Vec_BuildingHeatExchangeData { struct BuildingHeatExchangeData *begin; struct BuildingHeatExchangeData *end; struct BuildingHeatExchangeData *cap; };
struct Vec_BuildingTemperatureInfo { struct BuildingTemperatureInfo *begin; struct BuildingTemperatureInfo *end; struct BuildingTemperatureInfo *cap; };
struct Vec_BuildingToBuildingHeatExchangeData { struct BuildingToBuildingHeatExchangeData *begin; struct BuildingToBuildingHeatExchangeData *end; struct BuildingToBuildingHeatExchangeData *cap; };
struct Vec_CallbackInfo { struct CallbackInfo *begin; struct CallbackInfo *end; struct CallbackInfo *cap; };
struct Vec_CellDiseaseModification { struct CellDiseaseModification *begin; struct CellDiseaseModification *end; struct CellDiseaseModification *cap; };
struct Vec_CellEnergyModification { struct CellEnergyModification *begin; struct CellEnergyModification *end; struct CellEnergyModification *cap; };
struct Vec_CellMeltedInfo { struct CellMeltedInfo *begin; struct CellMeltedInfo *end; struct CellMeltedInfo *cap; };
struct Vec_CellModification { struct CellModification *begin; struct CellModification *end; struct CellModification *cap; };
struct Vec_CellPropertiesChange { struct CellPropertiesChange *begin; struct CellPropertiesChange *end; struct CellPropertiesChange *cap; };
struct Vec_CellRadiationModification { struct CellRadiationModification *begin; struct CellRadiationModification *end; struct CellRadiationModification *cap; };
struct Vec_CellWorldZoneModification { struct CellWorldZoneModification *begin; struct CellWorldZoneModification *end; struct CellWorldZoneModification *cap; };
struct Vec_ComponentStateChangedMessage { struct ComponentStateChangedMessage *begin; struct ComponentStateChangedMessage *end; struct ComponentStateChangedMessage *cap; };
struct Vec_ConduitTemperatureData { struct ConduitTemperatureData *begin; struct ConduitTemperatureData *end; struct ConduitTemperatureData *cap; };
struct Vec_ConsumeDisease { struct ConsumeDisease *begin; struct ConsumeDisease *end; struct ConsumeDisease *cap; };
struct Vec_ConsumedMassInfo { struct ConsumedMassInfo *begin; struct ConsumedMassInfo *end; struct ConsumedMassInfo *cap; };
struct Vec_ConsumedRadiationCallback { struct ConsumedRadiationCallback *begin; struct ConsumedRadiationCallback *end; struct ConsumedRadiationCallback *cap; };
struct Vec_DigPoint { struct DigPoint *begin; struct DigPoint *end; struct DigPoint *cap; };
struct Vec_DiseaseConsumerData { struct DiseaseConsumerData *begin; struct DiseaseConsumerData *end; struct DiseaseConsumerData *cap; };
struct Vec_DiseaseConsumptionCallback { struct DiseaseConsumptionCallback *begin; struct DiseaseConsumptionCallback *end; struct DiseaseConsumptionCallback *cap; };
struct Vec_DiseaseEmittedInfo { struct DiseaseEmittedInfo *begin; struct DiseaseEmittedInfo *end; struct DiseaseEmittedInfo *cap; };
struct Vec_DiseaseEmitterData { struct DiseaseEmitterData *begin; struct DiseaseEmitterData *end; struct DiseaseEmitterData *cap; };
struct Vec_Element { struct Element *begin; struct Element *end; struct Element *cap; };
struct Vec_ElementChunkData { struct ElementChunkData *begin; struct ElementChunkData *end; struct ElementChunkData *cap; };
struct Vec_ElementChunkInfo { struct ElementChunkInfo *begin; struct ElementChunkInfo *end; struct ElementChunkInfo *cap; };
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
struct Vec_LiquidChangeInfo { struct LiquidChangeInfo *begin; struct LiquidChangeInfo *end; struct LiquidChangeInfo *cap; };
struct Vec_MassConsumedCallback { struct MassConsumedCallback *begin; struct MassConsumedCallback *end; struct MassConsumedCallback *cap; };
struct Vec_MassConsumption { struct MassConsumption *begin; struct MassConsumption *end; struct MassConsumption *cap; };
struct Vec_MassEmission { struct MassEmission *begin; struct MassEmission *end; struct MassEmission *cap; };
struct Vec_MassEmittedCallback { struct MassEmittedCallback *begin; struct MassEmittedCallback *end; struct MassEmittedCallback *cap; };
struct Vec_MeltedInfo { struct MeltedInfo *begin; struct MeltedInfo *end; struct MeltedInfo *cap; };
struct Vec_ModifyBackwallDataMessage { struct ModifyBackwallDataMessage *begin; struct ModifyBackwallDataMessage *end; struct ModifyBackwallDataMessage *cap; };
struct Vec_ModifyBuildingEnergyMessage { struct ModifyBuildingEnergyMessage *begin; struct ModifyBuildingEnergyMessage *end; struct ModifyBuildingEnergyMessage *cap; };
struct Vec_ModifyBuildingHeatExchangeMessage { struct ModifyBuildingHeatExchangeMessage *begin; struct ModifyBuildingHeatExchangeMessage *end; struct ModifyBuildingHeatExchangeMessage *cap; };
struct Vec_ModifyDiseaseConsumerMessage { struct ModifyDiseaseConsumerMessage *begin; struct ModifyDiseaseConsumerMessage *end; struct ModifyDiseaseConsumerMessage *cap; };
struct Vec_ModifyDiseaseEmitterMessage { struct ModifyDiseaseEmitterMessage *begin; struct ModifyDiseaseEmitterMessage *end; struct ModifyDiseaseEmitterMessage *cap; };
struct Vec_ModifyElementChunkAdjusterMessage { struct ModifyElementChunkAdjusterMessage *begin; struct ModifyElementChunkAdjusterMessage *end; struct ModifyElementChunkAdjusterMessage *cap; };
struct Vec_ModifyElementChunkEnergyMessage { struct ModifyElementChunkEnergyMessage *begin; struct ModifyElementChunkEnergyMessage *end; struct ModifyElementChunkEnergyMessage *cap; };
struct Vec_ModifyElementChunkMessage { struct ModifyElementChunkMessage *begin; struct ModifyElementChunkMessage *end; struct ModifyElementChunkMessage *cap; };
struct Vec_ModifyElementConsumerMessage { struct ModifyElementConsumerMessage *begin; struct ModifyElementConsumerMessage *end; struct ModifyElementConsumerMessage *cap; };
struct Vec_ModifyElementEmitterMessage { struct ModifyElementEmitterMessage *begin; struct ModifyElementEmitterMessage *end; struct ModifyElementEmitterMessage *cap; };
struct Vec_ModifyRadiationEmitterMessage { struct ModifyRadiationEmitterMessage *begin; struct ModifyRadiationEmitterMessage *end; struct ModifyRadiationEmitterMessage *cap; };
struct Vec_MoveElementChunkMessage { struct MoveElementChunkMessage *begin; struct MoveElementChunkMessage *end; struct MoveElementChunkMessage *cap; };
struct Vec_PhysicsData { struct PhysicsData *begin; struct PhysicsData *end; struct PhysicsData *cap; };
struct Vec_RadiationEmitterData { struct RadiationEmitterData *begin; struct RadiationEmitterData *end; struct RadiationEmitterData *cap; };
struct Vec_RadiationParamsModification { struct RadiationParamsModification *begin; struct RadiationParamsModification *end; struct RadiationParamsModification *cap; };
struct Vec_RegisterBuildingToBuildingHeatExchangeMessage { struct RegisterBuildingToBuildingHeatExchangeMessage *begin; struct RegisterBuildingToBuildingHeatExchangeMessage *end; struct RegisterBuildingToBuildingHeatExchangeMessage *cap; };
struct Vec_RemoveBuildingHeatExchangeMessage { struct RemoveBuildingHeatExchangeMessage *begin; struct RemoveBuildingHeatExchangeMessage *end; struct RemoveBuildingHeatExchangeMessage *cap; };
struct Vec_RemoveBuildingInContactFromBuildingToBuildingHeatExchangeMessage { struct RemoveBuildingInContactFromBuildingToBuildingHeatExchangeMessage *begin; struct RemoveBuildingInContactFromBuildingToBuildingHeatExchangeMessage *end; struct RemoveBuildingInContactFromBuildingToBuildingHeatExchangeMessage *cap; };
struct Vec_RemoveBuildingToBuildingHeatExchangeMessage { struct RemoveBuildingToBuildingHeatExchangeMessage *begin; struct RemoveBuildingToBuildingHeatExchangeMessage *end; struct RemoveBuildingToBuildingHeatExchangeMessage *cap; };
struct Vec_RemoveDiseaseConsumerMessage { struct RemoveDiseaseConsumerMessage *begin; struct RemoveDiseaseConsumerMessage *end; struct RemoveDiseaseConsumerMessage *cap; };
struct Vec_RemoveDiseaseEmitterMessage { struct RemoveDiseaseEmitterMessage *begin; struct RemoveDiseaseEmitterMessage *end; struct RemoveDiseaseEmitterMessage *cap; };
struct Vec_RemoveElementChunkMessage { struct RemoveElementChunkMessage *begin; struct RemoveElementChunkMessage *end; struct RemoveElementChunkMessage *cap; };
struct Vec_RemoveElementConsumerMessage { struct RemoveElementConsumerMessage *begin; struct RemoveElementConsumerMessage *end; struct RemoveElementConsumerMessage *cap; };
struct Vec_RemoveElementEmitterMessage { struct RemoveElementEmitterMessage *begin; struct RemoveElementEmitterMessage *end; struct RemoveElementEmitterMessage *cap; };
struct Vec_RemoveRadiationEmitterMessage { struct RemoveRadiationEmitterMessage *begin; struct RemoveRadiationEmitterMessage *end; struct RemoveRadiationEmitterMessage *cap; };
struct Vec_SetCellFloatValue { struct SetCellFloatValue *begin; struct SetCellFloatValue *end; struct SetCellFloatValue *cap; };
struct Vec_SolidInfo { struct SolidInfo *begin; struct SolidInfo *end; struct SolidInfo *cap; };
struct Vec_SolidSubstanceChangeInfo { struct SolidSubstanceChangeInfo *begin; struct SolidSubstanceChangeInfo *end; struct SolidSubstanceChangeInfo *cap; };
struct Vec_SpawnFXInfo { struct SpawnFXInfo *begin; struct SpawnFXInfo *end; struct SpawnFXInfo *cap; };
struct Vec_SpawnFallingLiquidInfo { struct SpawnFallingLiquidInfo *begin; struct SpawnFallingLiquidInfo *end; struct SpawnFallingLiquidInfo *cap; };
struct Vec_SpawnOreInfo { struct SpawnOreInfo *begin; struct SpawnOreInfo *end; struct SpawnOreInfo *cap; };
struct Vec_SubstanceChangeInfo { struct SubstanceChangeInfo *begin; struct SubstanceChangeInfo *end; struct SubstanceChangeInfo *cap; };
struct Vec_UnstableCellInfo { struct UnstableCellInfo *begin; struct UnstableCellInfo *end; struct UnstableCellInfo *cap; };
struct Vec_WorldDamageInfo { struct WorldDamageInfo *begin; struct WorldDamageInfo *end; struct WorldDamageInfo *cap; };
struct Vec_WorldRect { struct WorldRect *begin; struct WorldRect *end; struct WorldRect *cap; };
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

struct RadiationEmitter {
    void * vtable;
    struct Vec_RadiationEmitterData data;
    struct Vec_u32 denseToHandle;
    struct Vec_u32 freeHandles;
    struct Vec_i32 handleToDense;
    struct Vec_u8 versions;
    void * componentVtable;
};

struct ElementChunk {
    void * vtable;
    struct Vec_ElementChunkData data;
    struct Vec_u32 denseToHandle;
    struct Vec_u32 freeHandles;
    struct Vec_i32 handleToDense;
    struct Vec_u8 versions;
    void * componentVtable;
    struct Vec_ElementChunkInfo info;
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

struct DiseaseEmitter {
    void * vtable;
    struct Vec_DiseaseEmitterData data;
    struct Vec_u32 denseToHandle;
    struct Vec_u32 freeHandles;
    struct Vec_i32 handleToDense;
    struct Vec_u8 versions;
    void * componentVtable;
    struct Vec_DiseaseEmittedInfo emitted;
    uint8_t queue[80];
    struct Vec_i32 visited;
    struct Vec_i32 reachable;
};

struct DiseaseConsumer {
    void * vtable;
    struct Vec_DiseaseConsumerData data;
    struct Vec_u32 denseToHandle;
    struct Vec_u32 freeHandles;
    struct Vec_i32 handleToDense;
    struct Vec_u8 versions;
    void * componentVtable;
    struct Vec_DiseaseEmittedInfo consumed;
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
    struct RadiationEmitter radiationEmitter;
    struct ElementChunk elementChunk;
    struct BuildingHeatExchange buildingHeatExchange;
    struct BuildingToBuildingHeatExchange buildingToBuildingHeatExchange;
    struct DiseaseEmitter diseaseEmitter;
    struct DiseaseConsumer diseaseConsumer;
    uint8_t unk_0x718;
    uint8_t headless;
    uint8_t radiationEnabled;
    uint8_t pad_0x71b[1];
    float unk_0x71c;
    float unk_0x720;
    float unk_0x724;
    float unk_0x728;
    float unk_0x72c;
    struct Vec_WorldRect worldRects;
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
    struct Vec_MeltedInfo elementChunkMelted;
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
    struct Vec_SpawnOreInfo digInfo;
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
    struct Vec_SubstanceChangeInfo substanceChanges;
    struct Vec_CallbackInfo callbacks;
    struct Vec_SpawnFallingLiquidInfo spawnFallingLiquid;
    struct Vec_SpawnOreInfo spawnOre;
    struct Vec_SpawnOreInfo digInfo;
    struct Vec_UnstableCellInfo unstableCells;
    struct Vec_WorldDamageInfo worldDamage;
    struct Vec_BuildingTemperatureInfo buildingTemperatures;
    struct Vec_MassConsumedCallback massConsumed;
    struct Vec_MassEmittedCallback massEmitted;
    struct Vec_DiseaseConsumptionCallback diseaseConsumed;
    struct Vec_ConsumedRadiationCallback radiationConsumed;
    struct Vec_SpawnFXInfo spawnFX;
    struct Vec_SolidInfo solidInfo;
    struct Vec_LiquidChangeInfo liquidChanges;
    struct Vec_SolidSubstanceChangeInfo solidSubstanceChanges;
    struct Vec_ConsumedMassInfo removedMass;
    struct Vec_EmittedMassInfo emittedMass;
    struct Vec_ElementChunkInfo elementChunkInfos;
    struct Vec_DiseaseEmittedInfo diseaseEmitted;
    struct Vec_DiseaseEmittedInfo diseaseConsumedInfos;
    struct Vec_MeltedInfo elementChunkMelted;
    struct Vec_MeltedInfo buildingMelted;
    struct Vec_MeltedInfo buildingOverheat;
    struct Vec_MeltedInfo buildingNoLongerOverheated;
    struct Vec_ComponentStateChangedMessage componentStateChanged;
    struct Vec_CellMeltedInfo cellMelted;
    struct Vec_BackwallElementChangedInfo backwallElementChanged;
    struct Vec_BackwallShouldTransitionInfo backwallShouldTransition;
    struct CellSOA * cells;
    struct BackwallSOA * backwall;
    struct Vector4f * flow;
    float * accumulatedFlow;
    uint8_t * visibleGrid;
    void * propertyTextureFlow;
    void * propertyTextureLiquid;
    void * propertyTextureLiquidData;
    void * propertyTextureMaterialData;
    uint8_t * propertyTextureExposedToSunlight;
};

struct DigPoint {
    int32_t cellIdx;
    int32_t callbackIdx;
    uint8_t skipEvent;
    uint8_t backwall;
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
    struct Vec_DigPoint digs;
    struct Vec_CellModification cellModifications;
    struct Vec_CellEnergyModification cellEnergyModifications;
    struct Vec_u8 unk_0x50;
    struct Vec_SetCellFloatValue insulationChanges;
    struct Vec_SetCellFloatValue strengthChanges;
    struct Vec_CellPropertiesChange propertyChanges;
    struct Vec_u8 unk_0xb0;
    struct Vec_ConsumeDisease diseaseConsumptions;
    struct Vec_CellDiseaseModification diseaseModifications;
    struct Vec_CellRadiationModification radiationModifications;
    struct Vec_RadiationParamsModification radiationParams;
    struct Vec_CellWorldZoneModification worldZoneModifications;
    struct Vec_MassConsumption massConsumptions;
    struct Vec_MassEmission massEmissions;
    struct Vec_AddBuildingHeatExchangeMessage addBuildingHeatExchange;
    struct Vec_ModifyBuildingHeatExchangeMessage modifyBuildingHeatExchange;
    struct Vec_RemoveBuildingHeatExchangeMessage removeBuildingHeatExchange;
    struct Vec_ModifyBuildingEnergyMessage modifyBuildingEnergy;
    struct Vec_RegisterBuildingToBuildingHeatExchangeMessage registerBuildingToBuilding;
    struct Vec_RemoveBuildingInContactFromBuildingToBuildingHeatExchangeMessage removeBuildingInContact;
    struct Vec_RemoveBuildingToBuildingHeatExchangeMessage removeBuildingToBuilding;
    struct Vec_AddInContactBuildingToBuildingToBuildingHeatExchangeMessage addBuildingInContact;
    struct Vec_AddElementChunkMessage addElementChunk;
    struct Vec_ModifyElementChunkMessage modifyElementChunk;
    struct Vec_RemoveElementChunkMessage removeElementChunk;
    struct Vec_MoveElementChunkMessage moveElementChunk;
    struct Vec_ModifyElementChunkEnergyMessage modifyElementChunkEnergy;
    struct Vec_ModifyElementChunkAdjusterMessage modifyElementChunkAdjuster;
    struct Vec_ModifyBackwallDataMessage modifyBackwall;
    struct Vec_AddElementConsumerMessage addElementConsumer;
    struct Vec_ModifyElementConsumerMessage modifyElementConsumer;
    struct Vec_RemoveElementConsumerMessage removeElementConsumer;
    struct Vec_AddElementEmitterMessage addElementEmitter;
    struct Vec_ModifyElementEmitterMessage modifyElementEmitter;
    struct Vec_RemoveElementEmitterMessage removeElementEmitter;
    struct Vec_AddDiseaseEmitterMessage addDiseaseEmitter;
    struct Vec_ModifyDiseaseEmitterMessage modifyDiseaseEmitter;
    struct Vec_RemoveDiseaseEmitterMessage removeDiseaseEmitter;
    struct Vec_AddDiseaseConsumerMessage addDiseaseConsumer;
    struct Vec_ModifyDiseaseConsumerMessage modifyDiseaseConsumer;
    struct Vec_RemoveDiseaseConsumerMessage removeDiseaseConsumer;
    struct Vec_AddRadiationEmitterMessage addRadiationEmitter;
    struct Vec_ModifyRadiationEmitterMessage modifyRadiationEmitter;
    struct Vec_RemoveRadiationEmitterMessage removeRadiationEmitter;
    struct DebugProperties debugProperties;
    uint8_t pad_0x44c[4];
};

struct SimFrameManager {
    uint8_t mutex[40];
    struct SimFrameInfo * current;
    struct Vec_ptr freeFrames;
    struct Vec_ptr queuedFrames;
    struct Vec_ptr processingFrames;
    struct Vec_ptr processedFrames;
    uint8_t pad_0x90[32];
    struct Vec_ActiveRegion activeRegions;
    int32_t nextFrame;
    uint8_t pad_0xcc[4];
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
