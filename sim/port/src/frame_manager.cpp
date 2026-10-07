// SimFrameManager: the game thread queues messages into the current frame; each NewGameFrame
// closes it, and the sim takes the closed frames in SimBase::Step.
#include <mutex>

#include "sim_common.h"

namespace {

std::mutex &Mutex(SimFrameManager &manager) {
    return *reinterpret_cast<std::mutex *>(manager.mutex);
}

template <typename T>
void Queue(std::vector<T> &messages, const void *data) {
    messages.push_back(*static_cast<const T *>(data));
}

// Message ids (SIM_HandleMessage), as unsigned values of the C# constants.
enum : uint32_t {
    kDig = 0x31a728a2,
    kModifyCell = 0xb551f21c,
    kModifyCellEnergy = 0x30c69504,
    kSetInsulation = 0xca6dcf7f,
    kSetStrength = 0x5ef6f94e,
    kSetCellProperties = 0xe406df65,
    kConsumeDisease = 0xc3367400,
    kModifyCellDisease = 0x91833896,
    kModifyCellRadiation = 0x8ddd489b,
    kModifyRadiationParams = 0x167a4883,
    kModifyCellWorldZone = 0xe531d902,
    kConsumeMass = 0x66f9f7e7,
    kEmitMass = 0x2f8570fb,
    kAddBuildingHeatExchange = 0x67a75d28,
    kModifyBuildingHeatExchange = 0x6c5c80a1,
    kRemoveBuildingHeatExchange = 0xe4d0366b,
    kModifyBuildingEnergy = 0xaf9b1296,
    kRegisterBuildingToBuilding = 0xb034c7f7,
    kRemoveBuildingInContact = 0x89282743,
    kRemoveBuildingToBuilding = 0x298ce9ba,
    kAddBuildingInContact = 0xa16c821f,
    kAddElementChunk = 0x562bffb2,
    kModifyElementChunk = 0xe610a87d,
    kRemoveElementChunk = 0xc9961ef5,
    kMoveElementChunk = 0xe9a74e82,
    kModifyElementChunkEnergy = 0x3cd47193,
    kModifyElementChunkAdjuster = 0xad4ae21d,
    kModifyBackwall = 0xdbea0035,
    kAddElementConsumer = 0x78a9f851,
    kModifyElementConsumer = 0x5de8d41a,
    kRemoveElementConsumer = 0x354fbb4e,
    kAddElementEmitter = 0xe1df1f33,
    kModifyElementEmitter = 0x180e482c,
    kRemoveElementEmitter = 0xa527ccf6,
    kAddDiseaseEmitter = 0x589e8233,
    kModifyDiseaseEmitter = 0x8ecdab2c,
    kRemoveDiseaseEmitter = 0x1be72ff6,
    kAddDiseaseConsumer = 0x14c35551,
    kModifyDiseaseConsumer = 0x93576a98,
    kRemoveDiseaseConsumer = 0xd169184e,
    kAddRadiationEmitter = 0xa63ddc6e,
    kModifyRadiationEmitter = 0xe1f618e7,
    kRemoveRadiationEmitter = 0xd605d8b1,
    kSetDebugProperties = 0x9bada664,
    kNewGameFrame = 0xd1c97543,
};

}  // namespace

// Closes the current frame (queues it; past 29 queued frames it keeps collecting into the same
// one) and takes a fresh frame from the pool.
void SimFrameManager::NewFrame() {
    std::lock_guard<std::mutex> lock(Mutex(*this));
    if (current != nullptr) {
        if (queuedFrames.size() > 29) {
            freeFrames.push_back(current);
        } else {
            queuedFrames.push_back(current);
        }
    }
    if (freeFrames.empty()) {
        current = new SimFrameInfo();
        current->debugProperties.buildingTemperatureScale = 0.001f;
        current->debugProperties.buildingToBuildingTemperatureScale = 0.001f;
    } else {
        current = static_cast<SimFrameInfo *>(freeFrames.back());
        freeFrames.pop_back();
    }
}

// Returns the processed frames to the pool and takes every queued frame; returns their count.
int SimFrameManager::BeginFrameProcessing() {
    {
        std::lock_guard<std::mutex> lock(Mutex(*this));
        freeFrames.insert(freeFrames.end(), processedFrames.begin(), processedFrames.end());
        processedFrames.clear();
        processingFrames.insert(processingFrames.end(), queuedFrames.begin(), queuedFrames.end());
        queuedFrames.clear();
    }
    nextFrame = 0;
    return static_cast<int>(processingFrames.size());
}

// Applies the next frame's messages and makes its active regions current; returns its dt.
float SimFrameManager::ProcessNextFrame(SimData &sim) {
    SimFrameInfo *frame = static_cast<SimFrameInfo *>(processingFrames[nextFrame]);
    ProcessFrame(*frame, sim);
    if (&activeRegions != &sim.activeRegions) {
        sim.activeRegions = activeRegions;
    }
    nextFrame++;
    return frame->dt;
}

