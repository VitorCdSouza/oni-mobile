// Worker-pool tasks: each one processes a band of rows of the grid.
#include <cmath>
#include <cstring>

#include "sim_common.h"

namespace {

// Heat flows between two cells only when they differ by at least 1 K, both conduct, and neither
// element has state flag 0x10 set.
bool Exchanges(const float *temperature, const uint16_t *elements, int a, int b) {
    const ElementTemperatureData &ea = gElementTemperatureData[elements[a]];
    const ElementTemperatureData &eb = gElementTemperatureData[elements[b]];
    return 1.0f <= std::fabs(temperature[a] - temperature[b]) && 0.0f < ea.thermalConductivity &&
           0.0f < eb.thermalConductivity && ((ea.state | eb.state) & 0x10) == 0;
}

}  // namespace

// Heat exchange of every cell with its right and upper neighbours (and its backwall), then the
// state transitions, over the task's region.
void SimUpdateTemperatureTask::InternalDoTask() {
    const int width = sim->width;
    const CellSOA &cells = *sim->cells;
    const float *temperature = cells.temperature.data();
    const float *mass = cells.mass.data();
    const uint16_t *elements = cells.elementIdx.data();
    for (int y = region.minY; y < region.maxY; y++) {
        int row = width * y;
        if (!(region.minX < region.maxX)) {
            continue;
        }
        int end = row + region.maxX;
        for (int c = row + region.minX; c < end; c++) {
            if (0.0f < mass[c] && 0.0f < sim->backwall->mass[c]) {
                UpdateTemperatureForBackwallExchange(*sim, *events, c);
            }
            if (c < end - 1 && Exchanges(temperature, elements, c, c + 1)) {
                UpdateTemperature(*sim, *events, c, c + 1);
            }
            if (y < region.maxY - 1) {
                int up = width + c;
                if (Exchanges(temperature, elements, c, up)) {
                    UpdateTemperature(*sim, *events, c, up);
                }
            }
        }
    }
    for (int y = region.minY; y < region.maxY; y++) {
        if (region.maxX <= region.minX) {
            return;
        }
        int row = width * y;
        for (int c = row + region.minX; c < row + region.maxX; c++) {
            DoStateTransition(*sim, *events, c, gElementTemperatureData[elements[c]]);
        }
    }
}

void UpdateCellSOATask::InternalDoTask() {
    for (int y = firstRow; y < firstRow + numRows; y++) {
        size_t n = static_cast<size_t>(rowLength);
        long s = static_cast<long>(y * srcStride + 1);
        long d = static_cast<long>((dstFirstRow - firstRow + y) * rowLength);
        std::memcpy(dst->elementIdx.data() + d, src->elementIdx.data() + s, n * 2);
        std::memcpy(dst->temperature.data() + d, src->temperature.data() + s, n * 4);
        std::memcpy(dst->mass.data() + d, src->mass.data() + s, n * 4);
        std::memcpy(dst->properties.data() + d, src->properties.data() + s, n);
        std::memcpy(dst->insulation.data() + d, src->insulation.data() + s, n);
        std::memcpy(dst->strengthInfo.data() + d, src->strengthInfo.data() + s, n);
        std::memcpy(dst->diseaseIdx.data() + d, src->diseaseIdx.data() + s, n);
        std::memcpy(dst->diseaseCount.data() + d, src->diseaseCount.data() + s, n * 4);
        std::memcpy(dst->diseaseInfestationTickCount.data() + d, src->diseaseInfestationTickCount.data() + s, n);
        std::memcpy(dst->diseaseAccumulatedError.data() + d, src->diseaseAccumulatedError.data() + s, n * 4);
        std::memcpy(dst->unk_0xf0.data() + d, src->unk_0xf0.data() + s, n * 4);
    }
}

void UpdateBackwallTask::InternalDoTask() {
    for (int y = firstRow; y < firstRow + numRows; y++) {
        size_t n = static_cast<size_t>(rowLength);
        long s = static_cast<long>(y * srcStride + 1);
        long d = static_cast<long>((dstFirstRow - firstRow + y) * rowLength);
        std::memcpy(dst->elementIdx.data() + d, src->elementIdx.data() + s, n * 2);
        std::memcpy(dst->mass.data() + d, src->mass.data() + s, n * 4);
        std::memcpy(dst->temperature.data() + d, src->temperature.data() + s, n * 4);
    }
}

// The same row copy for one per-cell Vector4 array (unk_0x60).
template <typename T>
struct UpdateCellTask {
    void *vtable;
    T *src;
    T *dst;
    int32_t numRows;
    int32_t firstRow;
    int32_t srcStride;
    int32_t dstFirstRow;
    int32_t rowLength;
    void InternalDoTask();
};
static_assert(sizeof(UpdateCellTask<Vector4<float>>) == 0x30, "UpdateCellTask size");

template <>
void UpdateCellTask<Vector4<float>>::InternalDoTask() {
    for (int y = firstRow; y < firstRow + numRows; y++) {
        std::memcpy(dst + static_cast<long>((dstFirstRow - firstRow + y) * rowLength),
                    src + (static_cast<long>(srcStride * y) + 1), static_cast<size_t>(rowLength) * 16);
    }
}
