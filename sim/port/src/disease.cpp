// Germ growth and death per cell (Disease::PostProcess).
#include <math.h>

#include "binary_buffer.h"
#include "sim_common.h"


// Half life at a temperature: interpolated between the half lives of the two RangeInfo bounds
// around it, infinite (no change) between minGrowth and maxGrowth or when a bound's half life is.
float CalculateRangeHalfLife(float temperature, const DiseaseInfo::RangeInfo &range_info,
                             const DiseaseInfo::RangeInfo &half_life_info) {
    const float *range = range_info.v;
    const float *half_lives = half_life_info.v;
    int lo;
    int hi;
    if (!(temperature > range[0])) {
        lo = 0;
        hi = 0;
    } else if (!(temperature > range[1])) {
        lo = 0;
        hi = 1;
    } else if (!(temperature > range[2])) {
        return INFINITY;
    } else if (!(temperature > range[3])) {
        lo = 2;
        hi = 3;
    } else {
        lo = 3;
        hi = 3;
    }
    float h_lo = half_lives[lo];
    float h_hi = half_lives[hi];
    if (h_lo > 0x1.fffffep+127f || h_hi > 0x1.fffffep+127f) {
        return INFINITY;
    }
    float span = range[hi] - range[lo];
    if (!(span > 0.0f)) {
        return h_lo;
    }
    float f = (temperature - range[lo]) / span;
    return h_lo * (1.0f - f) + f * h_hi;
}

// Growth factor per tick (0.2 s) for a half life in seconds; 0 stays 0, infinite means 1.
float HalfLifeToGrowthRate(float half_life) {
    if (!(half_life < 0.0f || half_life > 0.0f)) {
        return half_life;
    }
    if (half_life > 0x1.fffffep+127f) {
        return 1.0f;
    }
    return exp2f(-0.2f / half_life);
}

// Germ counts grow or decay with the temperature half life and the element's population limits
// (under the minimum they die at underPopulationDeathRate, over the maximum they decay with the
// overpopulation half life), and radiation kills them. The fractional part is carried in
// diseaseAccumulatedError. Then empty cells lose their germ type and the infestation tick count
// of every cell advances (up to 0xfe).
void Disease::PostProcess(SimData &sim, SimEvents &events, int min_x, int max_x, int min_y, int max_y) {
    (void)events;
    const bool radiation = sim.radiationEnabled;
    CellSOA &upd = *sim.updatedCells;
    if (!(min_y < max_y)) {
        return;
    }
    for (int y = min_y; y < max_y; y++) {
        int row = y * sim.width;
        for (int c = row + min_x; c < row + max_x; c++) {
            uint8_t idx = upd.diseaseIdx[c];
            if (idx == kNoDisease) {
                continue;
            }
            float half_life = CalculateRangeHalfLife(
                upd.temperature[c], Info(idx).temperatureRange, Info(idx).temperatureHalfLives);
            float rate = HalfLifeToGrowthRate(half_life);
            int count = upd.diseaseCount[c];
            float fc = static_cast<float>(count);
            uint16_t e = upd.elementIdx[c];
            float mass = upd.mass[c];
            float value = (upd.diseaseAccumulatedError[c] - fc) + rate * fc;
            float min_count = Info(idx).growth.minCountPerKG[e] * mass;
            float max_count = mass * Info(idx).growth.maxCountPerKG[e];
            const float *half_lives;
            if (!(max_count < fc) && fc >= min_count) {
                half_lives = Info(idx).growth.populationHalfLife.data();
            } else if (min_count > static_cast<float>(upd.diseaseCount[c])) {
                half_lives = nullptr;
                value -= 0.2f * Info(idx).growth.underPopulationDeathRate[e];
            } else {
                half_lives = Info(idx).growth.overPopulationHalfLife.data();
            }
            if (half_lives != nullptr) {
                float growth = HalfLifeToGrowthRate(half_lives[e]);
                count = upd.diseaseCount[c];
                value += (growth - 1.0f) * static_cast<float>(count);
            }
            if (radiation) {
                value -= upd.unk_0xf0[c] * Info(idx).radiationKillRate;
            }
            int whole = static_cast<int>(value);
            upd.diseaseAccumulatedError[c] = value - static_cast<float>(whole);
            upd.diseaseCount[c] = whole + count;
        }
    }
    if (!(min_x < max_x)) {
        return;
    }
    for (int y = min_y; y < max_y; y++) {
        int row = y * sim.width;
        for (int c = row + min_x; c < row + max_x; c++) {
            if (upd.diseaseCount[c] <= 0) {
                upd.diseaseIdx[c] = kNoDisease;
                upd.diseaseCount[c] = 0;
                upd.diseaseInfestationTickCount[c] = 0;
                upd.diseaseAccumulatedError[c] = 0.0f;
            }
            int ticks = upd.diseaseInfestationTickCount[c] + 1;
            upd.diseaseInfestationTickCount[c] = ticks > 0xfd ? 0xfe : static_cast<uint8_t>(ticks);
        }
    }
}

