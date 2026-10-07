// SimBase::CopySimDataToGame: hands the frame's results to the game (GameData, later turned into
// the C# GameDataUpdate by PrepareGameDataUpdate).
#include <algorithm>
#include <cstring>
#include <utility>

#include "sim_common.h"
#include "stl_sort.h"

void UpdateFlowTexture(const SimData &sim, GameData &game);
void UpdateLiquidPropertyTexture(const SimData &sim, GameData &game);
void UpdateExposedToSunPropertyTexture(const SimData &sim, GameData &game);

namespace {

template <typename T>
T &Field(SimBase *self, size_t offset) {
    return *reinterpret_cast<T *>(reinterpret_cast<char *>(self) + offset);
}

// Splits the game rows over the tasks (the first `rows % count` tasks take one extra row) and
// runs them. The original queues them on its worker pool and waits; with one worker they run in
// this order, and they write disjoint rows anyway.
template <typename Task, typename Src, typename Dst>
void RunRowTasks(std::vector<Task *> &tasks, int per_task, int extra, Src *src, Dst *dst, int src_stride,
                 int row_length) {
    int first_row = 1;
    int dst_row = 0;
    for (int i = 0; i < static_cast<int>(tasks.size()); i++) {
        Task *task = tasks[i];
        int rows = per_task + (i < extra ? 1 : 0);
        task->src = src;
        task->dst = dst;
        task->numRows = rows;
        task->firstRow = first_row;
        task->srcStride = src_stride;
        task->dstFirstRow = dst_row;
        task->rowLength = row_length;
        task->InternalDoTask();
        first_row += rows;
        dst_row += rows;
    }
}

template <typename T>
void CopyInto(std::vector<T> &dst, const std::vector<T> &src) {
    dst.resize(src.size());
    std::copy(src.begin(), src.end(), dst.begin());
}

bool OreBefore(const SpawnOreInfo &a, const SpawnOreInfo &b) {
    return a.cellIdx < b.cellIdx || (a.cellIdx == b.cellIdx && a.elemIdx < b.elemIdx);
}

// Merges the ore spawned in the same cell with the same element: masses add up and the
// temperature is the mass-weighted mean, clamped between the two temperatures.
void MergeOre(SpawnOreInfo &into, const SpawnOreInfo &other) {
    float ta = into.temperature;
    float tb = other.temperature;
    float mass = other.mass + into.mass;
    float t;
    if (0.0f >= mass) {
        t = 0.0f;
    } else {
        float mixed = (into.mass * ta + other.mass * tb) / mass;
        if (!(ta > tb)) {
            t = ta <= mixed ? MinSS(tb, mixed) : ta;
        } else if (!(mixed >= tb)) {
            t = tb;
        } else {
            t = MinSS(ta, mixed);
        }
    }
    into.temperature = t;
    into.mass = mass;
}

// Frames since unk_0x58 (the game's accumulatedFlow) was last cleared.
int gAccumulatedFlowFrames = 0;

}  // namespace