void SimFrameManager::EndFrameProcessing() {
    std::lock_guard<std::mutex> lock(Mutex(*this));
    auto done = processingFrames.begin() + nextFrame;
    processedFrames.insert(processedFrames.end(), processingFrames.begin(), done);
    processingFrames.erase(processingFrames.begin(), done);
}

// Queues one message into the current frame. Returns false for ids it does not handle.
bool SimFrameManager::HandleMessage(int msg_id, int length, void *data, void *&out) {
    (void)out;
    SimFrameInfo &f = *current;
    switch (static_cast<uint32_t>(msg_id)) {
    case kDig: Queue(f.digs, data); return true;
    case kModifyCell: Queue(f.cellModifications, data); return true;
    case kModifyCellEnergy: Queue(f.cellEnergyModifications, data); return true;
    case kSetInsulation: Queue(f.insulationChanges, data); return true;
    case kSetStrength: Queue(f.strengthChanges, data); return true;
    case kSetCellProperties: Queue(f.propertyChanges, data); return true;
    case kConsumeDisease: Queue(f.diseaseConsumptions, data); return true;
    case kModifyCellDisease: Queue(f.diseaseModifications, data); return true;
    case kModifyCellRadiation: Queue(f.radiationModifications, data); return true;
    case kModifyRadiationParams: Queue(f.radiationParams, data); return true;
    case kModifyCellWorldZone: Queue(f.worldZoneModifications, data); return true;
    case kConsumeMass: Queue(f.massConsumptions, data); return true;
    case kEmitMass: Queue(f.massEmissions, data); return true;
    case kAddBuildingHeatExchange: Queue(f.addBuildingHeatExchange, data); return true;
    case kModifyBuildingHeatExchange: Queue(f.modifyBuildingHeatExchange, data); return true;
    case kRemoveBuildingHeatExchange: Queue(f.removeBuildingHeatExchange, data); return true;
    case kModifyBuildingEnergy: Queue(f.modifyBuildingEnergy, data); return true;
    case kRegisterBuildingToBuilding: Queue(f.registerBuildingToBuilding, data); return true;
    case kRemoveBuildingInContact: Queue(f.removeBuildingInContact, data); return true;
    case kRemoveBuildingToBuilding: Queue(f.removeBuildingToBuilding, data); return true;
    case kAddBuildingInContact: Queue(f.addBuildingInContact, data); return true;
    case kAddElementChunk: Queue(f.addElementChunk, data); return true;
    case kModifyElementChunk: Queue(f.modifyElementChunk, data); return true;
    case kRemoveElementChunk: Queue(f.removeElementChunk, data); return true;
    case kMoveElementChunk: Queue(f.moveElementChunk, data); return true;
    case kModifyElementChunkEnergy: Queue(f.modifyElementChunkEnergy, data); return true;
    case kModifyElementChunkAdjuster: Queue(f.modifyElementChunkAdjuster, data); return true;
    case kModifyBackwall: Queue(f.modifyBackwall, data); return true;
    case kAddElementConsumer: Queue(f.addElementConsumer, data); return true;
    case kModifyElementConsumer: Queue(f.modifyElementConsumer, data); return true;
    case kRemoveElementConsumer: Queue(f.removeElementConsumer, data); return true;
    case kAddElementEmitter: Queue(f.addElementEmitter, data); return true;
    case kModifyElementEmitter: Queue(f.modifyElementEmitter, data); return true;
    case kRemoveElementEmitter: Queue(f.removeElementEmitter, data); return true;
    case kAddDiseaseEmitter: Queue(f.addDiseaseEmitter, data); return true;
    case kModifyDiseaseEmitter: Queue(f.modifyDiseaseEmitter, data); return true;
    case kRemoveDiseaseEmitter: Queue(f.removeDiseaseEmitter, data); return true;
    case kAddDiseaseConsumer: Queue(f.addDiseaseConsumer, data); return true;
    case kModifyDiseaseConsumer: Queue(f.modifyDiseaseConsumer, data); return true;
    case kRemoveDiseaseConsumer: Queue(f.removeDiseaseConsumer, data); return true;
    case kAddRadiationEmitter: Queue(f.addRadiationEmitter, data); return true;
    case kModifyRadiationEmitter: Queue(f.modifyRadiationEmitter, data); return true;
    case kRemoveRadiationEmitter: Queue(f.removeRadiationEmitter, data); return true;
    case kSetDebugProperties:
        f.debugProperties = *static_cast<const DebugProperties *>(data);
        return true;
    case kNewGameFrame: {
        // One entry per active world; the frame's dt comes from the first.
        const NewGameFrame *frames = static_cast<const NewGameFrame *>(data);
        f.dt = frames[0].elapsedSeconds;
        activeRegions.clear();
        unsigned long count = static_cast<unsigned long>(static_cast<long>(length)) / sizeof(NewGameFrame);
        for (unsigned long i = 0; i < count; i++) {
            const NewGameFrame &g = frames[i];
            activeRegions.push_back(ActiveRegion{g.minX + 1, g.minY + 1, g.maxX + 1, g.maxY + 1,
                                                 g.currentSunlightIntensity, g.currentCosmicRadiationIntensity});
        }
        NewFrame();
        return true;
    }
    default:
        return false;
    }
}
