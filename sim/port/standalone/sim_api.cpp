// Standalone build: the C entry points the game calls (SIM_*, ConduitTemperatureManager_*), the
// sim thread and the frame handshake between it and the game thread.
#include <pthread.h>

#include <array>
#include <atomic>
#include <condition_variable>
#include <cstring>
#include <functional>
#include <mutex>
#include <new>
#include <thread>
#include <vector>

#if defined(__x86_64__)
#include <xmmintrin.h>
#endif

#include "../src/binary_buffer.h"
#include "../src/sim_common.h"

long CreateElementsTable(BinaryBufferReader &reader);
void DestroyElementsTable();
SimData *AllocateCells(BinaryBufferReader &reader);
long SetWorldZones(BinaryBufferReader &reader);
long DefineWorldOffsets(BinaryBufferReader &reader);
long ClearUnoccupiedCells(BinaryBufferReader &reader);
long SetSavedOptions(BinaryBufferReader &reader);
long FreeGridCells(BinaryBufferReader &reader);
long ResizeAndInitializeVacuumCells(BinaryBufferReader &reader);
long InitializeFromCells(BinaryBufferReader &reader);
SimData *Load(BinaryBufferReader &reader);
void SettleThermalBoundaries(const SimData &sim, const CellSOA *cells, CellSOA *updated);
uint8_t *PrepareGameDataUpdate(const GameData &game);

namespace {

// Two GameData buffers: the sim fills `current` while the game reads `previous`; the game's
// PrepareGameData waits for the sim's step, swaps them and lets the sim run the next step.
struct FrameSync {
    GameData *current = nullptr;
    GameData *previous = nullptr;
    bool ready = false;  // the sim finished a step and waits
    bool initialized = false;
    std::mutex mutex;
    std::condition_variable gameWait;
    std::condition_variable simWait;
    std::mutex dataMutex;  // held by the sim thread while it steps

    void Clear() {
        delete current;
        current = nullptr;
        delete previous;
        previous = nullptr;
        ready = false;
        initialized = false;
    }

    // Both buffers start with the inside of the sim grid (the game sees no border).
    void InitGameData(int game_width, int game_height, int sim_width, const CellSOA &cells) {
        std::lock_guard<std::mutex> lock(mutex);
        if (initialized && !gAssertsDisabled) {
            AssertFunc("BREAKPT:", 0xb, "FrameSync.cpp");
        }
        size_t w = static_cast<size_t>(game_width);
        for (GameData **slot : {&current, &previous}) {
            delete *slot;
            *slot = new GameData(game_width, game_height);
            CellSOA &g = *(*slot)->cells;
            size_t src = static_cast<size_t>(sim_width) + 1;
            for (int y = 0; y < game_height; y++, src += static_cast<size_t>(sim_width)) {
                size_t dst = static_cast<size_t>(y) * w;
                std::memcpy(&g.elementIdx[dst], &cells.elementIdx[src], w * 2);
                std::memcpy(&g.temperature[dst], &cells.temperature[src], w * 4);
                std::memcpy(&g.mass[dst], &cells.mass[src], w * 4);
                std::memcpy(&g.properties[dst], &cells.properties[src], w);
                std::memcpy(&g.insulation[dst], &cells.insulation[src], w);
                std::memcpy(&g.strengthInfo[dst], &cells.strengthInfo[src], w);
                std::memcpy(&g.diseaseIdx[dst], &cells.diseaseIdx[src], w);
                std::memcpy(&g.diseaseCount[dst], &cells.diseaseCount[src], w * 4);
                std::memcpy(&g.diseaseInfestationTickCount[dst], &cells.diseaseInfestationTickCount[src], w);
                std::memcpy(&g.diseaseAccumulatedError[dst], &cells.diseaseAccumulatedError[src], w * 4);
                std::memcpy(&g.unk_0xf0[dst], &cells.unk_0xf0[src], w * 4);
            }
        }
        initialized = true;
    }

    // Game thread: wait for the sim's step, run `fn` on (current, previous), swap, release.
    void GameSync(const std::function<void(GameData *, GameData *)> &fn) {
        std::unique_lock<std::mutex> lock(mutex);
        if (!initialized && !gAssertsDisabled) {
            AssertFunc("BREAKPT:", 0x2b, "FrameSync.cpp");
        }
        gameWait.wait(lock, [this] { return ready; });
        fn(current, previous);
        {
            std::lock_guard<std::mutex> data(dataMutex);
            std::swap(current, previous);
            ready = false;
        }
        lock.unlock();
        simWait.notify_one();
    }

