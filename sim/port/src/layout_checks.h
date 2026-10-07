// Generated from sim/layout/layout.txt. Do not edit.
// Included by the port in hybrid builds, after its own struct definitions.
#pragma once
#include <cstddef>

static_assert(sizeof(CellSOA) == 0x108, "CellSOA size");
static_assert(offsetof(CellSOA, elementIdx) == 0x0, "CellSOA::elementIdx");
static_assert(offsetof(CellSOA, temperature) == 0x18, "CellSOA::temperature");
static_assert(offsetof(CellSOA, mass) == 0x30, "CellSOA::mass");
static_assert(offsetof(CellSOA, properties) == 0x48, "CellSOA::properties");
static_assert(offsetof(CellSOA, insulation) == 0x60, "CellSOA::insulation");
static_assert(offsetof(CellSOA, strengthInfo) == 0x78, "CellSOA::strengthInfo");
static_assert(offsetof(CellSOA, diseaseIdx) == 0x90, "CellSOA::diseaseIdx");
static_assert(offsetof(CellSOA, diseaseCount) == 0xa8, "CellSOA::diseaseCount");
static_assert(offsetof(CellSOA, diseaseInfestationTickCount) == 0xc0, "CellSOA::diseaseInfestationTickCount");
static_assert(offsetof(CellSOA, diseaseAccumulatedError) == 0xd8, "CellSOA::diseaseAccumulatedError");

static_assert(sizeof(SimData) == 0x748, "SimData size");
static_assert(offsetof(SimData, width) == 0x0, "SimData::width");
static_assert(offsetof(SimData, height) == 0x4, "SimData::height");
static_assert(offsetof(SimData, numGameCells) == 0x8, "SimData::numGameCells");
static_assert(offsetof(SimData, rngState) == 0xc, "SimData::rngState");
static_assert(offsetof(SimData, flowDirection) == 0x10, "SimData::flowDirection");
static_assert(offsetof(SimData, gasDirection) == 0x14, "SimData::gasDirection");
static_assert(offsetof(SimData, savedOptions) == 0x18, "SimData::savedOptions");
static_assert(offsetof(SimData, vacuumElementIdx) == 0x1c, "SimData::vacuumElementIdx");
static_assert(offsetof(SimData, cells) == 0x20, "SimData::cells");
static_assert(offsetof(SimData, updatedCells) == 0x28, "SimData::updatedCells");
static_assert(offsetof(SimData, backwall) == 0x30, "SimData::backwall");
static_assert(offsetof(SimData, radiationLight) == 0x40, "SimData::radiationLight");
static_assert(offsetof(SimData, activeRegions) == 0x70, "SimData::activeRegions");
static_assert(offsetof(SimData, displacementDirection) == 0x90, "SimData::displacementDirection");
static_assert(offsetof(SimData, events) == 0x98, "SimData::events");
static_assert(offsetof(SimData, components) == 0xb0, "SimData::components");
static_assert(offsetof(SimData, elementConsumer) == 0xc8, "SimData::elementConsumer");
static_assert(offsetof(SimData, elementEmitter) == 0x1e8, "SimData::elementEmitter");
static_assert(offsetof(SimData, radiationEmitter) == 0x308, "SimData::radiationEmitter");
static_assert(offsetof(SimData, elementChunk) == 0x390, "SimData::elementChunk");
static_assert(offsetof(SimData, buildingHeatExchange) == 0x430, "SimData::buildingHeatExchange");
static_assert(offsetof(SimData, buildingToBuildingHeatExchange) == 0x4d0, "SimData::buildingToBuildingHeatExchange");
static_assert(offsetof(SimData, diseaseEmitter) == 0x558, "SimData::diseaseEmitter");
static_assert(offsetof(SimData, diseaseConsumer) == 0x678, "SimData::diseaseConsumer");
static_assert(offsetof(SimData, headless) == 0x719, "SimData::headless");
static_assert(offsetof(SimData, radiationEnabled) == 0x71a, "SimData::radiationEnabled");
static_assert(offsetof(SimData, worldRects) == 0x730, "SimData::worldRects");

static_assert(sizeof(Vector4f) == 0x10, "Vector4f size");
static_assert(offsetof(Vector4f, x) == 0x0, "Vector4f::x");
static_assert(offsetof(Vector4f, y) == 0x4, "Vector4f::y");
static_assert(offsetof(Vector4f, z) == 0x8, "Vector4f::z");
static_assert(offsetof(Vector4f, w) == 0xc, "Vector4f::w");

static_assert(sizeof(CellAccessor) == 0x10, "CellAccessor size");
static_assert(offsetof(CellAccessor, soa) == 0x0, "CellAccessor::soa");
static_assert(offsetof(CellAccessor, cell) == 0x8, "CellAccessor::cell");

static_assert(sizeof(SimEvents) == 0x1e0, "SimEvents size");
static_assert(offsetof(SimEvents, substanceChanges) == 0x0, "SimEvents::substanceChanges");
static_assert(offsetof(SimEvents, spawnFallingLiquid) == 0x18, "SimEvents::spawnFallingLiquid");
static_assert(offsetof(SimEvents, spawnOre) == 0x30, "SimEvents::spawnOre");
static_assert(offsetof(SimEvents, unstableCells) == 0x48, "SimEvents::unstableCells");
static_assert(offsetof(SimEvents, elementChunkMelted) == 0x60, "SimEvents::elementChunkMelted");
static_assert(offsetof(SimEvents, buildingMelted) == 0x78, "SimEvents::buildingMelted");
static_assert(offsetof(SimEvents, buildingOverheat) == 0x90, "SimEvents::buildingOverheat");
static_assert(offsetof(SimEvents, buildingNoLongerOverheated) == 0xa8, "SimEvents::buildingNoLongerOverheated");
static_assert(offsetof(SimEvents, cellMelted) == 0xc0, "SimEvents::cellMelted");
static_assert(offsetof(SimEvents, callbacks) == 0xd8, "SimEvents::callbacks");
static_assert(offsetof(SimEvents, worldDamage) == 0xf0, "SimEvents::worldDamage");
static_assert(offsetof(SimEvents, massConsumed) == 0x108, "SimEvents::massConsumed");
static_assert(offsetof(SimEvents, radiationConsumed) == 0x120, "SimEvents::radiationConsumed");
static_assert(offsetof(SimEvents, massEmitted) == 0x138, "SimEvents::massEmitted");
static_assert(offsetof(SimEvents, diseaseConsumed) == 0x150, "SimEvents::diseaseConsumed");
static_assert(offsetof(SimEvents, spawnFX) == 0x168, "SimEvents::spawnFX");
static_assert(offsetof(SimEvents, componentStateChanged) == 0x180, "SimEvents::componentStateChanged");
static_assert(offsetof(SimEvents, digInfo) == 0x198, "SimEvents::digInfo");
static_assert(offsetof(SimEvents, backwallElementChanged) == 0x1b0, "SimEvents::backwallElementChanged");
static_assert(offsetof(SimEvents, backwallShouldTransition) == 0x1c8, "SimEvents::backwallShouldTransition");

static_assert(sizeof(SubstanceChangeInfo) == 0x8, "SubstanceChangeInfo size");
static_assert(offsetof(SubstanceChangeInfo, cellIdx) == 0x0, "SubstanceChangeInfo::cellIdx");
static_assert(offsetof(SubstanceChangeInfo, oldElemIdx) == 0x4, "SubstanceChangeInfo::oldElemIdx");
static_assert(offsetof(SubstanceChangeInfo, newElemIdx) == 0x6, "SubstanceChangeInfo::newElemIdx");

static_assert(sizeof(SpawnFallingLiquidInfo) == 0x14, "SpawnFallingLiquidInfo size");
static_assert(offsetof(SpawnFallingLiquidInfo, cellIdx) == 0x0, "SpawnFallingLiquidInfo::cellIdx");
static_assert(offsetof(SpawnFallingLiquidInfo, elemIdx) == 0x4, "SpawnFallingLiquidInfo::elemIdx");
static_assert(offsetof(SpawnFallingLiquidInfo, diseaseIdx) == 0x6, "SpawnFallingLiquidInfo::diseaseIdx");
static_assert(offsetof(SpawnFallingLiquidInfo, mass) == 0x8, "SpawnFallingLiquidInfo::mass");
static_assert(offsetof(SpawnFallingLiquidInfo, temperature) == 0xc, "SpawnFallingLiquidInfo::temperature");
static_assert(offsetof(SpawnFallingLiquidInfo, diseaseCount) == 0x10, "SpawnFallingLiquidInfo::diseaseCount");

static_assert(sizeof(SpawnOreInfo) == 0x14, "SpawnOreInfo size");
static_assert(offsetof(SpawnOreInfo, cellIdx) == 0x0, "SpawnOreInfo::cellIdx");
static_assert(offsetof(SpawnOreInfo, elemIdx) == 0x4, "SpawnOreInfo::elemIdx");
static_assert(offsetof(SpawnOreInfo, diseaseIdx) == 0x6, "SpawnOreInfo::diseaseIdx");
static_assert(offsetof(SpawnOreInfo, mass) == 0x8, "SpawnOreInfo::mass");
static_assert(offsetof(SpawnOreInfo, temperature) == 0xc, "SpawnOreInfo::temperature");
static_assert(offsetof(SpawnOreInfo, diseaseCount) == 0x10, "SpawnOreInfo::diseaseCount");

static_assert(sizeof(UnstableCellInfo) == 0x14, "UnstableCellInfo size");
static_assert(offsetof(UnstableCellInfo, cellIdx) == 0x0, "UnstableCellInfo::cellIdx");
static_assert(offsetof(UnstableCellInfo, elemIdx) == 0x4, "UnstableCellInfo::elemIdx");
static_assert(offsetof(UnstableCellInfo, fallingInfo) == 0x6, "UnstableCellInfo::fallingInfo");
static_assert(offsetof(UnstableCellInfo, diseaseIdx) == 0x7, "UnstableCellInfo::diseaseIdx");
static_assert(offsetof(UnstableCellInfo, mass) == 0x8, "UnstableCellInfo::mass");
static_assert(offsetof(UnstableCellInfo, temperature) == 0xc, "UnstableCellInfo::temperature");
static_assert(offsetof(UnstableCellInfo, diseaseCount) == 0x10, "UnstableCellInfo::diseaseCount");

static_assert(sizeof(MeltedInfo) == 0x4, "MeltedInfo size");
static_assert(offsetof(MeltedInfo, handle) == 0x0, "MeltedInfo::handle");

static_assert(sizeof(CellMeltedInfo) == 0x4, "CellMeltedInfo size");
static_assert(offsetof(CellMeltedInfo, gameCell) == 0x0, "CellMeltedInfo::gameCell");

static_assert(sizeof(CallbackInfo) == 0x4, "CallbackInfo size");
static_assert(offsetof(CallbackInfo, callbackIdx) == 0x0, "CallbackInfo::callbackIdx");

static_assert(sizeof(WorldDamageInfo) == 0x8, "WorldDamageInfo size");
static_assert(offsetof(WorldDamageInfo, gameCell) == 0x0, "WorldDamageInfo::gameCell");
static_assert(offsetof(WorldDamageInfo, damageSourceOffset) == 0x4, "WorldDamageInfo::damageSourceOffset");

static_assert(sizeof(MassConsumedCallback) == 0x14, "MassConsumedCallback size");
static_assert(offsetof(MassConsumedCallback, callbackIdx) == 0x0, "MassConsumedCallback::callbackIdx");
static_assert(offsetof(MassConsumedCallback, elemIdx) == 0x4, "MassConsumedCallback::elemIdx");
static_assert(offsetof(MassConsumedCallback, diseaseIdx) == 0x6, "MassConsumedCallback::diseaseIdx");
static_assert(offsetof(MassConsumedCallback, mass) == 0x8, "MassConsumedCallback::mass");
static_assert(offsetof(MassConsumedCallback, temperature) == 0xc, "MassConsumedCallback::temperature");
static_assert(offsetof(MassConsumedCallback, diseaseCount) == 0x10, "MassConsumedCallback::diseaseCount");

