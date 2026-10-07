// Germ growth and death per cell (Disease::PostProcess).
#include <math.h>

#include "sim_common.h"

// C# Klei.AI.Disease.RangeInfo: minViable, minGrowth, maxGrowth, maxViable.
struct DiseaseInfo {
    struct RangeInfo {
        float v[4];
    };
};

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
                upd.temperature[c], *reinterpret_cast<const DiseaseInfo::RangeInfo *>(Range(idx, kTemperatureRange)),
                *reinterpret_cast<const DiseaseInfo::RangeInfo *>(Range(idx, kTemperatureHalfLives)));
            float rate = HalfLifeToGrowthRate(half_life);
            int count = upd.diseaseCount[c];
            float fc = static_cast<float>(count);
            uint16_t e = upd.elementIdx[c];
            float mass = upd.mass[c];
            float value = (upd.diseaseAccumulatedError[c] - fc) + rate * fc;
            float min_count = Table<float>(idx, kMinCountPerKG)[e] * mass;
            float max_count = mass * Table<float>(idx, kMaxCountPerKG)[e];
            const float *half_lives;
            if (!(max_count < fc) && fc >= min_count) {
                half_lives = Table<float>(idx, kPopulationHalfLife);
            } else if (min_count > static_cast<float>(upd.diseaseCount[c])) {
                half_lives = nullptr;
                value -= 0.2f * Table<float>(idx, kUnderPopulationDeathRate)[e];
            } else {
                half_lives = Table<float>(idx, kOverPopulationHalfLife);
            }
            if (half_lives != nullptr) {
                float growth = HalfLifeToGrowthRate(half_lives[e]);
                count = upd.diseaseCount[c];
                value += (growth - 1.0f) * static_cast<float>(count);
            }
            if (radiation) {
                value -= upd.unk_0xf0[c] * Field(idx, kRadiationKillRate);
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
    size_t count = static_cast<size_t>(recordsEnd - records) / 0x110;
    for (size_t i = 0; i < count && i <= 0xfe; i++) {
        uint32_t record_hash;
        __builtin_memcpy(&record_hash, records + i * 0x110, sizeof record_hash);
        if (record_hash == hash) {
            return static_cast<uint8_t>(i);
        }
    }
    return 0xff;
}