    // Sim thread: report the step done and wait for the game's swap.
    void SimSync() {
        std::unique_lock<std::mutex> lock(mutex);
        if ((ready || !initialized) && !gAssertsDisabled) {
            AssertFunc("BREAKPT:", 0x46, "FrameSync.cpp");
        }
        ready = true;
        gameWait.notify_one();
        simWait.wait(lock, [this] { return !ready; });
    }
};

FrameSync gFrameSync;

// Placeholder for the original's ParallelTaskQueue: UpdateData reads the worker count from the
// worker thread vector at +0xe0 (the original runs with one worker).
struct TaskQueue {
    uint8_t pad[0xe0];
    void **workersBegin;
    void **workersEnd;
    void *worker;
};

// The original's Sim: a Thread (0x38 bytes) followed by SimBase, and the FrameSync it steps
// with. gSim points here; the frame manager is at gSim + 0x40.
struct Sim {
    void *vtable = nullptr;
    uint8_t threadData[0x28] = {};
    std::atomic<bool> stop{false};
    uint8_t pad[7] = {};
    SimBase base;
    FrameSync *sync = nullptr;
    std::thread thread;
    TaskQueue queue{};
};

void EnableFlushToZero() {
#if defined(__x86_64__)
    _mm_setcsr(_mm_getcsr() | 0x8000);
#elif defined(__aarch64__)
    uint64_t fpcr;
    __asm__ volatile("mrs %0, fpcr" : "=r"(fpcr));
    fpcr |= 1u << 24;
    __asm__ volatile("msr fpcr, %0" : : "r"(fpcr));
#endif
}

void SimMain(Sim *sim) {
    EnableFlushToZero();
    while (!sim->stop.load()) {
        {
            std::lock_guard<std::mutex> data(sim->sync->dataMutex);
            if (gConduitTemperatureManager != nullptr) {
                gConduitTemperatureManager->ReleaseQueuedHandles();
            }
            sim->base.Step(*gSimData, *sim->sync->current, *sim->sync->previous);
        }
        sim->sync->SimSync();
    }
}

Sim *NewSim(FrameSync &sync) {
    Sim *sim = new Sim();
    sim->sync = &sync;
    SimBase &b = sim->base;
    new (b.frames.mutex) std::mutex();
    b.frames.current = nullptr;
    b.frames.nextFrame = 0;
    b.frames.NewFrame();
    sim->queue.worker = nullptr;
    sim->queue.workersBegin = &sim->queue.worker;
    sim->queue.workersEnd = &sim->queue.worker + 1;
    b.taskQueue = &sim->queue;
    b.updateTasks.push_back(new SimUpdateTemperatureTask());
    b.taskEvents.push_back(new SimEvents());
    b.flowTasks.push_back(new UpdateCellTask<Vector4<float>>());
    b.cellTasks.push_back(new UpdateCellSOATask());
    b.backwallTasks.push_back(new UpdateBackwallTask());
    b.remainder = 0.0f;
    return sim;
}

void DeleteSim(Sim *sim) {
    SimBase &b = sim->base;
    for (SimUpdateTemperatureTask *t : b.updateTasks) delete t;
    for (SimEvents *e : b.taskEvents) delete e;
    for (void *t : b.flowTasks) delete static_cast<UpdateCellTask<Vector4<float>> *>(t);
    for (UpdateCellSOATask *t : b.cellTasks) delete t;
    for (UpdateBackwallTask *t : b.backwallTasks) delete t;
    for (std::vector<SimFrameInfo *> *frames :
         {&b.frames.freeFrames, &b.frames.queuedFrames, &b.frames.processingFrames, &b.frames.processedFrames}) {
        for (SimFrameInfo *f : *frames) delete f;
    }
    delete b.frames.current;
    reinterpret_cast<std::mutex *>(b.frames.mutex)->~mutex();
    delete sim;
}

Sim *gSimObject = nullptr;
void *gGameMessageHandler = nullptr;

// CreateElementInteractions keeps the gas obliteration rules; nothing in this version of the sim
// reads them (DoGasInteraction is never called).
std::mutex gInteractionsMutex;
std::vector<std::array<uint8_t, 0x1c>> gGasObliterations;

long CreateElementInteractions(BinaryBufferReader &reader) {
    gGasObliterations.clear();
    int count;
    std::memcpy(&count, reader.Current(), 4);
    const uint8_t *entries;
    std::memcpy(&entries, reader.Current() + 4, 8);
    for (int i = 0; i < count; i++) {
        const uint8_t *e = entries + static_cast<size_t>(i) * 0x20;
        uint32_t type;
        std::memcpy(&type, e, 4);
        if (type == 0x6826f67d) {
            std::array<uint8_t, 0x1c> entry;
            std::memcpy(entry.data(), e + 4, 0x1c);
            gGasObliterations.push_back(entry);
        }
    }
    return 0;
}

}  // namespace