static_assert(sizeof(MassEmittedCallback) == 0x14, "MassEmittedCallback size");
static_assert(offsetof(MassEmittedCallback, callbackIdx) == 0x0, "MassEmittedCallback::callbackIdx");
static_assert(offsetof(MassEmittedCallback, elemIdx) == 0x4, "MassEmittedCallback::elemIdx");
static_assert(offsetof(MassEmittedCallback, suceeded) == 0x6, "MassEmittedCallback::suceeded");
static_assert(offsetof(MassEmittedCallback, diseaseIdx) == 0x7, "MassEmittedCallback::diseaseIdx");
static_assert(offsetof(MassEmittedCallback, mass) == 0x8, "MassEmittedCallback::mass");
static_assert(offsetof(MassEmittedCallback, temperature) == 0xc, "MassEmittedCallback::temperature");
static_assert(offsetof(MassEmittedCallback, diseaseCount) == 0x10, "MassEmittedCallback::diseaseCount");

static_assert(sizeof(DiseaseConsumptionCallback) == 0xc, "DiseaseConsumptionCallback size");
static_assert(offsetof(DiseaseConsumptionCallback, callbackIdx) == 0x0, "DiseaseConsumptionCallback::callbackIdx");
static_assert(offsetof(DiseaseConsumptionCallback, diseaseIdx) == 0x4, "DiseaseConsumptionCallback::diseaseIdx");
static_assert(offsetof(DiseaseConsumptionCallback, diseaseCount) == 0x8, "DiseaseConsumptionCallback::diseaseCount");

static_assert(sizeof(ConsumedRadiationCallback) == 0xc, "ConsumedRadiationCallback size");
static_assert(offsetof(ConsumedRadiationCallback, callbackIdx) == 0x0, "ConsumedRadiationCallback::callbackIdx");
static_assert(offsetof(ConsumedRadiationCallback, gameCell) == 0x4, "ConsumedRadiationCallback::gameCell");
static_assert(offsetof(ConsumedRadiationCallback, radiation) == 0x8, "ConsumedRadiationCallback::radiation");

static_assert(sizeof(SpawnFXInfo) == 0xc, "SpawnFXInfo size");
static_assert(offsetof(SpawnFXInfo, cellIdx) == 0x0, "SpawnFXInfo::cellIdx");
static_assert(offsetof(SpawnFXInfo, fxHash) == 0x4, "SpawnFXInfo::fxHash");
static_assert(offsetof(SpawnFXInfo, rotation) == 0x8, "SpawnFXInfo::rotation");

static_assert(sizeof(ComponentStateChangedMessage) == 0x8, "ComponentStateChangedMessage size");
static_assert(offsetof(ComponentStateChangedMessage, callbackIdx) == 0x0, "ComponentStateChangedMessage::callbackIdx");
static_assert(offsetof(ComponentStateChangedMessage, simHandle) == 0x4, "ComponentStateChangedMessage::simHandle");

static_assert(sizeof(BackwallElementChangedInfo) == 0x4, "BackwallElementChangedInfo size");
static_assert(offsetof(BackwallElementChangedInfo, gameCell) == 0x0, "BackwallElementChangedInfo::gameCell");

static_assert(sizeof(BackwallShouldTransitionInfo) == 0x4, "BackwallShouldTransitionInfo size");
static_assert(offsetof(BackwallShouldTransitionInfo, gameCell) == 0x0, "BackwallShouldTransitionInfo::gameCell");

static_assert(sizeof(SolidInfo) == 0x8, "SolidInfo size");
static_assert(offsetof(SolidInfo, cellIdx) == 0x0, "SolidInfo::cellIdx");
static_assert(offsetof(SolidInfo, isSolid) == 0x4, "SolidInfo::isSolid");

static_assert(sizeof(LiquidChangeInfo) == 0x4, "LiquidChangeInfo size");
static_assert(offsetof(LiquidChangeInfo, cellIdx) == 0x0, "LiquidChangeInfo::cellIdx");

static_assert(sizeof(SolidSubstanceChangeInfo) == 0x4, "SolidSubstanceChangeInfo size");
static_assert(offsetof(SolidSubstanceChangeInfo, cellIdx) == 0x0, "SolidSubstanceChangeInfo::cellIdx");

static_assert(sizeof(Element) == 0xa4, "Element size");
static_assert(offsetof(Element, id) == 0x0, "Element::id");
static_assert(offsetof(Element, lowTempTransitionIdx) == 0x4, "Element::lowTempTransitionIdx");
static_assert(offsetof(Element, highTempTransitionIdx) == 0x6, "Element::highTempTransitionIdx");
static_assert(offsetof(Element, elementsTableIdx) == 0x8, "Element::elementsTableIdx");
static_assert(offsetof(Element, state) == 0xa, "Element::state");
static_assert(offsetof(Element, numberOfGradientColors) == 0xb, "Element::numberOfGradientColors");
static_assert(offsetof(Element, specificHeatCapacity) == 0xc, "Element::specificHeatCapacity");
static_assert(offsetof(Element, thermalConductivity) == 0x10, "Element::thermalConductivity");
static_assert(offsetof(Element, molarMass) == 0x14, "Element::molarMass");
static_assert(offsetof(Element, solidSurfaceAreaMultiplier) == 0x18, "Element::solidSurfaceAreaMultiplier");
static_assert(offsetof(Element, liquidSurfaceAreaMultiplier) == 0x1c, "Element::liquidSurfaceAreaMultiplier");
static_assert(offsetof(Element, gasSurfaceAreaMultiplier) == 0x20, "Element::gasSurfaceAreaMultiplier");
static_assert(offsetof(Element, flow) == 0x24, "Element::flow");
static_assert(offsetof(Element, viscosity) == 0x28, "Element::viscosity");
static_assert(offsetof(Element, minHorizontalFlow) == 0x2c, "Element::minHorizontalFlow");
static_assert(offsetof(Element, minVerticalFlow) == 0x30, "Element::minVerticalFlow");
static_assert(offsetof(Element, maxMass) == 0x34, "Element::maxMass");
static_assert(offsetof(Element, lowTemp) == 0x38, "Element::lowTemp");
static_assert(offsetof(Element, highTemp) == 0x3c, "Element::highTemp");
static_assert(offsetof(Element, strength) == 0x40, "Element::strength");
static_assert(offsetof(Element, lowTempTransitionOreID) == 0x44, "Element::lowTempTransitionOreID");
static_assert(offsetof(Element, lowTempTransitionOreMassConversion) == 0x48, "Element::lowTempTransitionOreMassConversion");
static_assert(offsetof(Element, highTempTransitionOreID) == 0x4c, "Element::highTempTransitionOreID");
static_assert(offsetof(Element, highTempTransitionOreMassConversion) == 0x50, "Element::highTempTransitionOreMassConversion");
static_assert(offsetof(Element, sublimateIndex) == 0x54, "Element::sublimateIndex");
static_assert(offsetof(Element, convertIndex) == 0x56, "Element::convertIndex");
static_assert(offsetof(Element, materialProperties) == 0x58, "Element::materialProperties");
static_assert(offsetof(Element, colour) == 0x5c, "Element::colour");
static_assert(offsetof(Element, gradientColours) == 0x60, "Element::gradientColours");
static_assert(offsetof(Element, sublimateFX) == 0x78, "Element::sublimateFX");
static_assert(offsetof(Element, sublimateRate) == 0x7c, "Element::sublimateRate");
static_assert(offsetof(Element, sublimateEfficiency) == 0x80, "Element::sublimateEfficiency");
static_assert(offsetof(Element, sublimateProbability) == 0x84, "Element::sublimateProbability");
static_assert(offsetof(Element, offGasProbability) == 0x88, "Element::offGasProbability");
static_assert(offsetof(Element, lightAbsorptionFactor) == 0x8c, "Element::lightAbsorptionFactor");
static_assert(offsetof(Element, radiationAbsorptionFactor) == 0x90, "Element::radiationAbsorptionFactor");
static_assert(offsetof(Element, radiationPer1000Mass) == 0x94, "Element::radiationPer1000Mass");
static_assert(offsetof(Element, defaultValues) == 0x98, "Element::defaultValues");

static_assert(sizeof(PhysicsData) == 0xc, "PhysicsData size");
static_assert(offsetof(PhysicsData, temperature) == 0x0, "PhysicsData::temperature");
static_assert(offsetof(PhysicsData, mass) == 0x4, "PhysicsData::mass");
static_assert(offsetof(PhysicsData, pressure) == 0x8, "PhysicsData::pressure");

static_assert(sizeof(ElementTemperatureData) == 0x38, "ElementTemperatureData size");
static_assert(offsetof(ElementTemperatureData, state) == 0x0, "ElementTemperatureData::state");
static_assert(offsetof(ElementTemperatureData, lowTempTransitionIdx) == 0x2, "ElementTemperatureData::lowTempTransitionIdx");
static_assert(offsetof(ElementTemperatureData, highTempTransitionIdx) == 0x4, "ElementTemperatureData::highTempTransitionIdx");
static_assert(offsetof(ElementTemperatureData, lowTempTransitionOreIdx) == 0x6, "ElementTemperatureData::lowTempTransitionOreIdx");
static_assert(offsetof(ElementTemperatureData, highTempTransitionOreIdx) == 0x8, "ElementTemperatureData::highTempTransitionOreIdx");
static_assert(offsetof(ElementTemperatureData, specificHeatCapacity) == 0xc, "ElementTemperatureData::specificHeatCapacity");
static_assert(offsetof(ElementTemperatureData, thermalConductivity) == 0x10, "ElementTemperatureData::thermalConductivity");
static_assert(offsetof(ElementTemperatureData, defaultMass) == 0x14, "ElementTemperatureData::defaultMass");
static_assert(offsetof(ElementTemperatureData, minMassForTemperature) == 0x18, "ElementTemperatureData::minMassForTemperature");
static_assert(offsetof(ElementTemperatureData, gasSurfaceAreaMultiplier) == 0x1c, "ElementTemperatureData::gasSurfaceAreaMultiplier");
static_assert(offsetof(ElementTemperatureData, liquidSurfaceAreaMultiplier) == 0x20, "ElementTemperatureData::liquidSurfaceAreaMultiplier");
static_assert(offsetof(ElementTemperatureData, solidSurfaceAreaMultiplier) == 0x24, "ElementTemperatureData::solidSurfaceAreaMultiplier");
static_assert(offsetof(ElementTemperatureData, lowTemp) == 0x28, "ElementTemperatureData::lowTemp");
static_assert(offsetof(ElementTemperatureData, highTemp) == 0x2c, "ElementTemperatureData::highTemp");
static_assert(offsetof(ElementTemperatureData, lowTempTransitionOreMassConversion) == 0x30, "ElementTemperatureData::lowTempTransitionOreMassConversion");
static_assert(offsetof(ElementTemperatureData, highTempTransitionOreMassConversion) == 0x34, "ElementTemperatureData::highTempTransitionOreMassConversion");

static_assert(sizeof(ElementPostProcessData) == 0x2c, "ElementPostProcessData size");
static_assert(offsetof(ElementPostProcessData, sublimateIndex) == 0x0, "ElementPostProcessData::sublimateIndex");
static_assert(offsetof(ElementPostProcessData, convertIndex) == 0x2, "ElementPostProcessData::convertIndex");
static_assert(offsetof(ElementPostProcessData, state) == 0x4, "ElementPostProcessData::state");
static_assert(offsetof(ElementPostProcessData, sublimateRate) == 0x8, "ElementPostProcessData::sublimateRate");
static_assert(offsetof(ElementPostProcessData, sublimateEfficiency) == 0xc, "ElementPostProcessData::sublimateEfficiency");
static_assert(offsetof(ElementPostProcessData, sublimateProbability) == 0x10, "ElementPostProcessData::sublimateProbability");
static_assert(offsetof(ElementPostProcessData, offGasProbability) == 0x14, "ElementPostProcessData::offGasProbability");
static_assert(offsetof(ElementPostProcessData, molarMass) == 0x18, "ElementPostProcessData::molarMass");
static_assert(offsetof(ElementPostProcessData, strength) == 0x1c, "ElementPostProcessData::strength");
static_assert(offsetof(ElementPostProcessData, maxMass) == 0x20, "ElementPostProcessData::maxMass");
static_assert(offsetof(ElementPostProcessData, minHorizontalFlow) == 0x24, "ElementPostProcessData::minHorizontalFlow");
static_assert(offsetof(ElementPostProcessData, sublimateFX) == 0x28, "ElementPostProcessData::sublimateFX");

static_assert(sizeof(ElementLiquidData) == 0x18, "ElementLiquidData size");
static_assert(offsetof(ElementLiquidData, state) == 0x0, "ElementLiquidData::state");
static_assert(offsetof(ElementLiquidData, flow) == 0x4, "ElementLiquidData::flow");
static_assert(offsetof(ElementLiquidData, viscosity) == 0x8, "ElementLiquidData::viscosity");
static_assert(offsetof(ElementLiquidData, minHorizontalFlow) == 0xc, "ElementLiquidData::minHorizontalFlow");
static_assert(offsetof(ElementLiquidData, minVerticalFlow) == 0x10, "ElementLiquidData::minVerticalFlow");
static_assert(offsetof(ElementLiquidData, maxMass) == 0x14, "ElementLiquidData::maxMass");

