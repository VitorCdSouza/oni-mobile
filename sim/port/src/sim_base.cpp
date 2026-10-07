// SimBase: the per-frame driver around the simulation data.
#include <cstring>

#include "sim_common.h"

namespace {

template <typename T>
void CopyVector(std::vector<T> &dst, const std::vector<T> &src) {
    dst.resize(src.size());
    if (!src.empty()) {
        std::memcpy(dst.data(), src.data(), src.size() * sizeof(T));
    }
}

}  // namespace

// Makes `cells` (the previous state read by the next pass) equal to `updatedCells`. The region
// arguments are ignored by the original too.
void SimBase::UpdatedCellsToCells(SimData &sim, Vector2<int> min, Vector2<int> max) {
    (void)min;
    (void)max;
    CellSOA &cells = *sim.cells;
    const CellSOA &upd = *sim.updatedCells;
    CopyVector(cells.elementIdx, upd.elementIdx);
    CopyVector(cells.temperature, upd.temperature);
    CopyVector(cells.mass, upd.mass);
    CopyVector(cells.properties, upd.properties);
    CopyVector(cells.insulation, upd.insulation);
    CopyVector(cells.strengthInfo, upd.strengthInfo);
    CopyVector(cells.diseaseIdx, upd.diseaseIdx);
    CopyVector(cells.diseaseCount, upd.diseaseCount);
    CopyVector(cells.diseaseInfestationTickCount, upd.diseaseInfestationTickCount);
    CopyVector(cells.diseaseAccumulatedError, upd.diseaseAccumulatedError);
    CopyVector(cells.unk_0xf0, upd.unk_0xf0);
}

void SimBase::CopyUpdatedCellsToCells(SimData &sim) {
    UpdatedCellsToCells(sim, Vector2<int>{0, 0}, Vector2<int>{sim.width, sim.height});
}