char *gSim = nullptr;

namespace {

void CleanUp() {
    if (gSimObject != nullptr) {
        Sim *sim = gSimObject;
        if (sim->thread.joinable()) {
            gFrameSync.GameSync([sim](GameData *, GameData *) { sim->stop = true; });
            sim->thread.join();
        }
        gSimObject = nullptr;
        gSim = nullptr;
        DeleteSim(sim);
    }
    gGameMessageHandler = nullptr;
    SimData *sim_data = gSimData;
    gSimData = nullptr;
    delete sim_data;
    gFrameSync.Clear();
    DestroyElementsTable();
    Disease *disease = gDisease;
    gDisease = nullptr;
    delete disease;
}

long LoadDiseases(BinaryBufferReader &reader) {
    Disease *created = new Disease(reinterpret_cast<StreamReader &>(reader));
    Disease *old = gDisease;
    gDisease = created;
    delete old;
    return 0;
}

// Starts the sim: settles old saves, clears the events, fills both GameData buffers, draws the
// map border, starts the sim thread and returns the first GameDataUpdate.
uint8_t *Start(BinaryBufferReader &) {
    SimData &sim = *gSimData;
    if (sim.unk_0x718) {
        *sim.cells = *sim.updatedCells;
        SettleThermalBoundaries(sim, sim.cells, sim.updatedCells);
    }
    SimEvents &e = *sim.events;
    e.substanceChanges.clear();
    e.spawnFallingLiquid.clear();
    e.elementChunkMelted.clear();
    e.buildingMelted.clear();
    e.buildingOverheat.clear();
    e.buildingNoLongerOverheated.clear();
    e.cellMelted.clear();
    e.spawnOre.clear();
    e.spawnFX.clear();
    e.unstableCells.clear();
    e.callbacks.clear();
    e.worldDamage.clear();
    e.massConsumed.clear();
    e.radiationConsumed.clear();
    e.massEmitted.clear();
    e.diseaseConsumed.clear();
    e.componentStateChanged.clear();
    e.digInfo.clear();
    e.backwallElementChanged.clear();
    e.backwallShouldTransition.clear();
    gFrameSync.InitGameData(sim.width - 2, sim.height - 2, sim.width, *sim.updatedCells);
    gSimData->InitializeBoundary();
    *gSimData->cells = *gSimData->updatedCells;
    Sim *s = gSimObject;
    if (s->thread.joinable()) {
        if (!gAssertsDisabled) {
            AssertFunc("BREAKPT:", 0x18, "thread.cpp");
        }
        s->thread.join();
    }
    s->stop = false;
    s->thread = std::thread(SimMain, s);
    return PrepareGameDataUpdate(*gFrameSync.previous);
}

// Waits for the sim's step, hands it the game's visibility grid and returns the step's result.
uint8_t *PrepareGameData(BinaryBufferReader &reader) {
    if (gSimObject == nullptr) {
        return nullptr;
    }
    gFrameSync.GameSync([&reader](GameData *, GameData *previous) {
        size_t n = static_cast<size_t>(static_cast<long>(previous->width * previous->height));
        size_t left = reader.pos <= reader.size ? reader.size - reader.pos : 0;
        if (n != left && !gAssertsDisabled) {
            AssertFunc("BREAKPT:", 0x30f, "SimBindings.cpp");
        }
        // The original copies n bytes regardless; a short buffer would read past its end.
        size_t copied = n < left ? n : left;
        std::memcpy(previous->visibleGrid, reader.data + reader.pos, copied);
        std::memset(previous->visibleGrid + copied, 0, n - copied);
    });
    return PrepareGameDataUpdate(*gFrameSync.previous);
}

using Handler = void *(*)(BinaryBufferReader &);

template <typename R, R (*F)(BinaryBufferReader &)>
void *Wrap(BinaryBufferReader &reader) {
    return reinterpret_cast<void *>(F(reader));
}

long CreateElementInteractionsLocked(BinaryBufferReader &reader) {
    std::lock_guard<std::mutex> lock(gInteractionsMutex);
    return CreateElementInteractions(reader);
}

long ResizeAndInitializeVacuumCellsMessage(BinaryBufferReader &reader) {
    ResizeAndInitializeVacuumCells(reader);
    return 0;
}

struct MessageHandler {
    uint32_t id;
    Handler handler;
};

const MessageHandler kHandlers[] = {
    {0x404a7123, Wrap<uint8_t *, PrepareGameData>},
    {0x421169ea, Wrap<long, CreateElementsTable>},
    {0xc88ce785, Wrap<long, CreateElementInteractionsLocked>},
    {0x31311baf, Wrap<long, LoadDiseases>},
    {0xe4be0717, Wrap<long, SetWorldZones>},
    {0x7aee0fb9, Wrap<long, InitializeFromCells>},
    {0xd32312c7, Wrap<long, ResizeAndInitializeVacuumCellsMessage>},
    {0xba64e4e7, Wrap<long, FreeGridCells>},
    {0xd7e9e1c6, Wrap<SimData *, Load>},
    {0xc87b4062, Wrap<uint8_t *, Start>},
    {0x411cd3f4, Wrap<SimData *, AllocateCells>},
    {0xca9a7769, Wrap<long, DefineWorldOffsets>},
    {0x928dbf0d, Wrap<long, ClearUnoccupiedCells>},
    {0x44cab6b9, Wrap<long, SetSavedOptions>},
};

}  // namespace