static_assert(sizeof(ElementPressureData) == 0x8, "ElementPressureData size");
static_assert(offsetof(ElementPressureData, state) == 0x0, "ElementPressureData::state");
static_assert(offsetof(ElementPressureData, flow) == 0x4, "ElementPressureData::flow");

static_assert(sizeof(ElementPropertyTextureData) == 0x24, "ElementPropertyTextureData size");
static_assert(offsetof(ElementPropertyTextureData, colour) == 0x0, "ElementPropertyTextureData::colour");
static_assert(offsetof(ElementPropertyTextureData, gradientColours) == 0x4, "ElementPropertyTextureData::gradientColours");
static_assert(offsetof(ElementPropertyTextureData, materialProperties) == 0x1c, "ElementPropertyTextureData::materialProperties");
static_assert(offsetof(ElementPropertyTextureData, state) == 0x20, "ElementPropertyTextureData::state");
static_assert(offsetof(ElementPropertyTextureData, numberOfGradientColors) == 0x21, "ElementPropertyTextureData::numberOfGradientColors");

static_assert(sizeof(ElementLightAbsorptionData) == 0x8, "ElementLightAbsorptionData size");
static_assert(offsetof(ElementLightAbsorptionData, lightAbsorptionFactor) == 0x0, "ElementLightAbsorptionData::lightAbsorptionFactor");
static_assert(offsetof(ElementLightAbsorptionData, inverseDefaultMass) == 0x4, "ElementLightAbsorptionData::inverseDefaultMass");

static_assert(sizeof(ElementRadiationData) == 0x8, "ElementRadiationData size");
static_assert(offsetof(ElementRadiationData, radiationAbsorptionFactor) == 0x0, "ElementRadiationData::radiationAbsorptionFactor");
static_assert(offsetof(ElementRadiationData, radiationPer1000Mass) == 0x4, "ElementRadiationData::radiationPer1000Mass");

static_assert(sizeof(BackwallSOA) == 0x4c, "BackwallSOA size");
static_assert(offsetof(BackwallSOA, noneElementIdx) == 0x0, "BackwallSOA::noneElementIdx");
static_assert(offsetof(BackwallSOA, elementIdx) == 0x4, "BackwallSOA::elementIdx");
static_assert(offsetof(BackwallSOA, mass) == 0x1c, "BackwallSOA::mass");
static_assert(offsetof(BackwallSOA, temperature) == 0x34, "BackwallSOA::temperature");

static_assert(sizeof(WorldRect) == 0x10, "WorldRect size");
static_assert(offsetof(WorldRect, x) == 0x0, "WorldRect::x");
static_assert(offsetof(WorldRect, y) == 0x4, "WorldRect::y");
static_assert(offsetof(WorldRect, width) == 0x8, "WorldRect::width");
static_assert(offsetof(WorldRect, height) == 0xc, "WorldRect::height");

static_assert(sizeof(ActiveRegion) == 0x18, "ActiveRegion size");
static_assert(offsetof(ActiveRegion, minX) == 0x0, "ActiveRegion::minX");
static_assert(offsetof(ActiveRegion, minY) == 0x4, "ActiveRegion::minY");
static_assert(offsetof(ActiveRegion, maxX) == 0x8, "ActiveRegion::maxX");
static_assert(offsetof(ActiveRegion, maxY) == 0xc, "ActiveRegion::maxY");
static_assert(offsetof(ActiveRegion, sunlightIntensity) == 0x10, "ActiveRegion::sunlightIntensity");
static_assert(offsetof(ActiveRegion, cosmicRadiationIntensity) == 0x14, "ActiveRegion::cosmicRadiationIntensity");

static_assert(sizeof(Region) == 0x10, "Region size");
static_assert(offsetof(Region, minX) == 0x0, "Region::minX");
static_assert(offsetof(Region, minY) == 0x4, "Region::minY");
static_assert(offsetof(Region, maxX) == 0x8, "Region::maxX");
static_assert(offsetof(Region, maxY) == 0xc, "Region::maxY");

static_assert(sizeof(SimUpdateTemperatureTask) == 0x28, "SimUpdateTemperatureTask size");
static_assert(offsetof(SimUpdateTemperatureTask, vtable) == 0x0, "SimUpdateTemperatureTask::vtable");
static_assert(offsetof(SimUpdateTemperatureTask, sim) == 0x8, "SimUpdateTemperatureTask::sim");
static_assert(offsetof(SimUpdateTemperatureTask, events) == 0x10, "SimUpdateTemperatureTask::events");
static_assert(offsetof(SimUpdateTemperatureTask, region) == 0x18, "SimUpdateTemperatureTask::region");

static_assert(sizeof(UpdateCellSOATask) == 0x30, "UpdateCellSOATask size");
static_assert(offsetof(UpdateCellSOATask, vtable) == 0x0, "UpdateCellSOATask::vtable");
static_assert(offsetof(UpdateCellSOATask, src) == 0x8, "UpdateCellSOATask::src");
static_assert(offsetof(UpdateCellSOATask, dst) == 0x10, "UpdateCellSOATask::dst");
static_assert(offsetof(UpdateCellSOATask, numRows) == 0x18, "UpdateCellSOATask::numRows");
static_assert(offsetof(UpdateCellSOATask, firstRow) == 0x1c, "UpdateCellSOATask::firstRow");
static_assert(offsetof(UpdateCellSOATask, srcStride) == 0x20, "UpdateCellSOATask::srcStride");
static_assert(offsetof(UpdateCellSOATask, dstFirstRow) == 0x24, "UpdateCellSOATask::dstFirstRow");
static_assert(offsetof(UpdateCellSOATask, rowLength) == 0x28, "UpdateCellSOATask::rowLength");

static_assert(sizeof(UpdateBackwallTask) == 0x30, "UpdateBackwallTask size");
static_assert(offsetof(UpdateBackwallTask, vtable) == 0x0, "UpdateBackwallTask::vtable");
static_assert(offsetof(UpdateBackwallTask, src) == 0x8, "UpdateBackwallTask::src");
static_assert(offsetof(UpdateBackwallTask, dst) == 0x10, "UpdateBackwallTask::dst");
static_assert(offsetof(UpdateBackwallTask, numRows) == 0x18, "UpdateBackwallTask::numRows");
static_assert(offsetof(UpdateBackwallTask, firstRow) == 0x1c, "UpdateBackwallTask::firstRow");
static_assert(offsetof(UpdateBackwallTask, srcStride) == 0x20, "UpdateBackwallTask::srcStride");
static_assert(offsetof(UpdateBackwallTask, dstFirstRow) == 0x24, "UpdateBackwallTask::dstFirstRow");
static_assert(offsetof(UpdateBackwallTask, rowLength) == 0x28, "UpdateBackwallTask::rowLength");

static_assert(sizeof(BuildingHeatExchangeData) == 0x2c, "BuildingHeatExchangeData size");
static_assert(offsetof(BuildingHeatExchangeData, temperature) == 0x0, "BuildingHeatExchangeData::temperature");
static_assert(offsetof(BuildingHeatExchangeData, overheatTemperature) == 0x4, "BuildingHeatExchangeData::overheatTemperature");
static_assert(offsetof(BuildingHeatExchangeData, operatingKilowatts) == 0x8, "BuildingHeatExchangeData::operatingKilowatts");
static_assert(offsetof(BuildingHeatExchangeData, heatCapacity) == 0xc, "BuildingHeatExchangeData::heatCapacity");
static_assert(offsetof(BuildingHeatExchangeData, heatCapacityPerCell) == 0x10, "BuildingHeatExchangeData::heatCapacityPerCell");
static_assert(offsetof(BuildingHeatExchangeData, conductivity) == 0x14, "BuildingHeatExchangeData::conductivity");
static_assert(offsetof(BuildingHeatExchangeData, meltTemperature) == 0x18, "BuildingHeatExchangeData::meltTemperature");
static_assert(offsetof(BuildingHeatExchangeData, area) == 0x1c, "BuildingHeatExchangeData::area");

static_assert(sizeof(BuildingTemperatureInfo) == 0x8, "BuildingTemperatureInfo size");
static_assert(offsetof(BuildingTemperatureInfo, handle) == 0x0, "BuildingTemperatureInfo::handle");
static_assert(offsetof(BuildingTemperatureInfo, temperature) == 0x4, "BuildingTemperatureInfo::temperature");

static_assert(sizeof(BuildingHeatExchange) == 0xa0, "BuildingHeatExchange size");
static_assert(offsetof(BuildingHeatExchange, vtable) == 0x0, "BuildingHeatExchange::vtable");
static_assert(offsetof(BuildingHeatExchange, data) == 0x8, "BuildingHeatExchange::data");
static_assert(offsetof(BuildingHeatExchange, denseToHandle) == 0x20, "BuildingHeatExchange::denseToHandle");
static_assert(offsetof(BuildingHeatExchange, freeHandles) == 0x38, "BuildingHeatExchange::freeHandles");
static_assert(offsetof(BuildingHeatExchange, handleToDense) == 0x50, "BuildingHeatExchange::handleToDense");
static_assert(offsetof(BuildingHeatExchange, versions) == 0x68, "BuildingHeatExchange::versions");
static_assert(offsetof(BuildingHeatExchange, componentVtable) == 0x80, "BuildingHeatExchange::componentVtable");
static_assert(offsetof(BuildingHeatExchange, temperatures) == 0x88, "BuildingHeatExchange::temperatures");

static_assert(sizeof(BuildingContact) == 0x8, "BuildingContact size");
static_assert(offsetof(BuildingContact, handle) == 0x0, "BuildingContact::handle");
static_assert(offsetof(BuildingContact, cellsInContact) == 0x4, "BuildingContact::cellsInContact");

static_assert(sizeof(BuildingToBuildingHeatExchangeData) == 0x20, "BuildingToBuildingHeatExchangeData size");
static_assert(offsetof(BuildingToBuildingHeatExchangeData, handle) == 0x0, "BuildingToBuildingHeatExchangeData::handle");
static_assert(offsetof(BuildingToBuildingHeatExchangeData, contacts) == 0x8, "BuildingToBuildingHeatExchangeData::contacts");

static_assert(sizeof(BuildingToBuildingHeatExchange) == 0x88, "BuildingToBuildingHeatExchange size");
static_assert(offsetof(BuildingToBuildingHeatExchange, vtable) == 0x0, "BuildingToBuildingHeatExchange::vtable");
static_assert(offsetof(BuildingToBuildingHeatExchange, data) == 0x8, "BuildingToBuildingHeatExchange::data");
static_assert(offsetof(BuildingToBuildingHeatExchange, denseToHandle) == 0x20, "BuildingToBuildingHeatExchange::denseToHandle");
static_assert(offsetof(BuildingToBuildingHeatExchange, freeHandles) == 0x38, "BuildingToBuildingHeatExchange::freeHandles");
static_assert(offsetof(BuildingToBuildingHeatExchange, handleToDense) == 0x50, "BuildingToBuildingHeatExchange::handleToDense");
static_assert(offsetof(BuildingToBuildingHeatExchange, versions) == 0x68, "BuildingToBuildingHeatExchange::versions");
static_assert(offsetof(BuildingToBuildingHeatExchange, componentVtable) == 0x80, "BuildingToBuildingHeatExchange::componentVtable");

static_assert(sizeof(ConduitTemperatureData) == 0x24, "ConduitTemperatureData size");
static_assert(offsetof(ConduitTemperatureData, temperature) == 0x0, "ConduitTemperatureData::temperature");
static_assert(offsetof(ConduitTemperatureData, contentsConductivity) == 0x4, "ConduitTemperatureData::contentsConductivity");
static_assert(offsetof(ConduitTemperatureData, contentsHeatCapacity) == 0x8, "ConduitTemperatureData::contentsHeatCapacity");
static_assert(offsetof(ConduitTemperatureData, structureHandle) == 0xc, "ConduitTemperatureData::structureHandle");
static_assert(offsetof(ConduitTemperatureData, conduitHeatCapacity) == 0x10, "ConduitTemperatureData::conduitHeatCapacity");
static_assert(offsetof(ConduitTemperatureData, conduitConductivity) == 0x14, "ConduitTemperatureData::conduitConductivity");
static_assert(offsetof(ConduitTemperatureData, insulated) == 0x18, "ConduitTemperatureData::insulated");
static_assert(offsetof(ConduitTemperatureData, lowTemp) == 0x1c, "ConduitTemperatureData::lowTemp");
static_assert(offsetof(ConduitTemperatureData, highTemp) == 0x20, "ConduitTemperatureData::highTemp");

