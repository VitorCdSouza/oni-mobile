// Buildings and critters that put germs into the cells around them.
#include "sim_common.h"

void DiseaseEmitter::Update(float dt, SimData &sim, const Region &region) {
    emitted.resize(handleToDense.size());
    for (size_t i = 0; i < data.size(); i++) {
        DiseaseEmitterData &d = data[i];
        if (d.diseaseIdx == kNoDisease) {
            continue;
        }
        int y = d.cell / sim.width;
        int x = d.cell % sim.width;
        if (!(region.minX <= x && region.minY <= y && x <= region.maxX && y <= region.maxY)) {
            continue;
        }
        float timer = d.timer;
        if (timer >= d.emitInterval) {
            GetReachableCells(sim, x, y, d.range, visited, queue, reachable, false);
            for (int cell : reachable) {
                if (d.count < 0 && sim.updatedCells->diseaseIdx[cell] != d.diseaseIdx) {
                    continue;
                }
                gDisease->AddDiseaseToCell(sim.updatedCells, cell, d.diseaseIdx, d.count);
            }
            timer = d.timer - d.emitInterval;
        }
        d.timer = timer + dt;
    }
}
