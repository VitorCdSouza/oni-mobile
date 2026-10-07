// Flood fills used by buildings that take matter from the cells around them.
#include <algorithm>

#include "sim_common.h"

namespace {

void ClearQueue(std::queue<FloodFillInfo> &queue) {
    while (!queue.empty()) {
        queue.pop();
    }
}

void PushNeighbours(std::queue<FloodFillInfo> &queue, const FloodFillInfo &at) {
    uint16_t d = static_cast<uint16_t>(at.distance + 1);
    queue.push(FloodFillInfo{at.x, static_cast<uint16_t>(at.y - 1), d});
    queue.push(FloodFillInfo{static_cast<uint16_t>(at.x - 1), at.y, d});
    queue.push(FloodFillInfo{static_cast<uint16_t>(at.x + 1), at.y, d});
    queue.push(FloodFillInfo{at.x, static_cast<uint16_t>(at.y + 1), d});
}

bool InGrid(const SimData &sim, const FloodFillInfo &at, int radius) {
    return static_cast<int>(at.distance) < radius && at.x != 0 && at.y != 0 && static_cast<int>(at.x) < sim.width &&
           static_cast<int>(at.y) < sim.height;
}

}  // namespace

// Breadth-first search from (x, y) up to `radius` steps; `reachable` gets the cells in visiting
// order. With skip_solid, solid cells stop the search.
void GetReachableCells(SimData &sim, int x, int y, int radius, std::vector<int> &visited,
                       std::queue<FloodFillInfo> &queue, std::vector<int> &reachable, bool skip_solid) {
    visited.clear();
    reachable.clear();
    ClearQueue(queue);
    queue.push(FloodFillInfo{static_cast<uint16_t>(x), static_cast<uint16_t>(y), 0});
    while (!queue.empty()) {
        FloodFillInfo at = queue.front();
        queue.pop();
        if (!InGrid(sim, at, radius)) {
            continue;
        }
        int cell = sim.width * at.y + at.x;
        if (skip_solid && StateOf(gElementPostProcessData[sim.updatedCells->elementIdx[cell]].state) == kSolid) {
            continue;
        }
        if (std::find(visited.begin(), visited.end(), cell) != visited.end()) {
            continue;
        }
        visited.push_back(cell);
        reachable.push_back(cell);
        PushNeighbours(queue, at);
    }
}

// Takes up to `amount` kg of `element` from the cells reachable from (x, y), nearest first,
// mixing what it takes into `info`. A solid consumer only crosses solid and vacuum cells; the
// others cross anything but solids.
// Takes up to `amount` of `element` from one cell into `info` (mass, mixed temperature, germs);
// an emptied cell becomes vacuum.
static void ConsumeCell(SimData &sim, float &amount, unsigned short element, ConsumedMassInfo &info, int cell) {
    CellSOA *upd = sim.updatedCells;
    if (element == upd->elementIdx[cell]) {
        float want = amount;
        float have = upd->mass[cell];
        float ratio, left, taken;
        if (want > have) {
            ratio = 1.0f;
            left = 0.0f;
            taken = have;
        } else {
            ratio = want / have;
            left = have - want;
            taken = want;
        }
        uint8_t cell_disease = upd->diseaseIdx[cell];
        int disease = upd->diseaseCount[cell];
        upd->mass[cell] = left;
        int moved_disease = static_cast<int>(static_cast<float>(disease) * ratio);
        amount = amount - taken;
        upd->diseaseCount[cell] = disease - moved_disease;
        if (!(upd->diseaseCount[cell] > 0)) {
            ClearDisease(*upd, cell);
        }
        float info_temperature = info.temperature;
        float cell_temperature = upd->temperature[cell];
        float hi, lo;
        if (cell_temperature > info_temperature) {
            hi = cell_temperature;
            lo = info_temperature;
        } else {
            hi = info_temperature;
            lo = cell_temperature;
        }
        float info_mass = info.mass;
        float total = info_mass + taken;
        info.mass = total;
        info.temperature =
            MaxSS(MinSS((info_temperature * info_mass + taken * cell_temperature) / total, hi), lo);
        if (cell_disease != kNoDisease) {
            DiseaseCount result = CombineDisease(*gDisease, info.diseaseIdx, info.diseaseCount, cell_disease,
                                                 moved_disease);
            info.diseaseIdx = result.idx;
            info.diseaseCount = result.count;
        }
        if (!(0x1p-126f < upd->mass[cell])) {
            CellAccessor accessor = Accessor(upd, cell);
            sim.ClearCell(accessor);
            int game_cell = ToGameCell(sim, cell);
            if (IsValidGameCell(sim, game_cell)) {
                PushSubstanceChange(*sim.events, game_cell);
            }
            sim.unk_0x68[cell] |= 0x1f;
        }
    }
}