extern "C" {

__attribute__((visibility("default"))) void SIM_Initialize(void *message_handler) {
    CleanUp();
    gGameMessageHandler = message_handler;
    gFrameSync.Clear();
    Sim *old = gSimObject;
    gSimObject = NewSim(gFrameSync);
    gSim = reinterpret_cast<char *>(gSimObject);
    if (old != nullptr) {
        DeleteSim(old);
    }
}

__attribute__((visibility("default"))) void SIM_Shutdown() {
    CleanUp();
}

__attribute__((visibility("default"))) void *SIM_HandleMessage(int msg_id, int length, void *data) {
    if (gSimObject == nullptr) {
        return nullptr;
    }
    BinaryBufferReader reader{nullptr, 0, static_cast<const uint8_t *>(data),
                              static_cast<uint64_t>(static_cast<long>(length))};
    void *out = nullptr;
    if (gSimObject->base.frames.HandleMessage(msg_id, length, data, out)) {
        return nullptr;
    }
    for (const MessageHandler &h : kHandlers) {
        if (h.id == static_cast<uint32_t>(msg_id)) {
            return h.handler(reader);
        }
    }
    return nullptr;
}

__attribute__((visibility("default"))) void *SIM_HandleMessages(int msg_id, int length, int count, uint8_t *data) {
    void *result = nullptr;
    for (int i = 0; i < count; i++) {
        result = SIM_HandleMessage(msg_id, length, data);
        data += length;
    }
    return result;
}

__attribute__((visibility("default"))) void SIM_DebugCrash() {
    AssertFunc("BREAKPT:", 0x1b2, "SimBindings.cpp");
    KCrashReporterReportMessage("SimBindings.cpp", 0x1b2, "Assert failed: false");
}

// --- Pipe contents (called from the game thread) ---

__attribute__((visibility("default"))) void ConduitTemperatureManager_Initialize() {
    ConduitTemperatureManager *old = gConduitTemperatureManager;
    gConduitTemperatureManager = new ConduitTemperatureManager();
    delete old;
}

__attribute__((visibility("default"))) void ConduitTemperatureManager_Shutdown() {
    ConduitTemperatureManager *old = gConduitTemperatureManager;
    gConduitTemperatureManager = nullptr;
    delete old;
}

__attribute__((visibility("default"))) uint32_t ConduitTemperatureManager_Add(
    float contents_temperature, float contents_mass, float conduit_heat_capacity, float conduit_conductivity,
    int element_hash, int structure_handle, bool insulated) {
    return gConduitTemperatureManager->Add(contents_temperature, contents_mass, element_hash, structure_handle,
                                           conduit_heat_capacity, conduit_conductivity, insulated);
}

// The game declares an int return and ignores it; the original returns 0.
__attribute__((visibility("default"))) int ConduitTemperatureManager_Set(float contents_temperature,
                                                                         float contents_mass, int handle,
                                                                         int element_hash) {
    gConduitTemperatureManager->Set(handle, contents_temperature, contents_mass, element_hash);
    return 0;
}

__attribute__((visibility("default"))) void ConduitTemperatureManager_Remove(int handle) {
    gConduitTemperatureManager->Remove(handle);
}

__attribute__((visibility("default"))) void ConduitTemperatureManager_Clear() {
    gConduitTemperatureManager->Clear();
}

__attribute__((visibility("default"))) ConduitTemperatureUpdateData *ConduitTemperatureManager_Update(
    float dt, const BuildingTemperatureInfo *buildings) {
    return gConduitTemperatureManager->Update(dt, buildings);
}

}  // extern "C"