static_assert(sizeof(ConduitTemperatureUpdateData) == 0x24, "ConduitTemperatureUpdateData size");
static_assert(offsetof(ConduitTemperatureUpdateData, numEntries) == 0x0, "ConduitTemperatureUpdateData::numEntries");
static_assert(offsetof(ConduitTemperatureUpdateData, temperatures) == 0x4, "ConduitTemperatureUpdateData::temperatures");
static_assert(offsetof(ConduitTemperatureUpdateData, numFrozenHandles) == 0xc, "ConduitTemperatureUpdateData::numFrozenHandles");
static_assert(offsetof(ConduitTemperatureUpdateData, frozenHandles) == 0x10, "ConduitTemperatureUpdateData::frozenHandles");
static_assert(offsetof(ConduitTemperatureUpdateData, numMeltedHandles) == 0x18, "ConduitTemperatureUpdateData::numMeltedHandles");
static_assert(offsetof(ConduitTemperatureUpdateData, meltedHandles) == 0x1c, "ConduitTemperatureUpdateData::meltedHandles");

static_assert(sizeof(ConduitTemperatureManager) == 0x148, "ConduitTemperatureManager size");
static_assert(offsetof(ConduitTemperatureManager, vtable) == 0x0, "ConduitTemperatureManager::vtable");
static_assert(offsetof(ConduitTemperatureManager, data) == 0x8, "ConduitTemperatureManager::data");
static_assert(offsetof(ConduitTemperatureManager, denseToHandle) == 0x20, "ConduitTemperatureManager::denseToHandle");
static_assert(offsetof(ConduitTemperatureManager, freeHandles) == 0x38, "ConduitTemperatureManager::freeHandles");
static_assert(offsetof(ConduitTemperatureManager, handleToDense) == 0x50, "ConduitTemperatureManager::handleToDense");
static_assert(offsetof(ConduitTemperatureManager, versions) == 0x68, "ConduitTemperatureManager::versions");
static_assert(offsetof(ConduitTemperatureManager, melted) == 0x80, "ConduitTemperatureManager::melted");
static_assert(offsetof(ConduitTemperatureManager, frozen) == 0x98, "ConduitTemperatureManager::frozen");
static_assert(offsetof(ConduitTemperatureManager, temperatures) == 0xb0, "ConduitTemperatureManager::temperatures");
static_assert(offsetof(ConduitTemperatureManager, result) == 0xc8, "ConduitTemperatureManager::result");
static_assert(offsetof(ConduitTemperatureManager, mutex) == 0x120, "ConduitTemperatureManager::mutex");

static_assert(sizeof(FloodFillInfo) == 0x6, "FloodFillInfo size");
static_assert(offsetof(FloodFillInfo, x) == 0x0, "FloodFillInfo::x");
static_assert(offsetof(FloodFillInfo, y) == 0x2, "FloodFillInfo::y");
static_assert(offsetof(FloodFillInfo, distance) == 0x4, "FloodFillInfo::distance");

static_assert(sizeof(ConsumedMassInfo) == 0x14, "ConsumedMassInfo size");
static_assert(offsetof(ConsumedMassInfo, simHandle) == 0x0, "ConsumedMassInfo::simHandle");
static_assert(offsetof(ConsumedMassInfo, removedElemIdx) == 0x4, "ConsumedMassInfo::removedElemIdx");
static_assert(offsetof(ConsumedMassInfo, diseaseIdx) == 0x6, "ConsumedMassInfo::diseaseIdx");
static_assert(offsetof(ConsumedMassInfo, mass) == 0x8, "ConsumedMassInfo::mass");
static_assert(offsetof(ConsumedMassInfo, temperature) == 0xc, "ConsumedMassInfo::temperature");
static_assert(offsetof(ConsumedMassInfo, diseaseCount) == 0x10, "ConsumedMassInfo::diseaseCount");

static_assert(sizeof(ElementConsumerData) == 0x10, "ElementConsumerData size");
static_assert(offsetof(ElementConsumerData, consumptionRate) == 0x0, "ElementConsumerData::consumptionRate");
static_assert(offsetof(ElementConsumerData, radius) == 0x4, "ElementConsumerData::radius");
static_assert(offsetof(ElementConsumerData, configuration) == 0x5, "ElementConsumerData::configuration");
static_assert(offsetof(ElementConsumerData, elementIdx) == 0x6, "ElementConsumerData::elementIdx");
static_assert(offsetof(ElementConsumerData, roundRobin) == 0x8, "ElementConsumerData::roundRobin");
static_assert(offsetof(ElementConsumerData, x) == 0xa, "ElementConsumerData::x");
static_assert(offsetof(ElementConsumerData, y) == 0xc, "ElementConsumerData::y");

static_assert(sizeof(ElementConsumer) == 0x120, "ElementConsumer size");
static_assert(offsetof(ElementConsumer, vtable) == 0x0, "ElementConsumer::vtable");
static_assert(offsetof(ElementConsumer, data) == 0x8, "ElementConsumer::data");
static_assert(offsetof(ElementConsumer, denseToHandle) == 0x20, "ElementConsumer::denseToHandle");
static_assert(offsetof(ElementConsumer, freeHandles) == 0x38, "ElementConsumer::freeHandles");
static_assert(offsetof(ElementConsumer, handleToDense) == 0x50, "ElementConsumer::handleToDense");
static_assert(offsetof(ElementConsumer, versions) == 0x68, "ElementConsumer::versions");
static_assert(offsetof(ElementConsumer, componentVtable) == 0x80, "ElementConsumer::componentVtable");
static_assert(offsetof(ElementConsumer, visited) == 0x88, "ElementConsumer::visited");
static_assert(offsetof(ElementConsumer, queue) == 0xa0, "ElementConsumer::queue");
static_assert(offsetof(ElementConsumer, reachable) == 0xf0, "ElementConsumer::reachable");
static_assert(offsetof(ElementConsumer, consumed) == 0x108, "ElementConsumer::consumed");

static_assert(sizeof(ElementEmitterData) == 0x2c, "ElementEmitterData size");
static_assert(offsetof(ElementEmitterData, timer) == 0x0, "ElementEmitterData::timer");
static_assert(offsetof(ElementEmitterData, emitInterval) == 0x4, "ElementEmitterData::emitInterval");
static_assert(offsetof(ElementEmitterData, emitMass) == 0x8, "ElementEmitterData::emitMass");
static_assert(offsetof(ElementEmitterData, emitTemperature) == 0xc, "ElementEmitterData::emitTemperature");
static_assert(offsetof(ElementEmitterData, maxPressure) == 0x10, "ElementEmitterData::maxPressure");
static_assert(offsetof(ElementEmitterData, diseaseCount) == 0x14, "ElementEmitterData::diseaseCount");
static_assert(offsetof(ElementEmitterData, x) == 0x18, "ElementEmitterData::x");
static_assert(offsetof(ElementEmitterData, y) == 0x1a, "ElementEmitterData::y");
static_assert(offsetof(ElementEmitterData, elementIdx) == 0x1c, "ElementEmitterData::elementIdx");
static_assert(offsetof(ElementEmitterData, range) == 0x1e, "ElementEmitterData::range");
static_assert(offsetof(ElementEmitterData, startIndex) == 0x1f, "ElementEmitterData::startIndex");
static_assert(offsetof(ElementEmitterData, blocked) == 0x20, "ElementEmitterData::blocked");
static_assert(offsetof(ElementEmitterData, diseaseIdx) == 0x21, "ElementEmitterData::diseaseIdx");
static_assert(offsetof(ElementEmitterData, onBlockedCallback) == 0x24, "ElementEmitterData::onBlockedCallback");
static_assert(offsetof(ElementEmitterData, onUnblockedCallback) == 0x28, "ElementEmitterData::onUnblockedCallback");

static_assert(sizeof(EmittedMassInfo) == 0x10, "EmittedMassInfo size");
static_assert(offsetof(EmittedMassInfo, elemIdx) == 0x0, "EmittedMassInfo::elemIdx");
static_assert(offsetof(EmittedMassInfo, diseaseIdx) == 0x2, "EmittedMassInfo::diseaseIdx");
static_assert(offsetof(EmittedMassInfo, pad0) == 0x3, "EmittedMassInfo::pad0");
static_assert(offsetof(EmittedMassInfo, mass) == 0x4, "EmittedMassInfo::mass");
static_assert(offsetof(EmittedMassInfo, temperature) == 0x8, "EmittedMassInfo::temperature");
static_assert(offsetof(EmittedMassInfo, diseaseCount) == 0xc, "EmittedMassInfo::diseaseCount");

static_assert(sizeof(ElementEmitter) == 0x120, "ElementEmitter size");
static_assert(offsetof(ElementEmitter, vtable) == 0x0, "ElementEmitter::vtable");
static_assert(offsetof(ElementEmitter, data) == 0x8, "ElementEmitter::data");
static_assert(offsetof(ElementEmitter, denseToHandle) == 0x20, "ElementEmitter::denseToHandle");
static_assert(offsetof(ElementEmitter, freeHandles) == 0x38, "ElementEmitter::freeHandles");
static_assert(offsetof(ElementEmitter, handleToDense) == 0x50, "ElementEmitter::handleToDense");
static_assert(offsetof(ElementEmitter, versions) == 0x68, "ElementEmitter::versions");
static_assert(offsetof(ElementEmitter, componentVtable) == 0x80, "ElementEmitter::componentVtable");
static_assert(offsetof(ElementEmitter, emitted) == 0x88, "ElementEmitter::emitted");
static_assert(offsetof(ElementEmitter, visited) == 0xa0, "ElementEmitter::visited");
static_assert(offsetof(ElementEmitter, reachable) == 0xb8, "ElementEmitter::reachable");
static_assert(offsetof(ElementEmitter, queue) == 0xd0, "ElementEmitter::queue");

static_assert(sizeof(DiseaseEmitterData) == 0x14, "DiseaseEmitterData size");
static_assert(offsetof(DiseaseEmitterData, cell) == 0x0, "DiseaseEmitterData::cell");
static_assert(offsetof(DiseaseEmitterData, diseaseIdx) == 0x4, "DiseaseEmitterData::diseaseIdx");
static_assert(offsetof(DiseaseEmitterData, range) == 0x5, "DiseaseEmitterData::range");
static_assert(offsetof(DiseaseEmitterData, count) == 0x8, "DiseaseEmitterData::count");
static_assert(offsetof(DiseaseEmitterData, emitInterval) == 0xc, "DiseaseEmitterData::emitInterval");
static_assert(offsetof(DiseaseEmitterData, timer) == 0x10, "DiseaseEmitterData::timer");

static_assert(sizeof(DiseaseEmittedInfo) == 0x8, "DiseaseEmittedInfo size");
static_assert(offsetof(DiseaseEmittedInfo, diseaseIdx) == 0x0, "DiseaseEmittedInfo::diseaseIdx");
static_assert(offsetof(DiseaseEmittedInfo, count) == 0x4, "DiseaseEmittedInfo::count");

static_assert(sizeof(DiseaseEmitter) == 0x120, "DiseaseEmitter size");
static_assert(offsetof(DiseaseEmitter, vtable) == 0x0, "DiseaseEmitter::vtable");
static_assert(offsetof(DiseaseEmitter, data) == 0x8, "DiseaseEmitter::data");
static_assert(offsetof(DiseaseEmitter, denseToHandle) == 0x20, "DiseaseEmitter::denseToHandle");
static_assert(offsetof(DiseaseEmitter, freeHandles) == 0x38, "DiseaseEmitter::freeHandles");
static_assert(offsetof(DiseaseEmitter, handleToDense) == 0x50, "DiseaseEmitter::handleToDense");
static_assert(offsetof(DiseaseEmitter, versions) == 0x68, "DiseaseEmitter::versions");
static_assert(offsetof(DiseaseEmitter, componentVtable) == 0x80, "DiseaseEmitter::componentVtable");
static_assert(offsetof(DiseaseEmitter, emitted) == 0x88, "DiseaseEmitter::emitted");
static_assert(offsetof(DiseaseEmitter, queue) == 0xa0, "DiseaseEmitter::queue");
static_assert(offsetof(DiseaseEmitter, visited) == 0xf0, "DiseaseEmitter::visited");
static_assert(offsetof(DiseaseEmitter, reachable) == 0x108, "DiseaseEmitter::reachable");