void SimBase::CopySimDataToGame(SimData &sim, GameData &game, const GameData &previous, int frames) {
    // Cell grids: flow, cells and backwalls, without the border.
    auto &flow_tasks = Field<std::vector<UpdateCellTask<Vector4<float>> *>>(this, 0x110);
    auto &cell_tasks = Field<std::vector<UpdateCellSOATask *>>(this, 0x128);
    auto &backwall_tasks = Field<std::vector<UpdateBackwallTask *>>(this, 0x140);
    int task_count = static_cast<int>(flow_tasks.size());
    int per_task = game.height / task_count;
    int extra = game.height % task_count;
    RunRowTasks(flow_tasks, per_task, extra, reinterpret_cast<Vector4<float> *>(sim.unk_0x60),
                reinterpret_cast<Vector4<float> *>(game.flow), sim.width, game.width);
    RunRowTasks(cell_tasks, per_task, extra, sim.updatedCells, game.cells, sim.width, game.width);
    RunRowTasks(backwall_tasks, per_task, extra, sim.backwall, game.backwall, sim.width, game.width);

    // Component results.
    std::vector<ElementChunkInfo> &chunks = sim.elementChunk.info;
    CopyInto(game.elementChunkInfos, chunks);
    for (ElementChunkInfo &info : chunks) {
        info.deltaKJ = 0.0f;
    }
    std::swap(sim.elementConsumer.consumed, game.removedMass);
    sim.elementConsumer.consumed.clear();
    std::vector<EmittedMassInfo> &emitted = sim.elementEmitter.emitted;
    CopyInto(game.emittedMass, emitted);
    for (EmittedMassInfo &info : emitted) {
        info.elemIdx = sim.vacuumElementIdx;
        info.mass = 0.0f;
        info.temperature = 0.0f;
    }
    CopyInto(game.buildingTemperatures, sim.buildingHeatExchange.temperatures);
    // Swapped, not cleared: the components refill them by handle.
    std::swap(sim.diseaseEmitter.emitted, game.diseaseEmitted);
    std::swap(sim.diseaseConsumer.consumed, game.diseaseConsumedInfos);

    // Events.
    SimEvents &events = *sim.events;
    std::swap(events.substanceChanges, game.substanceChanges);
    std::swap(events.callbacks, game.callbacks);
    std::swap(events.spawnFallingLiquid, game.spawnFallingLiquid);
    std::swap(events.unstableCells, game.unstableCells);
    std::swap(events.elementChunkMelted, game.elementChunkMelted);
    std::swap(events.buildingMelted, game.buildingMelted);
    std::swap(events.buildingOverheat, game.buildingOverheat);
    std::swap(events.buildingNoLongerOverheated, game.buildingNoLongerOverheated);
    std::swap(events.cellMelted, game.cellMelted);
    std::swap(events.worldDamage, game.worldDamage);
    std::swap(events.massConsumed, game.massConsumed);
    std::swap(events.radiationConsumed, game.radiationConsumed);
    std::swap(events.massEmitted, game.massEmitted);
    std::swap(events.diseaseConsumed, game.diseaseConsumed);
    std::swap(events.spawnFX, game.spawnFX);
    std::swap(events.componentStateChanged, game.componentStateChanged);
    std::swap(events.digInfo, game.digInfo);
    std::swap(events.backwallElementChanged, game.backwallElementChanged);
    std::swap(events.backwallShouldTransition, game.backwallShouldTransition);

    // Ore: sorted by cell and element, one entry per pair.
    game.spawnOre.clear();
    std::vector<SpawnOreInfo> &ore = events.spawnOre;
    if (!ore.empty()) {
        stl::Sort(ore.data(), ore.data() + ore.size(), OreBefore);
        size_t current = 0;
        for (size_t i = 1; i < ore.size(); i++) {
            if (ore[current].cellIdx == ore[i].cellIdx && ore[current].elemIdx == ore[i].elemIdx) {
                MergeOre(ore[current], ore[i]);
            } else {
                game.spawnOre.push_back(ore[current]);
                current = i;
            }
        }
        game.spawnOre.push_back(ore[current]);
    }

    events.substanceChanges.clear();
    events.spawnFallingLiquid.clear();
    events.elementChunkMelted.clear();
    events.buildingMelted.clear();
    events.buildingOverheat.clear();
    events.buildingNoLongerOverheated.clear();
    events.cellMelted.clear();
    events.spawnOre.clear();
    events.spawnFX.clear();
    events.unstableCells.clear();
    events.callbacks.clear();
    events.worldDamage.clear();
    events.massConsumed.clear();
    events.radiationConsumed.clear();
    events.massEmitted.clear();
    events.diseaseConsumed.clear();
    events.componentStateChanged.clear();
    events.digInfo.clear();
    events.backwallElementChanged.clear();
    events.backwallShouldTransition.clear();
    game.solidInfo.clear();
    game.liquidChanges.clear();
    game.solidSubstanceChanges.clear();

    // Substance changes: the last one per cell, with the element before (previous frame's grid)
    // and after, plus the solid and liquid change lists the game uses for its caches.
    std::vector<SubstanceChangeInfo> &changes = game.substanceChanges;
    std::stable_sort(changes.begin(), changes.end(),
                     [](const SubstanceChangeInfo &a, const SubstanceChangeInfo &b) { return a.cellIdx < b.cellIdx; });
    auto kept = std::unique(changes.rbegin(), changes.rend(),
                            [](const SubstanceChangeInfo &a, const SubstanceChangeInfo &b) { return a.cellIdx == b.cellIdx; });
    changes.erase(changes.begin(), kept.base());
    const uint16_t *old_elements = previous.cells->elementIdx.data();
    const uint16_t *new_elements = game.cells->elementIdx.data();
    for (SubstanceChangeInfo &change : changes) {
        int cell = change.cellIdx;
        uint16_t old_element = old_elements[cell];
        uint16_t new_element = new_elements[cell];
        change.oldElemIdx = old_element;
        change.newElemIdx = new_element;
        if (old_element == new_element) {
            continue;
        }
        uint8_t old_state = StateOf(gElementTemperatureData[old_element].state);
        uint8_t new_state = StateOf(gElementTemperatureData[new_element].state);
        bool new_solid = new_state == kSolid;
        if (new_solid != (old_state == kSolid)) {
            game.solidInfo.push_back(SolidInfo{cell, new_solid ? -1 : 0});
        }
        if (old_state == kSolid || new_state == kSolid) {
            game.solidSubstanceChanges.push_back(SolidSubstanceChangeInfo{cell});
        }
        if (old_state == kLiquid || new_state == kLiquid) {
            game.liquidChanges.push_back(LiquidChangeInfo{cell});
        }
    }

    // Accumulated flow, game-sized; the sim side restarts every 15 frames.
    for (int y = 0; y < game.height; y++) {
        std::memcpy(game.accumulatedFlow + static_cast<long>(y * game.width),
                    sim.unk_0x58 + (sim.width * (y + 1) + 1), static_cast<size_t>(game.width) * 4);
    }
    gAccumulatedFlowFrames++;
    if (gAccumulatedFlowFrames > 14) {
        std::memset(sim.unk_0x58, 0, static_cast<long>(sim.width) * static_cast<long>(sim.height) * 4);
        gAccumulatedFlowFrames = 0;
    }

    UpdateFlowTexture(sim, game);
    UpdateExposedToSunPropertyTexture(sim, game);
    UpdateLiquidPropertyTexture(sim, game);
    std::swap(sim.unk_0x88, game.visibleGrid);
    game.numFramesProcessed = frames;
}