void FloodRemoved(SimData &sim, float &amount, unsigned short element, ConsumedMassInfo &info, int x, int y, int radius,
                  std::vector<int> &visited, std::queue<FloodFillInfo> &queue) {
    uint8_t consumed_state = gElementPostProcessData[element].state;
    queue.push(FloodFillInfo{static_cast<uint16_t>(x), static_cast<uint16_t>(y), 0});
    while (!queue.empty()) {
        FloodFillInfo at = queue.front();
        queue.pop();
        if (!InGrid(sim, at, radius)) {
            continue;
        }
        int cell = sim.width * at.y + at.x;
        CellSOA *upd = sim.updatedCells;
        uint8_t state = StateOf(gElementPostProcessData[upd->elementIdx[cell]].state);
        if (StateOf(consumed_state) == kSolid) {
            if (!(1 < static_cast<uint8_t>(state - 1))) {
                continue;
            }
        } else if (state == kSolid) {
            continue;
        }
        if (std::find(visited.begin(), visited.end(), cell) != visited.end()) {
            continue;
        }
        visited.push_back(cell);
        ConsumeCell(sim, amount, element, info, cell);
        if (amount <= 0.0f) {
            return;
        }
        PushNeighbours(queue, at);
    }
}

// Buildings that take matter from the cells around them (pumps, scrubbers, intakes...).
void ElementConsumer::Update(float dt, SimData &sim, const Region &region) {
    for (size_t i = 0; i < data.size(); i++) {
        ElementConsumerData &d = data[i];
        int x = d.x, y = d.y;
        if (!(region.minX <= x && region.minY <= y && x <= region.maxX && y <= region.maxY)) {
            continue;
        }
        float amount = d.consumptionRate * dt;
        bool consume = false;
        uint16_t element = 0;
        if (d.configuration == 1 || d.configuration == 2) {
            // Any liquid / any gas: the first matching cell in reach, round robin.
            GetReachableCells(sim, x, y, d.radius, visited, queue, reachable, true);
            size_t n = reachable.size();
            uint8_t wanted = d.configuration == 1 ? 2 : 1;
            for (size_t k = d.roundRobin; k < n + d.roundRobin; k++) {
                uint16_t e = sim.updatedCells->elementIdx[reachable[k % n]];
                if (gElements[e].state & wanted) {
                    element = e;
                    consume = true;
                    break;
                }
            }
        } else if (d.configuration == 0) {
            element = d.elementIdx;
            consume = true;
        }
        reachable.clear();
        visited.clear();
        if (!consume) {
            continue;
        }
        ConsumedMassInfo info{};
        info.simHandle = static_cast<int32_t>(denseToHandle[i]);
        info.removedElemIdx = element;
        info.diseaseIdx = kNoDisease;
        FloodRemoved(sim, amount, element, info, x, y, d.radius, visited, queue);
        if (0.0f < info.mass) {
            consumed.push_back(info);
        }
        ClearQueue(queue);
        d.roundRobin = static_cast<uint8_t>(d.roundRobin + 1);
    }
}

// The consumer variant with a rectangle (radius columns by height rows from x, y) instead of a
// flood fill.
void RectangularRemoved(SimData &sim, float &amount, unsigned short element, ConsumedMassInfo &info, int x, int y,
                        int radius, int height) {
    for (int row = y; row < y + height; row++) {
        for (int col = x; col < x + radius; col++) {
            ConsumeCell(sim, amount, element, info, sim.width * row + col);
            if (amount <= 0.0f) {
                return;
            }
        }
    }
}