static_assert(sizeof(DiseaseConsumerData) == 0xc, "DiseaseConsumerData size");

static_assert(sizeof(DiseaseConsumer) == 0xa0, "DiseaseConsumer size");
static_assert(offsetof(DiseaseConsumer, vtable) == 0x0, "DiseaseConsumer::vtable");
static_assert(offsetof(DiseaseConsumer, data) == 0x8, "DiseaseConsumer::data");
static_assert(offsetof(DiseaseConsumer, denseToHandle) == 0x20, "DiseaseConsumer::denseToHandle");
static_assert(offsetof(DiseaseConsumer, freeHandles) == 0x38, "DiseaseConsumer::freeHandles");
static_assert(offsetof(DiseaseConsumer, handleToDense) == 0x50, "DiseaseConsumer::handleToDense");
static_assert(offsetof(DiseaseConsumer, versions) == 0x68, "DiseaseConsumer::versions");
static_assert(offsetof(DiseaseConsumer, componentVtable) == 0x80, "DiseaseConsumer::componentVtable");
static_assert(offsetof(DiseaseConsumer, consumed) == 0x88, "DiseaseConsumer::consumed");

static_assert(sizeof(ElementChunkData) == 0x2c, "ElementChunkData size");
static_assert(offsetof(ElementChunkData, cell) == 0x0, "ElementChunkData::cell");
static_assert(offsetof(ElementChunkData, temperature) == 0x4, "ElementChunkData::temperature");
static_assert(offsetof(ElementChunkData, heatCapacity) == 0x8, "ElementChunkData::heatCapacity");
static_assert(offsetof(ElementChunkData, thermalConductivity) == 0xc, "ElementChunkData::thermalConductivity");
static_assert(offsetof(ElementChunkData, areaOverThickness) == 0x10, "ElementChunkData::areaOverThickness");
static_assert(offsetof(ElementChunkData, groundTransferScale) == 0x14, "ElementChunkData::groundTransferScale");
static_assert(offsetof(ElementChunkData, highTemp) == 0x18, "ElementChunkData::highTemp");
static_assert(offsetof(ElementChunkData, lowTemp) == 0x1c, "ElementChunkData::lowTemp");
static_assert(offsetof(ElementChunkData, adjusterTemperature) == 0x20, "ElementChunkData::adjusterTemperature");
static_assert(offsetof(ElementChunkData, adjusterHeatCapacity) == 0x24, "ElementChunkData::adjusterHeatCapacity");
static_assert(offsetof(ElementChunkData, adjusterRate) == 0x28, "ElementChunkData::adjusterRate");

static_assert(sizeof(ElementChunkInfo) == 0x8, "ElementChunkInfo size");
static_assert(offsetof(ElementChunkInfo, temperature) == 0x0, "ElementChunkInfo::temperature");
static_assert(offsetof(ElementChunkInfo, deltaKJ) == 0x4, "ElementChunkInfo::deltaKJ");

static_assert(sizeof(ElementChunk) == 0xa0, "ElementChunk size");
static_assert(offsetof(ElementChunk, vtable) == 0x0, "ElementChunk::vtable");
static_assert(offsetof(ElementChunk, data) == 0x8, "ElementChunk::data");
static_assert(offsetof(ElementChunk, denseToHandle) == 0x20, "ElementChunk::denseToHandle");
static_assert(offsetof(ElementChunk, freeHandles) == 0x38, "ElementChunk::freeHandles");
static_assert(offsetof(ElementChunk, handleToDense) == 0x50, "ElementChunk::handleToDense");
static_assert(offsetof(ElementChunk, versions) == 0x68, "ElementChunk::versions");
static_assert(offsetof(ElementChunk, componentVtable) == 0x80, "ElementChunk::componentVtable");
static_assert(offsetof(ElementChunk, info) == 0x88, "ElementChunk::info");

static_assert(sizeof(RadiationEmitterData) == 0x2c, "RadiationEmitterData size");
static_assert(offsetof(RadiationEmitterData, cell) == 0x0, "RadiationEmitterData::cell");
static_assert(offsetof(RadiationEmitterData, radiusX) == 0x4, "RadiationEmitterData::radiusX");
static_assert(offsetof(RadiationEmitterData, radiusY) == 0x6, "RadiationEmitterData::radiusY");
static_assert(offsetof(RadiationEmitterData, intensity) == 0x8, "RadiationEmitterData::intensity");
static_assert(offsetof(RadiationEmitterData, pulseInterval) == 0xc, "RadiationEmitterData::pulseInterval");
static_assert(offsetof(RadiationEmitterData, pulseDuration) == 0x10, "RadiationEmitterData::pulseDuration");
static_assert(offsetof(RadiationEmitterData, direction) == 0x14, "RadiationEmitterData::direction");
static_assert(offsetof(RadiationEmitterData, angle) == 0x18, "RadiationEmitterData::angle");
static_assert(offsetof(RadiationEmitterData, timer) == 0x1c, "RadiationEmitterData::timer");
static_assert(offsetof(RadiationEmitterData, pulseTimer) == 0x20, "RadiationEmitterData::pulseTimer");
static_assert(offsetof(RadiationEmitterData, type) == 0x24, "RadiationEmitterData::type");
static_assert(offsetof(RadiationEmitterData, step) == 0x28, "RadiationEmitterData::step");

static_assert(sizeof(RadiationEmitter) == 0x88, "RadiationEmitter size");
static_assert(offsetof(RadiationEmitter, vtable) == 0x0, "RadiationEmitter::vtable");
static_assert(offsetof(RadiationEmitter, data) == 0x8, "RadiationEmitter::data");
static_assert(offsetof(RadiationEmitter, denseToHandle) == 0x20, "RadiationEmitter::denseToHandle");
static_assert(offsetof(RadiationEmitter, freeHandles) == 0x38, "RadiationEmitter::freeHandles");
static_assert(offsetof(RadiationEmitter, handleToDense) == 0x50, "RadiationEmitter::handleToDense");
static_assert(offsetof(RadiationEmitter, versions) == 0x68, "RadiationEmitter::versions");
static_assert(offsetof(RadiationEmitter, componentVtable) == 0x80, "RadiationEmitter::componentVtable");

static_assert(sizeof(GameData) == 0x318, "GameData size");
static_assert(offsetof(GameData, width) == 0x0, "GameData::width");
static_assert(offsetof(GameData, height) == 0x4, "GameData::height");
static_assert(offsetof(GameData, numFramesProcessed) == 0x8, "GameData::numFramesProcessed");
static_assert(offsetof(GameData, substanceChanges) == 0x10, "GameData::substanceChanges");
static_assert(offsetof(GameData, callbacks) == 0x28, "GameData::callbacks");
static_assert(offsetof(GameData, spawnFallingLiquid) == 0x40, "GameData::spawnFallingLiquid");
static_assert(offsetof(GameData, spawnOre) == 0x58, "GameData::spawnOre");
static_assert(offsetof(GameData, digInfo) == 0x70, "GameData::digInfo");
static_assert(offsetof(GameData, unstableCells) == 0x88, "GameData::unstableCells");
static_assert(offsetof(GameData, worldDamage) == 0xa0, "GameData::worldDamage");
static_assert(offsetof(GameData, buildingTemperatures) == 0xb8, "GameData::buildingTemperatures");
static_assert(offsetof(GameData, massConsumed) == 0xd0, "GameData::massConsumed");
static_assert(offsetof(GameData, massEmitted) == 0xe8, "GameData::massEmitted");
static_assert(offsetof(GameData, diseaseConsumed) == 0x100, "GameData::diseaseConsumed");
static_assert(offsetof(GameData, radiationConsumed) == 0x118, "GameData::radiationConsumed");
static_assert(offsetof(GameData, spawnFX) == 0x130, "GameData::spawnFX");
static_assert(offsetof(GameData, solidInfo) == 0x148, "GameData::solidInfo");
static_assert(offsetof(GameData, liquidChanges) == 0x160, "GameData::liquidChanges");
static_assert(offsetof(GameData, solidSubstanceChanges) == 0x178, "GameData::solidSubstanceChanges");
static_assert(offsetof(GameData, removedMass) == 0x190, "GameData::removedMass");
static_assert(offsetof(GameData, emittedMass) == 0x1a8, "GameData::emittedMass");
static_assert(offsetof(GameData, elementChunkInfos) == 0x1c0, "GameData::elementChunkInfos");
static_assert(offsetof(GameData, diseaseEmitted) == 0x1d8, "GameData::diseaseEmitted");
static_assert(offsetof(GameData, diseaseConsumedInfos) == 0x1f0, "GameData::diseaseConsumedInfos");
static_assert(offsetof(GameData, elementChunkMelted) == 0x208, "GameData::elementChunkMelted");
static_assert(offsetof(GameData, buildingMelted) == 0x220, "GameData::buildingMelted");
static_assert(offsetof(GameData, buildingOverheat) == 0x238, "GameData::buildingOverheat");
static_assert(offsetof(GameData, buildingNoLongerOverheated) == 0x250, "GameData::buildingNoLongerOverheated");
static_assert(offsetof(GameData, componentStateChanged) == 0x268, "GameData::componentStateChanged");
static_assert(offsetof(GameData, cellMelted) == 0x280, "GameData::cellMelted");
static_assert(offsetof(GameData, backwallElementChanged) == 0x298, "GameData::backwallElementChanged");
static_assert(offsetof(GameData, backwallShouldTransition) == 0x2b0, "GameData::backwallShouldTransition");
static_assert(offsetof(GameData, cells) == 0x2c8, "GameData::cells");
static_assert(offsetof(GameData, backwall) == 0x2d0, "GameData::backwall");
static_assert(offsetof(GameData, flow) == 0x2d8, "GameData::flow");
static_assert(offsetof(GameData, accumulatedFlow) == 0x2e0, "GameData::accumulatedFlow");
static_assert(offsetof(GameData, visibleGrid) == 0x2e8, "GameData::visibleGrid");
static_assert(offsetof(GameData, propertyTextureFlow) == 0x2f0, "GameData::propertyTextureFlow");
static_assert(offsetof(GameData, propertyTextureLiquid) == 0x2f8, "GameData::propertyTextureLiquid");
static_assert(offsetof(GameData, propertyTextureLiquidData) == 0x300, "GameData::propertyTextureLiquidData");
static_assert(offsetof(GameData, propertyTextureMaterialData) == 0x308, "GameData::propertyTextureMaterialData");
static_assert(offsetof(GameData, propertyTextureExposedToSunlight) == 0x310, "GameData::propertyTextureExposedToSunlight");

static_assert(sizeof(DigPoint) == 0xc, "DigPoint size");
static_assert(offsetof(DigPoint, cellIdx) == 0x0, "DigPoint::cellIdx");
static_assert(offsetof(DigPoint, callbackIdx) == 0x4, "DigPoint::callbackIdx");
static_assert(offsetof(DigPoint, skipEvent) == 0x8, "DigPoint::skipEvent");
static_assert(offsetof(DigPoint, backwall) == 0x9, "DigPoint::backwall");

static_assert(sizeof(CellModification) == 0x1c, "CellModification size");
static_assert(offsetof(CellModification, cellIdx) == 0x0, "CellModification::cellIdx");
static_assert(offsetof(CellModification, callbackIdx) == 0x4, "CellModification::callbackIdx");
static_assert(offsetof(CellModification, temperature) == 0x8, "CellModification::temperature");
static_assert(offsetof(CellModification, mass) == 0xc, "CellModification::mass");
static_assert(offsetof(CellModification, diseaseCount) == 0x10, "CellModification::diseaseCount");
static_assert(offsetof(CellModification, elementIdx) == 0x14, "CellModification::elementIdx");
static_assert(offsetof(CellModification, replaceType) == 0x16, "CellModification::replaceType");
static_assert(offsetof(CellModification, diseaseIdx) == 0x17, "CellModification::diseaseIdx");
static_assert(offsetof(CellModification, addSubType) == 0x18, "CellModification::addSubType");

static_assert(sizeof(CellEnergyModification) == 0x10, "CellEnergyModification size");
static_assert(offsetof(CellEnergyModification, cellIdx) == 0x0, "CellEnergyModification::cellIdx");
static_assert(offsetof(CellEnergyModification, kilojoules) == 0x4, "CellEnergyModification::kilojoules");
static_assert(offsetof(CellEnergyModification, maxTemperature) == 0x8, "CellEnergyModification::maxTemperature");
static_assert(offsetof(CellEnergyModification, id) == 0xc, "CellEnergyModification::id");

