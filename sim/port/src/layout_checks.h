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
static_assert(offsetof(SimData, buildingHeatExchange) == 0x430, "SimData::buildingHeatExchange");
static_assert(offsetof(SimData, buildingToBuildingHeatExchange) == 0x4d0, "SimData::buildingToBuildingHeatExchange");
static_assert(offsetof(SimData, headless) == 0x719, "SimData::headless");
static_assert(offsetof(SimData, radiationEnabled) == 0x71a, "SimData::radiationEnabled");

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