// Index of the disease with this name hash (the first uint32 of its record), 0xff if none.
// Only the first 255 records are searched.
uint8_t Disease::GetDiseaseIndex(uint32_t hash) const {
    for (size_t i = 0; i < infos.size() && i <= 0xfe; i++) {
        if (infos[i].hash == hash) {
            return static_cast<uint8_t>(i);
        }
    }
    return 0xff;
}

void DiseaseInfo::ElemGrowthInfo::Initialize(int element_count) {
    size_t n = static_cast<size_t>(static_cast<long>(element_count));
    underPopulationDeathRate.resize(n);
    populationHalfLife.resize(n);
    overPopulationHalfLife.resize(n);
    diffusionScale.resize(n);
    minCountPerKG.resize(n);
    maxCountPerKG.resize(n);
    minDiffusionCount.resize(n);
    minDiffusionInfestationTickCount.resize(n);
}

namespace {

void ReadRange(BinaryBufferReader &reader, DiseaseInfo::RangeInfo &range) {
    for (float &v : range.v) {
        reader >> v;
    }
}

}  // namespace

// Message: int germ count, int element count, then per germ: name, hash, strength, four
// RangeInfo (temperature, temperature half lives, pressure, pressure half lives), radiation kill
// rate and the ElemGrowthInfo of every element.
Disease::Disease(StreamReader &stream) {
    BinaryBufferReader &reader = reinterpret_cast<BinaryBufferReader &>(stream);
    int count = reader.Read<int32_t>();
    int element_count = reader.Read<int32_t>();
    names.resize(static_cast<size_t>(static_cast<long>(count)));
    infos.resize(static_cast<size_t>(static_cast<long>(count)));
    if (static_cast<long>(element_count) != static_cast<long>(gElements.size())) {
        AssertFunc("BREAKPT:", 0x4a, "Disease.cpp");
        KCrashReporterReportMessage("Disease.cpp", 0x4a,
                                    "Elements table contains %d elements but diseases are only listing %d growth info "
                                    "entries",
                                    element_count, static_cast<int>(gElements.size()));
    }
    for (int i = 0; i < count; i++) {
        DiseaseInfo &d = infos[static_cast<size_t>(i)];
        reader >> names[static_cast<size_t>(i)];
        reader >> d.hash;
        reader >> d.strength;
        ReadRange(reader, d.temperatureRange);
        ReadRange(reader, d.temperatureHalfLives);
        ReadRange(reader, d.pressureRange);
        ReadRange(reader, d.pressureHalfLives);
        reader >> d.radiationKillRate;
        d.growth.Initialize(element_count);
        for (size_t e = 0; e < static_cast<size_t>(static_cast<long>(element_count)); e++) {
            reader >> d.growth.underPopulationDeathRate[e];
            reader >> d.growth.populationHalfLife[e];
            reader >> d.growth.overPopulationHalfLife[e];
            reader >> d.growth.diffusionScale[e];
            reader >> d.growth.minCountPerKG[e];
            reader >> d.growth.maxCountPerKG[e];
            reader >> d.growth.minDiffusionCount[e];
            reader >> d.growth.minDiffusionInfestationTickCount[e];
        }
    }
}