static_assert(sizeof(SetCellFloatValue) == 0x8, "SetCellFloatValue size");
static_assert(offsetof(SetCellFloatValue, cellIdx) == 0x0, "SetCellFloatValue::cellIdx");
static_assert(offsetof(SetCellFloatValue, value) == 0x4, "SetCellFloatValue::value");

static_assert(sizeof(CellPropertiesChange) == 0xc, "CellPropertiesChange size");
static_assert(offsetof(CellPropertiesChange, cellIdx) == 0x0, "CellPropertiesChange::cellIdx");
static_assert(offsetof(CellPropertiesChange, callbackIdx) == 0x4, "CellPropertiesChange::callbackIdx");
static_assert(offsetof(CellPropertiesChange, properties) == 0x8, "CellPropertiesChange::properties");
static_assert(offsetof(CellPropertiesChange, set) == 0x9, "CellPropertiesChange::set");
static_assert(offsetof(CellPropertiesChange, pad0) == 0xa, "CellPropertiesChange::pad0");
static_assert(offsetof(CellPropertiesChange, pad1) == 0xb, "CellPropertiesChange::pad1");

static_assert(sizeof(ConsumeDisease) == 0x10, "ConsumeDisease size");
static_assert(offsetof(ConsumeDisease, gameCell) == 0x0, "ConsumeDisease::gameCell");
static_assert(offsetof(ConsumeDisease, callbackIdx) == 0x4, "ConsumeDisease::callbackIdx");
static_assert(offsetof(ConsumeDisease, percentToConsume) == 0x8, "ConsumeDisease::percentToConsume");
static_assert(offsetof(ConsumeDisease, maxToConsume) == 0xc, "ConsumeDisease::maxToConsume");

static_assert(sizeof(CellDiseaseModification) == 0xc, "CellDiseaseModification size");
static_assert(offsetof(CellDiseaseModification, cellIdx) == 0x0, "CellDiseaseModification::cellIdx");
static_assert(offsetof(CellDiseaseModification, diseaseIdx) == 0x4, "CellDiseaseModification::diseaseIdx");
static_assert(offsetof(CellDiseaseModification, pad0) == 0x5, "CellDiseaseModification::pad0");
static_assert(offsetof(CellDiseaseModification, pad1) == 0x6, "CellDiseaseModification::pad1");
static_assert(offsetof(CellDiseaseModification, pad2) == 0x7, "CellDiseaseModification::pad2");
static_assert(offsetof(CellDiseaseModification, diseaseCount) == 0x8, "CellDiseaseModification::diseaseCount");

static_assert(sizeof(CellRadiationModification) == 0xc, "CellRadiationModification size");
static_assert(offsetof(CellRadiationModification, cellIdx) == 0x0, "CellRadiationModification::cellIdx");
static_assert(offsetof(CellRadiationModification, radiationDelta) == 0x4, "CellRadiationModification::radiationDelta");
static_assert(offsetof(CellRadiationModification, callbackIdx) == 0x8, "CellRadiationModification::callbackIdx");

static_assert(sizeof(RadiationParamsModification) == 0x8, "RadiationParamsModification size");
static_assert(offsetof(RadiationParamsModification, RadiationParamsType) == 0x0, "RadiationParamsModification::RadiationParamsType");
static_assert(offsetof(RadiationParamsModification, value) == 0x4, "RadiationParamsModification::value");

static_assert(sizeof(CellWorldZoneModification) == 0x8, "CellWorldZoneModification size");
static_assert(offsetof(CellWorldZoneModification, cell) == 0x0, "CellWorldZoneModification::cell");
static_assert(offsetof(CellWorldZoneModification, zoneID) == 0x4, "CellWorldZoneModification::zoneID");

static_assert(sizeof(MassConsumption) == 0x10, "MassConsumption size");
static_assert(offsetof(MassConsumption, cellIdx) == 0x0, "MassConsumption::cellIdx");
static_assert(offsetof(MassConsumption, callbackIdx) == 0x4, "MassConsumption::callbackIdx");
static_assert(offsetof(MassConsumption, mass) == 0x8, "MassConsumption::mass");
static_assert(offsetof(MassConsumption, elementIdx) == 0xc, "MassConsumption::elementIdx");
static_assert(offsetof(MassConsumption, radius) == 0xe, "MassConsumption::radius");
static_assert(offsetof(MassConsumption, height) == 0xf, "MassConsumption::height");

static_assert(sizeof(MassEmission) == 0x18, "MassEmission size");
static_assert(offsetof(MassEmission, cellIdx) == 0x0, "MassEmission::cellIdx");
static_assert(offsetof(MassEmission, callbackIdx) == 0x4, "MassEmission::callbackIdx");
static_assert(offsetof(MassEmission, mass) == 0x8, "MassEmission::mass");
static_assert(offsetof(MassEmission, temperature) == 0xc, "MassEmission::temperature");
static_assert(offsetof(MassEmission, diseaseCount) == 0x10, "MassEmission::diseaseCount");
static_assert(offsetof(MassEmission, elementIdx) == 0x14, "MassEmission::elementIdx");
static_assert(offsetof(MassEmission, diseaseIdx) == 0x16, "MassEmission::diseaseIdx");

static_assert(sizeof(AddBuildingHeatExchangeMessage) == 0x2c, "AddBuildingHeatExchangeMessage size");
static_assert(offsetof(AddBuildingHeatExchangeMessage, callbackIdx) == 0x0, "AddBuildingHeatExchangeMessage::callbackIdx");
static_assert(offsetof(AddBuildingHeatExchangeMessage, elemIdx) == 0x4, "AddBuildingHeatExchangeMessage::elemIdx");
static_assert(offsetof(AddBuildingHeatExchangeMessage, pad0) == 0x6, "AddBuildingHeatExchangeMessage::pad0");
static_assert(offsetof(AddBuildingHeatExchangeMessage, pad1) == 0x7, "AddBuildingHeatExchangeMessage::pad1");
static_assert(offsetof(AddBuildingHeatExchangeMessage, mass) == 0x8, "AddBuildingHeatExchangeMessage::mass");
static_assert(offsetof(AddBuildingHeatExchangeMessage, temperature) == 0xc, "AddBuildingHeatExchangeMessage::temperature");
static_assert(offsetof(AddBuildingHeatExchangeMessage, thermalConductivity) == 0x10, "AddBuildingHeatExchangeMessage::thermalConductivity");
static_assert(offsetof(AddBuildingHeatExchangeMessage, overheatTemperature) == 0x14, "AddBuildingHeatExchangeMessage::overheatTemperature");
static_assert(offsetof(AddBuildingHeatExchangeMessage, operatingKilowatts) == 0x18, "AddBuildingHeatExchangeMessage::operatingKilowatts");
static_assert(offsetof(AddBuildingHeatExchangeMessage, minX) == 0x1c, "AddBuildingHeatExchangeMessage::minX");
static_assert(offsetof(AddBuildingHeatExchangeMessage, minY) == 0x20, "AddBuildingHeatExchangeMessage::minY");
static_assert(offsetof(AddBuildingHeatExchangeMessage, maxX) == 0x24, "AddBuildingHeatExchangeMessage::maxX");
static_assert(offsetof(AddBuildingHeatExchangeMessage, maxY) == 0x28, "AddBuildingHeatExchangeMessage::maxY");

static_assert(sizeof(ModifyBuildingHeatExchangeMessage) == 0x2c, "ModifyBuildingHeatExchangeMessage size");
static_assert(offsetof(ModifyBuildingHeatExchangeMessage, callbackIdx) == 0x0, "ModifyBuildingHeatExchangeMessage::callbackIdx");
static_assert(offsetof(ModifyBuildingHeatExchangeMessage, elemIdx) == 0x4, "ModifyBuildingHeatExchangeMessage::elemIdx");
static_assert(offsetof(ModifyBuildingHeatExchangeMessage, pad0) == 0x6, "ModifyBuildingHeatExchangeMessage::pad0");
static_assert(offsetof(ModifyBuildingHeatExchangeMessage, pad1) == 0x7, "ModifyBuildingHeatExchangeMessage::pad1");
static_assert(offsetof(ModifyBuildingHeatExchangeMessage, mass) == 0x8, "ModifyBuildingHeatExchangeMessage::mass");
static_assert(offsetof(ModifyBuildingHeatExchangeMessage, temperature) == 0xc, "ModifyBuildingHeatExchangeMessage::temperature");
static_assert(offsetof(ModifyBuildingHeatExchangeMessage, thermalConductivity) == 0x10, "ModifyBuildingHeatExchangeMessage::thermalConductivity");
static_assert(offsetof(ModifyBuildingHeatExchangeMessage, overheatTemperature) == 0x14, "ModifyBuildingHeatExchangeMessage::overheatTemperature");
static_assert(offsetof(ModifyBuildingHeatExchangeMessage, operatingKilowatts) == 0x18, "ModifyBuildingHeatExchangeMessage::operatingKilowatts");
static_assert(offsetof(ModifyBuildingHeatExchangeMessage, minX) == 0x1c, "ModifyBuildingHeatExchangeMessage::minX");
static_assert(offsetof(ModifyBuildingHeatExchangeMessage, minY) == 0x20, "ModifyBuildingHeatExchangeMessage::minY");
static_assert(offsetof(ModifyBuildingHeatExchangeMessage, maxX) == 0x24, "ModifyBuildingHeatExchangeMessage::maxX");
static_assert(offsetof(ModifyBuildingHeatExchangeMessage, maxY) == 0x28, "ModifyBuildingHeatExchangeMessage::maxY");

static_assert(sizeof(RemoveBuildingHeatExchangeMessage) == 0x8, "RemoveBuildingHeatExchangeMessage size");
static_assert(offsetof(RemoveBuildingHeatExchangeMessage, handle) == 0x0, "RemoveBuildingHeatExchangeMessage::handle");
static_assert(offsetof(RemoveBuildingHeatExchangeMessage, callbackIdx) == 0x4, "RemoveBuildingHeatExchangeMessage::callbackIdx");

static_assert(sizeof(ModifyBuildingEnergyMessage) == 0x10, "ModifyBuildingEnergyMessage size");
static_assert(offsetof(ModifyBuildingEnergyMessage, handle) == 0x0, "ModifyBuildingEnergyMessage::handle");
static_assert(offsetof(ModifyBuildingEnergyMessage, deltaKJ) == 0x4, "ModifyBuildingEnergyMessage::deltaKJ");
static_assert(offsetof(ModifyBuildingEnergyMessage, minTemperature) == 0x8, "ModifyBuildingEnergyMessage::minTemperature");
static_assert(offsetof(ModifyBuildingEnergyMessage, maxTemperature) == 0xc, "ModifyBuildingEnergyMessage::maxTemperature");

static_assert(sizeof(RegisterBuildingToBuildingHeatExchangeMessage) == 0x8, "RegisterBuildingToBuildingHeatExchangeMessage size");
static_assert(offsetof(RegisterBuildingToBuildingHeatExchangeMessage, callbackIdx) == 0x0, "RegisterBuildingToBuildingHeatExchangeMessage::callbackIdx");
static_assert(offsetof(RegisterBuildingToBuildingHeatExchangeMessage, structureTemperatureHandler) == 0x4, "RegisterBuildingToBuildingHeatExchangeMessage::structureTemperatureHandler");

static_assert(sizeof(RemoveBuildingInContactFromBuildingToBuildingHeatExchangeMessage) == 0x8, "RemoveBuildingInContactFromBuildingToBuildingHeatExchangeMessage size");
static_assert(offsetof(RemoveBuildingInContactFromBuildingToBuildingHeatExchangeMessage, selfHandler) == 0x0, "RemoveBuildingInContactFromBuildingToBuildingHeatExchangeMessage::selfHandler");
static_assert(offsetof(RemoveBuildingInContactFromBuildingToBuildingHeatExchangeMessage, buildingNoLongerInContactHandler) == 0x4, "RemoveBuildingInContactFromBuildingToBuildingHeatExchangeMessage::buildingNoLongerInContactHandler");

static_assert(sizeof(RemoveBuildingToBuildingHeatExchangeMessage) == 0x8, "RemoveBuildingToBuildingHeatExchangeMessage size");
static_assert(offsetof(RemoveBuildingToBuildingHeatExchangeMessage, callbackIdx) == 0x0, "RemoveBuildingToBuildingHeatExchangeMessage::callbackIdx");
static_assert(offsetof(RemoveBuildingToBuildingHeatExchangeMessage, selfHandler) == 0x4, "RemoveBuildingToBuildingHeatExchangeMessage::selfHandler");

