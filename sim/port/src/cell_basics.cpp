// Small per-cell operations used everywhere: clearing a cell, mixing germs, and the stability
// counter of unstable solids.
#include "sim_common.h"

void SimData::ClearCell(CellAccessor &cell) {
    CellSOA &soa = *cell.soa;
    int c = cell.cell;
    soa.elementIdx[c] = vacuumElementIdx;
    soa.mass[c] = 0.0f;
    soa.temperature[c] = 0.0f;
    ClearDisease(soa, c);
}

// The low 5 bits of unk_0x68 count the ticks an unsupported unstable solid keeps standing; 0x1f
// means "not counting", which starts a new count of 3 to 6 ticks. Returns the remaining count
// (0 when the solid falls now).
uint8_t SimData::GetStableTicksRemaining(unsigned long cell) {
    uint8_t &flags = unk_0x68[cell];
    uint32_t ticks = flags & 0x1f;
    if (ticks != 0x1f) {
        if (ticks != 0) {
            ticks -= 1;
            flags = static_cast<uint8_t>((flags & 0xe0) | (ticks & 0x1f));
        }
        return static_cast<uint8_t>(ticks);
    }
    rngState = rngState * 0x343fdu + 0x269ec3u;
    float roll = static_cast<float>(static_cast<int>((rngState >> 16) & 0x7fff));
    int start = static_cast<int>(roll * 0x1.8003p-14f) + 3;
    flags = static_cast<uint8_t>((flags & 0xe0) | (start & 0x1f));
    return static_cast<uint8_t>(start);
}

namespace {

struct DiseaseCount {
    uint8_t idx;
    int count;
};

// Disease::CalculateFinalDiseaseCount: two germ populations in one cell fight; the stronger
// (count times weight) one survives with what is left of it.
DiseaseCount CombineDisease(const Disease &disease, uint8_t idx_a, int count_a, uint8_t idx_b, int count_b) {
    if (idx_a == idx_b) {
        return {idx_a, count_b + count_a};
    }
    if (idx_a == kNoDisease) {
        return {idx_b, count_b};
    }
    if (idx_b == kNoDisease) {
        return {idx_a, count_a};
    }
    float a = disease.Weight(idx_a) * static_cast<float>(count_a);
    float b = disease.Weight(idx_b) * static_cast<float>(count_b);
    if (a > b) {
        int left = static_cast<int>(static_cast<float>(count_b) - (a / b) * static_cast<float>(count_a));
        if (left < 0) {
            return {idx_a, -left};
        }
        return {idx_b, left};
    }
    if (count_a < 0) {
        return {idx_b, -count_a};
    }
    return {idx_a, count_a};
}

}  // namespace

void Disease::AddDiseaseToCell(CellSOA *soa, int cell, unsigned char disease_idx, int count) {
    uint8_t current = soa->diseaseIdx[cell];
    DiseaseCount result = CombineDisease(*this, current, soa->diseaseCount[cell], disease_idx, count);
    soa->diseaseIdx[cell] = result.idx;
    soa->diseaseCount[cell] = result.count;
    if (result.count > 0) {
        if (result.idx != current) {
            soa->diseaseInfestationTickCount[cell] = 0;
        }
        return;
    }
    ClearDisease(*soa, cell);
}

// How readily the germs in `cell` spread: zero below the disease's minimum count or minimum
// infestation ticks for that element.
float Disease::GetDiffusionScale(const SimData &sim, int cell) {
    const CellSOA &cells = *sim.cells;
    uint8_t idx = cells.diseaseIdx[cell];
    int count = cells.diseaseCount[cell];
    uint16_t element = cells.elementIdx[cell];
    if (Table<int32_t>(idx, 0xd8)[element] > count) {
        return 0.0f;
    }
    if (Table<uint8_t>(idx, 0xf0)[element] > cells.diseaseInfestationTickCount[cell]) {
        return 0.0f;
    }
    return Table<float>(idx, 0x90)[element];
}

// Germs spreading between two neighbouring cells of the same state: 1/8 of the dominant side
// (scaled by its diffusion scale) moves across; the same germ evens out its counts instead.
void Disease::UpdateCells(float dt, SimData &sim, SimEvents &events, int a, int b) {
    (void)dt;
    (void)events;
    const CellSOA &cells = *sim.cells;
    const ElementPostProcessData *pp = gElementPostProcessData.data();
    if ((pp[cells.elementIdx[a]].state ^ pp[cells.elementIdx[b]].state) & 3) {
        return;
    }
    uint8_t ia = cells.diseaseIdx[a];
    uint8_t ib = cells.diseaseIdx[b];
    int to_a, to_b;
    uint8_t idx_a, idx_b;
    if (ia == ib) {
        if (ia == kNoDisease) {
            return;
        }
        int diff = cells.diseaseCount[b] - cells.diseaseCount[a];
        float scale = GetDiffusionScale(sim, diff > 0 ? b : a);
        idx_a = cells.diseaseIdx[a];
        idx_b = cells.diseaseIdx[b];
        to_a = static_cast<int>(static_cast<float>(diff) * 0.125f * scale);
        to_b = -to_a;
    } else {
        bool b_dominates = ia == kNoDisease;
        if (!b_dominates && ib != kNoDisease) {
            float wa = static_cast<float>(cells.diseaseCount[a]) * Weight(ia);
            float wb = static_cast<float>(cells.diseaseCount[b]) * Weight(ib);
            b_dominates = !(wa > wb);
        }
        if (b_dominates) {
            float scale = GetDiffusionScale(sim, b);
            idx_a = idx_b = sim.cells->diseaseIdx[b];
            to_a = static_cast<int>(static_cast<float>(sim.cells->diseaseCount[b]) * 0.125f * scale);
            to_b = -to_a;
        } else {
            float scale = GetDiffusionScale(sim, a);
            idx_a = idx_b = sim.cells->diseaseIdx[a];
            to_b = static_cast<int>(static_cast<float>(sim.cells->diseaseCount[a]) * 0.125f * scale);
            to_a = -to_b;
        }
    }
    if (to_a != 0) {
        AddDiseaseToCell(sim.updatedCells, a, idx_a, to_a);
    }
    if (to_b != 0) {
        AddDiseaseToCell(sim.updatedCells, b, idx_b, to_b);
    }
}
