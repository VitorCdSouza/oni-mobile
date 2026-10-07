// Standalone build: GameData (the buffer the sim fills for the game each frame) and its
// conversion to the C# GameDataUpdate.
#include <cstring>

#include "../src/sim_common.h"

CellSOA *NewCellSOA(size_t n);

GameData::GameData(int w, int h) : width(w), height(h), numFramesProcessed(0) {
    size_t n = static_cast<size_t>(static_cast<long>(w * h));
    cells = NewCellSOA(n);
    backwall = new BackwallSOA();
    backwall->noneElementIdx = kInvalidElement;
    backwall->elementIdx.assign(n, kInvalidElement);
    backwall->mass.assign(n, 0.0f);
    backwall->temperature.assign(n, 0.0f);
    flow = new Vector4f[n]();
    accumulatedFlow = new float[n]();
    propertyTextureFlow = new uint64_t[n]();
    propertyTextureLiquid = new uint32_t[n]();
    propertyTextureLiquidData = new uint32_t[n]();
    propertyTextureMaterialData = new uint32_t[n]();
    visibleGrid = new uint8_t[n]();
    propertyTextureExposedToSunlight = new uint8_t[n]();
}

GameData::~GameData() {
    delete cells;
    delete backwall;
    delete[] flow;
    delete[] accumulatedFlow;
    delete[] static_cast<uint64_t *>(propertyTextureFlow);
    delete[] static_cast<uint32_t *>(propertyTextureLiquid);
    delete[] static_cast<uint32_t *>(propertyTextureLiquidData);
    delete[] static_cast<uint32_t *>(propertyTextureMaterialData);
    delete[] visibleGrid;
    delete[] propertyTextureExposedToSunlight;
}

namespace {

// C# Sim.GameDataUpdate (Pack = 4, 496 bytes): numFramesProcessed, twelve cell array pointers,
// then (count, pointer) pairs for the event arrays, the accumulated flow and the five property
// textures. Offsets from PrepareGameDataUpdate and the C# struct.
uint8_t gUpdate[496];

void PutInt(size_t offset, int32_t v) {
    std::memcpy(gUpdate + offset, &v, 4);
}

void PutPtr(size_t offset, const void *p) {
    std::memcpy(gUpdate + offset, &p, 8);
}

template <typename T>
const void *DataOrNull(const std::vector<T> &v) {
    return v.empty() ? nullptr : v.data();
}

template <typename T>
void PutArray(size_t count_offset, const std::vector<T> &v) {
    PutPtr(count_offset + 4, DataOrNull(v));
    PutInt(count_offset, static_cast<int32_t>(v.size()));
}

}  // namespace

uint8_t *PrepareGameDataUpdate(const GameData &game) {
    PutInt(0, game.numFramesProcessed);
    const CellSOA &c = *game.cells;
    PutPtr(4, DataOrNull(c.elementIdx));
    PutPtr(12, DataOrNull(c.temperature));
    PutPtr(52, DataOrNull(c.unk_0xf0));
    PutPtr(20, DataOrNull(c.mass));
    PutPtr(28, DataOrNull(c.properties));
    PutPtr(36, DataOrNull(c.insulation));
    PutPtr(44, DataOrNull(c.strengthInfo));
    PutPtr(60, DataOrNull(c.diseaseIdx));
    PutPtr(68, DataOrNull(c.diseaseCount));
    const BackwallSOA &bw = *game.backwall;
    PutPtr(76, DataOrNull(bw.elementIdx));
    PutPtr(84, DataOrNull(bw.mass));
    PutPtr(92, DataOrNull(bw.temperature));
    PutArray(100, game.solidInfo);
    PutArray(112, game.liquidChanges);
    for (const LiquidChangeInfo &change : game.liquidChanges) {
        if (change.cellIdx < 0) {
            if (!gAssertsDisabled) {
                AssertFunc("BREAKPT:", 0x1d7, "SimBindings.cpp");
                if (gSimData->numGameCells <= change.cellIdx && !gAssertsDisabled) {
                    AssertFunc("BREAKPT:", 0x1d8, "SimBindings.cpp");
                }
            }
        } else if (gSimData->numGameCells <= change.cellIdx && !gAssertsDisabled) {
            AssertFunc("BREAKPT:", 0x1d8, "SimBindings.cpp");
        }
    }
    PutArray(124, game.solidSubstanceChanges);
    PutArray(136, game.substanceChanges);
    PutArray(148, game.callbacks);
    PutArray(160, game.spawnFallingLiquid);
    PutArray(172, game.digInfo);
    PutArray(184, game.spawnOre);
    PutArray(196, game.spawnFX);
    PutArray(208, game.unstableCells);
    PutArray(220, game.worldDamage);
    PutArray(232, game.buildingTemperatures);
    PutArray(244, game.massConsumed);
    PutArray(256, game.massEmitted);
    PutArray(268, game.diseaseConsumed);
    PutArray(280, game.componentStateChanged);
    PutArray(292, game.removedMass);
    PutArray(304, game.emittedMass);
    PutArray(316, game.elementChunkInfos);
    PutArray(328, game.elementChunkMelted);
    PutArray(340, game.buildingOverheat);
    PutArray(352, game.buildingNoLongerOverheated);
    PutArray(364, game.buildingMelted);
    PutArray(376, game.cellMelted);
    PutArray(388, game.backwallElementChanged);
    PutArray(400, game.backwallShouldTransition);
    PutArray(412, game.diseaseEmitted);
    PutArray(424, game.diseaseConsumedInfos);
    PutArray(436, game.radiationConsumed);
    PutPtr(448, game.accumulatedFlow);
    PutPtr(456, game.propertyTextureFlow);
    PutPtr(464, game.propertyTextureLiquid);
    PutPtr(472, game.propertyTextureLiquidData);
    PutPtr(480, game.propertyTextureMaterialData);
    PutPtr(488, game.propertyTextureExposedToSunlight);
    return gUpdate;
}