static_assert(sizeof(AddInContactBuildingToBuildingToBuildingHeatExchangeMessage) == 0xc, "AddInContactBuildingToBuildingToBuildingHeatExchangeMessage size");
static_assert(offsetof(AddInContactBuildingToBuildingToBuildingHeatExchangeMessage, selfHandler) == 0x0, "AddInContactBuildingToBuildingToBuildingHeatExchangeMessage::selfHandler");
static_assert(offsetof(AddInContactBuildingToBuildingToBuildingHeatExchangeMessage, buildingInContactHandle) == 0x4, "AddInContactBuildingToBuildingToBuildingHeatExchangeMessage::buildingInContactHandle");
static_assert(offsetof(AddInContactBuildingToBuildingToBuildingHeatExchangeMessage, cellsInContact) == 0x8, "AddInContactBuildingToBuildingToBuildingHeatExchangeMessage::cellsInContact");

static_assert(sizeof(AddElementChunkMessage) == 0x20, "AddElementChunkMessage size");
static_assert(offsetof(AddElementChunkMessage, gameCell) == 0x0, "AddElementChunkMessage::gameCell");
static_assert(offsetof(AddElementChunkMessage, callbackIdx) == 0x4, "AddElementChunkMessage::callbackIdx");
static_assert(offsetof(AddElementChunkMessage, mass) == 0x8, "AddElementChunkMessage::mass");
static_assert(offsetof(AddElementChunkMessage, temperature) == 0xc, "AddElementChunkMessage::temperature");
static_assert(offsetof(AddElementChunkMessage, surfaceArea) == 0x10, "AddElementChunkMessage::surfaceArea");
static_assert(offsetof(AddElementChunkMessage, thickness) == 0x14, "AddElementChunkMessage::thickness");
static_assert(offsetof(AddElementChunkMessage, groundTransferScale) == 0x18, "AddElementChunkMessage::groundTransferScale");
static_assert(offsetof(AddElementChunkMessage, elementIdx) == 0x1c, "AddElementChunkMessage::elementIdx");
static_assert(offsetof(AddElementChunkMessage, pad0) == 0x1e, "AddElementChunkMessage::pad0");
static_assert(offsetof(AddElementChunkMessage, pad1) == 0x1f, "AddElementChunkMessage::pad1");

static_assert(sizeof(ModifyElementChunkMessage) == 0xc, "ModifyElementChunkMessage size");
static_assert(offsetof(ModifyElementChunkMessage, handle) == 0x0, "ModifyElementChunkMessage::handle");
static_assert(offsetof(ModifyElementChunkMessage, temperature) == 0x4, "ModifyElementChunkMessage::temperature");
static_assert(offsetof(ModifyElementChunkMessage, heatCapacity) == 0x8, "ModifyElementChunkMessage::heatCapacity");

static_assert(sizeof(RemoveElementChunkMessage) == 0x8, "RemoveElementChunkMessage size");
static_assert(offsetof(RemoveElementChunkMessage, handle) == 0x0, "RemoveElementChunkMessage::handle");
static_assert(offsetof(RemoveElementChunkMessage, callbackIdx) == 0x4, "RemoveElementChunkMessage::callbackIdx");

static_assert(sizeof(MoveElementChunkMessage) == 0x8, "MoveElementChunkMessage size");
static_assert(offsetof(MoveElementChunkMessage, handle) == 0x0, "MoveElementChunkMessage::handle");
static_assert(offsetof(MoveElementChunkMessage, gameCell) == 0x4, "MoveElementChunkMessage::gameCell");

static_assert(sizeof(ModifyElementChunkEnergyMessage) == 0x8, "ModifyElementChunkEnergyMessage size");
static_assert(offsetof(ModifyElementChunkEnergyMessage, handle) == 0x0, "ModifyElementChunkEnergyMessage::handle");
static_assert(offsetof(ModifyElementChunkEnergyMessage, deltaKJ) == 0x4, "ModifyElementChunkEnergyMessage::deltaKJ");

static_assert(sizeof(ModifyElementChunkAdjusterMessage) == 0x10, "ModifyElementChunkAdjusterMessage size");
static_assert(offsetof(ModifyElementChunkAdjusterMessage, handle) == 0x0, "ModifyElementChunkAdjusterMessage::handle");
static_assert(offsetof(ModifyElementChunkAdjusterMessage, temperature) == 0x4, "ModifyElementChunkAdjusterMessage::temperature");
static_assert(offsetof(ModifyElementChunkAdjusterMessage, heatCapacity) == 0x8, "ModifyElementChunkAdjusterMessage::heatCapacity");
static_assert(offsetof(ModifyElementChunkAdjusterMessage, thermalConductivity) == 0xc, "ModifyElementChunkAdjusterMessage::thermalConductivity");

static_assert(sizeof(ModifyBackwallDataMessage) == 0x10, "ModifyBackwallDataMessage size");
static_assert(offsetof(ModifyBackwallDataMessage, gameCell) == 0x0, "ModifyBackwallDataMessage::gameCell");
static_assert(offsetof(ModifyBackwallDataMessage, elementIdx) == 0x4, "ModifyBackwallDataMessage::elementIdx");
static_assert(offsetof(ModifyBackwallDataMessage, pad) == 0x6, "ModifyBackwallDataMessage::pad");
static_assert(offsetof(ModifyBackwallDataMessage, mass) == 0x8, "ModifyBackwallDataMessage::mass");
static_assert(offsetof(ModifyBackwallDataMessage, temperature) == 0xc, "ModifyBackwallDataMessage::temperature");

static_assert(sizeof(AddElementConsumerMessage) == 0xc, "AddElementConsumerMessage size");
static_assert(offsetof(AddElementConsumerMessage, cellIdx) == 0x0, "AddElementConsumerMessage::cellIdx");
static_assert(offsetof(AddElementConsumerMessage, callbackIdx) == 0x4, "AddElementConsumerMessage::callbackIdx");
static_assert(offsetof(AddElementConsumerMessage, radius) == 0x8, "AddElementConsumerMessage::radius");
static_assert(offsetof(AddElementConsumerMessage, configuration) == 0x9, "AddElementConsumerMessage::configuration");
static_assert(offsetof(AddElementConsumerMessage, elementIdx) == 0xa, "AddElementConsumerMessage::elementIdx");

static_assert(sizeof(ModifyElementConsumerMessage) == 0xc, "ModifyElementConsumerMessage size");
static_assert(offsetof(ModifyElementConsumerMessage, handle) == 0x0, "ModifyElementConsumerMessage::handle");
static_assert(offsetof(ModifyElementConsumerMessage, cell) == 0x4, "ModifyElementConsumerMessage::cell");
static_assert(offsetof(ModifyElementConsumerMessage, consumptionRate) == 0x8, "ModifyElementConsumerMessage::consumptionRate");

static_assert(sizeof(RemoveElementConsumerMessage) == 0x8, "RemoveElementConsumerMessage size");
static_assert(offsetof(RemoveElementConsumerMessage, handle) == 0x0, "RemoveElementConsumerMessage::handle");
static_assert(offsetof(RemoveElementConsumerMessage, callbackIdx) == 0x4, "RemoveElementConsumerMessage::callbackIdx");

static_assert(sizeof(AddElementEmitterMessage) == 0x10, "AddElementEmitterMessage size");
static_assert(offsetof(AddElementEmitterMessage, maxPressure) == 0x0, "AddElementEmitterMessage::maxPressure");
static_assert(offsetof(AddElementEmitterMessage, callbackIdx) == 0x4, "AddElementEmitterMessage::callbackIdx");
static_assert(offsetof(AddElementEmitterMessage, onBlockedCB) == 0x8, "AddElementEmitterMessage::onBlockedCB");
static_assert(offsetof(AddElementEmitterMessage, onUnblockedCB) == 0xc, "AddElementEmitterMessage::onUnblockedCB");

static_assert(sizeof(ModifyElementEmitterMessage) == 0x20, "ModifyElementEmitterMessage size");
static_assert(offsetof(ModifyElementEmitterMessage, handle) == 0x0, "ModifyElementEmitterMessage::handle");
static_assert(offsetof(ModifyElementEmitterMessage, cellIdx) == 0x4, "ModifyElementEmitterMessage::cellIdx");
static_assert(offsetof(ModifyElementEmitterMessage, emitInterval) == 0x8, "ModifyElementEmitterMessage::emitInterval");
static_assert(offsetof(ModifyElementEmitterMessage, emitMass) == 0xc, "ModifyElementEmitterMessage::emitMass");
static_assert(offsetof(ModifyElementEmitterMessage, emitTemperature) == 0x10, "ModifyElementEmitterMessage::emitTemperature");
static_assert(offsetof(ModifyElementEmitterMessage, maxPressure) == 0x14, "ModifyElementEmitterMessage::maxPressure");
static_assert(offsetof(ModifyElementEmitterMessage, diseaseCount) == 0x18, "ModifyElementEmitterMessage::diseaseCount");
static_assert(offsetof(ModifyElementEmitterMessage, elementIdx) == 0x1c, "ModifyElementEmitterMessage::elementIdx");
static_assert(offsetof(ModifyElementEmitterMessage, maxDepth) == 0x1e, "ModifyElementEmitterMessage::maxDepth");
static_assert(offsetof(ModifyElementEmitterMessage, diseaseIdx) == 0x1f, "ModifyElementEmitterMessage::diseaseIdx");

static_assert(sizeof(RemoveElementEmitterMessage) == 0x8, "RemoveElementEmitterMessage size");
static_assert(offsetof(RemoveElementEmitterMessage, handle) == 0x0, "RemoveElementEmitterMessage::handle");
static_assert(offsetof(RemoveElementEmitterMessage, callbackIdx) == 0x4, "RemoveElementEmitterMessage::callbackIdx");

static_assert(sizeof(AddDiseaseEmitterMessage) == 0x4, "AddDiseaseEmitterMessage size");
static_assert(offsetof(AddDiseaseEmitterMessage, callbackIdx) == 0x0, "AddDiseaseEmitterMessage::callbackIdx");

static_assert(sizeof(ModifyDiseaseEmitterMessage) == 0x14, "ModifyDiseaseEmitterMessage size");
static_assert(offsetof(ModifyDiseaseEmitterMessage, handle) == 0x0, "ModifyDiseaseEmitterMessage::handle");
static_assert(offsetof(ModifyDiseaseEmitterMessage, gameCell) == 0x4, "ModifyDiseaseEmitterMessage::gameCell");
static_assert(offsetof(ModifyDiseaseEmitterMessage, diseaseIdx) == 0x8, "ModifyDiseaseEmitterMessage::diseaseIdx");
static_assert(offsetof(ModifyDiseaseEmitterMessage, maxDepth) == 0x9, "ModifyDiseaseEmitterMessage::maxDepth");
static_assert(offsetof(ModifyDiseaseEmitterMessage, emitInterval) == 0xc, "ModifyDiseaseEmitterMessage::emitInterval");
static_assert(offsetof(ModifyDiseaseEmitterMessage, emitCount) == 0x10, "ModifyDiseaseEmitterMessage::emitCount");

static_assert(sizeof(RemoveDiseaseEmitterMessage) == 0x8, "RemoveDiseaseEmitterMessage size");
static_assert(offsetof(RemoveDiseaseEmitterMessage, handle) == 0x0, "RemoveDiseaseEmitterMessage::handle");
static_assert(offsetof(RemoveDiseaseEmitterMessage, callbackIdx) == 0x4, "RemoveDiseaseEmitterMessage::callbackIdx");

static_assert(sizeof(AddDiseaseConsumerMessage) == 0xc, "AddDiseaseConsumerMessage size");

static_assert(sizeof(ModifyDiseaseConsumerMessage) == 0x14, "ModifyDiseaseConsumerMessage size");

static_assert(sizeof(RemoveDiseaseConsumerMessage) == 0x8, "RemoveDiseaseConsumerMessage size");
static_assert(offsetof(RemoveDiseaseConsumerMessage, handle) == 0x0, "RemoveDiseaseConsumerMessage::handle");
static_assert(offsetof(RemoveDiseaseConsumerMessage, callbackIdx) == 0x4, "RemoveDiseaseConsumerMessage::callbackIdx");

static_assert(sizeof(AddRadiationEmitterMessage) == 0x24, "AddRadiationEmitterMessage size");
static_assert(offsetof(AddRadiationEmitterMessage, callbackIdx) == 0x0, "AddRadiationEmitterMessage::callbackIdx");
static_assert(offsetof(AddRadiationEmitterMessage, cell) == 0x4, "AddRadiationEmitterMessage::cell");
static_assert(offsetof(AddRadiationEmitterMessage, emitRadiusX) == 0x8, "AddRadiationEmitterMessage::emitRadiusX");
static_assert(offsetof(AddRadiationEmitterMessage, emitRadiusY) == 0xa, "AddRadiationEmitterMessage::emitRadiusY");
static_assert(offsetof(AddRadiationEmitterMessage, emitRads) == 0xc, "AddRadiationEmitterMessage::emitRads");
static_assert(offsetof(AddRadiationEmitterMessage, emitRate) == 0x10, "AddRadiationEmitterMessage::emitRate");
static_assert(offsetof(AddRadiationEmitterMessage, emitSpeed) == 0x14, "AddRadiationEmitterMessage::emitSpeed");
static_assert(offsetof(AddRadiationEmitterMessage, emitDirection) == 0x18, "AddRadiationEmitterMessage::emitDirection");
static_assert(offsetof(AddRadiationEmitterMessage, emitAngle) == 0x1c, "AddRadiationEmitterMessage::emitAngle");
static_assert(offsetof(AddRadiationEmitterMessage, emitType) == 0x20, "AddRadiationEmitterMessage::emitType");

static_assert(sizeof(ModifyRadiationEmitterMessage) == 0x28, "ModifyRadiationEmitterMessage size");
static_assert(offsetof(ModifyRadiationEmitterMessage, handle) == 0x0, "ModifyRadiationEmitterMessage::handle");
static_assert(offsetof(ModifyRadiationEmitterMessage, cell) == 0x4, "ModifyRadiationEmitterMessage::cell");
static_assert(offsetof(ModifyRadiationEmitterMessage, callbackIdx) == 0x8, "ModifyRadiationEmitterMessage::callbackIdx");
static_assert(offsetof(ModifyRadiationEmitterMessage, emitRadiusX) == 0xc, "ModifyRadiationEmitterMessage::emitRadiusX");
static_assert(offsetof(ModifyRadiationEmitterMessage, emitRadiusY) == 0xe, "ModifyRadiationEmitterMessage::emitRadiusY");
static_assert(offsetof(ModifyRadiationEmitterMessage, emitRads) == 0x10, "ModifyRadiationEmitterMessage::emitRads");
static_assert(offsetof(ModifyRadiationEmitterMessage, emitRate) == 0x14, "ModifyRadiationEmitterMessage::emitRate");
static_assert(offsetof(ModifyRadiationEmitterMessage, emitSpeed) == 0x18, "ModifyRadiationEmitterMessage::emitSpeed");
static_assert(offsetof(ModifyRadiationEmitterMessage, emitDirection) == 0x1c, "ModifyRadiationEmitterMessage::emitDirection");
static_assert(offsetof(ModifyRadiationEmitterMessage, emitAngle) == 0x20, "ModifyRadiationEmitterMessage::emitAngle");
static_assert(offsetof(ModifyRadiationEmitterMessage, emitType) == 0x24, "ModifyRadiationEmitterMessage::emitType");

static_assert(sizeof(RemoveRadiationEmitterMessage) == 0x8, "RemoveRadiationEmitterMessage size");
static_assert(offsetof(RemoveRadiationEmitterMessage, handle) == 0x0, "RemoveRadiationEmitterMessage::handle");
static_assert(offsetof(RemoveRadiationEmitterMessage, callbackIdx) == 0x4, "RemoveRadiationEmitterMessage::callbackIdx");

static_assert(sizeof(DebugProperties) == 0xc, "DebugProperties size");
static_assert(offsetof(DebugProperties, buildingTemperatureScale) == 0x0, "DebugProperties::buildingTemperatureScale");
static_assert(offsetof(DebugProperties, buildingToBuildingTemperatureScale) == 0x4, "DebugProperties::buildingToBuildingTemperatureScale");
static_assert(offsetof(DebugProperties, isDebugEditing) == 0x8, "DebugProperties::isDebugEditing");
static_assert(offsetof(DebugProperties, pad0) == 0x9, "DebugProperties::pad0");
static_assert(offsetof(DebugProperties, pad1) == 0xa, "DebugProperties::pad1");
static_assert(offsetof(DebugProperties, pad2) == 0xb, "DebugProperties::pad2");

static_assert(sizeof(NewGameFrame) == 0x1c, "NewGameFrame size");
static_assert(offsetof(NewGameFrame, elapsedSeconds) == 0x0, "NewGameFrame::elapsedSeconds");
static_assert(offsetof(NewGameFrame, minX) == 0x4, "NewGameFrame::minX");
static_assert(offsetof(NewGameFrame, minY) == 0x8, "NewGameFrame::minY");
static_assert(offsetof(NewGameFrame, maxX) == 0xc, "NewGameFrame::maxX");
static_assert(offsetof(NewGameFrame, maxY) == 0x10, "NewGameFrame::maxY");
static_assert(offsetof(NewGameFrame, currentSunlightIntensity) == 0x14, "NewGameFrame::currentSunlightIntensity");
static_assert(offsetof(NewGameFrame, currentCosmicRadiationIntensity) == 0x18, "NewGameFrame::currentCosmicRadiationIntensity");

static_assert(sizeof(SimFrameInfo) == 0x450, "SimFrameInfo size");
static_assert(offsetof(SimFrameInfo, dt) == 0x0, "SimFrameInfo::dt");
static_assert(offsetof(SimFrameInfo, digs) == 0x8, "SimFrameInfo::digs");
static_assert(offsetof(SimFrameInfo, cellModifications) == 0x20, "SimFrameInfo::cellModifications");
static_assert(offsetof(SimFrameInfo, cellEnergyModifications) == 0x38, "SimFrameInfo::cellEnergyModifications");
static_assert(offsetof(SimFrameInfo, insulationChanges) == 0x68, "SimFrameInfo::insulationChanges");
static_assert(offsetof(SimFrameInfo, strengthChanges) == 0x80, "SimFrameInfo::strengthChanges");
static_assert(offsetof(SimFrameInfo, propertyChanges) == 0x98, "SimFrameInfo::propertyChanges");
static_assert(offsetof(SimFrameInfo, diseaseConsumptions) == 0xc8, "SimFrameInfo::diseaseConsumptions");
static_assert(offsetof(SimFrameInfo, diseaseModifications) == 0xe0, "SimFrameInfo::diseaseModifications");
static_assert(offsetof(SimFrameInfo, radiationModifications) == 0xf8, "SimFrameInfo::radiationModifications");
static_assert(offsetof(SimFrameInfo, radiationParams) == 0x110, "SimFrameInfo::radiationParams");
static_assert(offsetof(SimFrameInfo, worldZoneModifications) == 0x128, "SimFrameInfo::worldZoneModifications");
static_assert(offsetof(SimFrameInfo, massConsumptions) == 0x140, "SimFrameInfo::massConsumptions");
static_assert(offsetof(SimFrameInfo, massEmissions) == 0x158, "SimFrameInfo::massEmissions");
static_assert(offsetof(SimFrameInfo, addBuildingHeatExchange) == 0x170, "SimFrameInfo::addBuildingHeatExchange");
static_assert(offsetof(SimFrameInfo, modifyBuildingHeatExchange) == 0x188, "SimFrameInfo::modifyBuildingHeatExchange");
static_assert(offsetof(SimFrameInfo, removeBuildingHeatExchange) == 0x1a0, "SimFrameInfo::removeBuildingHeatExchange");
static_assert(offsetof(SimFrameInfo, modifyBuildingEnergy) == 0x1b8, "SimFrameInfo::modifyBuildingEnergy");
static_assert(offsetof(SimFrameInfo, registerBuildingToBuilding) == 0x1d0, "SimFrameInfo::registerBuildingToBuilding");
static_assert(offsetof(SimFrameInfo, removeBuildingInContact) == 0x1e8, "SimFrameInfo::removeBuildingInContact");
static_assert(offsetof(SimFrameInfo, removeBuildingToBuilding) == 0x200, "SimFrameInfo::removeBuildingToBuilding");
static_assert(offsetof(SimFrameInfo, addBuildingInContact) == 0x218, "SimFrameInfo::addBuildingInContact");
static_assert(offsetof(SimFrameInfo, addElementChunk) == 0x230, "SimFrameInfo::addElementChunk");
static_assert(offsetof(SimFrameInfo, modifyElementChunk) == 0x248, "SimFrameInfo::modifyElementChunk");
static_assert(offsetof(SimFrameInfo, removeElementChunk) == 0x260, "SimFrameInfo::removeElementChunk");
static_assert(offsetof(SimFrameInfo, moveElementChunk) == 0x278, "SimFrameInfo::moveElementChunk");
static_assert(offsetof(SimFrameInfo, modifyElementChunkEnergy) == 0x290, "SimFrameInfo::modifyElementChunkEnergy");
static_assert(offsetof(SimFrameInfo, modifyElementChunkAdjuster) == 0x2a8, "SimFrameInfo::modifyElementChunkAdjuster");
static_assert(offsetof(SimFrameInfo, modifyBackwall) == 0x2c0, "SimFrameInfo::modifyBackwall");
static_assert(offsetof(SimFrameInfo, addElementConsumer) == 0x2d8, "SimFrameInfo::addElementConsumer");
static_assert(offsetof(SimFrameInfo, modifyElementConsumer) == 0x2f0, "SimFrameInfo::modifyElementConsumer");
static_assert(offsetof(SimFrameInfo, removeElementConsumer) == 0x308, "SimFrameInfo::removeElementConsumer");
static_assert(offsetof(SimFrameInfo, addElementEmitter) == 0x320, "SimFrameInfo::addElementEmitter");
static_assert(offsetof(SimFrameInfo, modifyElementEmitter) == 0x338, "SimFrameInfo::modifyElementEmitter");
static_assert(offsetof(SimFrameInfo, removeElementEmitter) == 0x350, "SimFrameInfo::removeElementEmitter");
static_assert(offsetof(SimFrameInfo, addDiseaseEmitter) == 0x368, "SimFrameInfo::addDiseaseEmitter");
static_assert(offsetof(SimFrameInfo, modifyDiseaseEmitter) == 0x380, "SimFrameInfo::modifyDiseaseEmitter");
static_assert(offsetof(SimFrameInfo, removeDiseaseEmitter) == 0x398, "SimFrameInfo::removeDiseaseEmitter");
static_assert(offsetof(SimFrameInfo, addDiseaseConsumer) == 0x3b0, "SimFrameInfo::addDiseaseConsumer");
static_assert(offsetof(SimFrameInfo, modifyDiseaseConsumer) == 0x3c8, "SimFrameInfo::modifyDiseaseConsumer");
static_assert(offsetof(SimFrameInfo, removeDiseaseConsumer) == 0x3e0, "SimFrameInfo::removeDiseaseConsumer");
static_assert(offsetof(SimFrameInfo, addRadiationEmitter) == 0x3f8, "SimFrameInfo::addRadiationEmitter");
static_assert(offsetof(SimFrameInfo, modifyRadiationEmitter) == 0x410, "SimFrameInfo::modifyRadiationEmitter");
static_assert(offsetof(SimFrameInfo, removeRadiationEmitter) == 0x428, "SimFrameInfo::removeRadiationEmitter");
static_assert(offsetof(SimFrameInfo, debugProperties) == 0x440, "SimFrameInfo::debugProperties");

static_assert(sizeof(SimFrameManager) == 0xd0, "SimFrameManager size");
static_assert(offsetof(SimFrameManager, mutex) == 0x0, "SimFrameManager::mutex");
static_assert(offsetof(SimFrameManager, current) == 0x28, "SimFrameManager::current");
static_assert(offsetof(SimFrameManager, freeFrames) == 0x30, "SimFrameManager::freeFrames");
static_assert(offsetof(SimFrameManager, queuedFrames) == 0x48, "SimFrameManager::queuedFrames");
static_assert(offsetof(SimFrameManager, processingFrames) == 0x60, "SimFrameManager::processingFrames");
static_assert(offsetof(SimFrameManager, processedFrames) == 0x78, "SimFrameManager::processedFrames");
static_assert(offsetof(SimFrameManager, activeRegions) == 0xb0, "SimFrameManager::activeRegions");
static_assert(offsetof(SimFrameManager, nextFrame) == 0xc8, "SimFrameManager::nextFrame");

