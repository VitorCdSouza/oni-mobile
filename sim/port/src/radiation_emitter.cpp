// Radiation sources: continuous emitters lighting an ellipse (blocked by matter in the way) and
// pulsing emitters whose ring grows a step at a time.
#include <cmath>
#if defined(__x86_64__)
#include <xmmintrin.h>
#endif

#include "sim_common.h"

// The original's pi constant (3.1415927f), read by the cone test.
namespace Math {
struct FloatConstants {
    static const float PI;
};
}  // namespace Math

namespace {

const float &kPi = Math::FloatConstants::PI;

// rsqrtss: an approximation whose exact bits depend on the CPU. On x86 the port uses the same
// instruction (bit-exact with the original on the same machine); elsewhere the exact value,
// which only changes results when the rounded ellipse extent lands on a .5 boundary.
float ApproxRsqrt(float x) {
#if defined(__x86_64__)
    return _mm_cvtss_f32(_mm_rsqrt_ss(_mm_set_ss(x)));
#else
    return 1.0f / std::sqrt(x);
#endif
}

float AbsSS(float x) {
    return std::fabs(x);
}

// The original's inline floor/round for |x| < 2^23 (larger values are already integers).
float FloorSmall(float x) {
    if (!(8388608.0f > AbsSS(x))) {
        return x;
    }
    float t = static_cast<float>(static_cast<int>(x));
    return t > x ? t - 1.0f : t;
}

float RoundSmall(float x) {
    if (!(8388608.0f > AbsSS(x))) {
        return x;
    }
    float r = static_cast<float>(static_cast<int>(AbsSS(x) + 0x1.fffffep-2f));
    return std::copysign(r, x);
}

int FloorInt(float x) {
    int i = static_cast<int>(x);
    return x < static_cast<float>(i) ? i - 1 : i;
}

// Whether the direction from (cx, cy) to (x, y) falls inside the emitter's cone.
bool InCone(const RadiationEmitterData &d, float dy, float dx) {
    float half = d.angle * 0.5f;
    float a = atan2f(dy, dx);
    float base = 360.0f + d.direction;
    float lo = std::fmod(base - half, 360.0f);
    float hi = std::fmod(half + base, 360.0f);
    float deg = std::fmod((a * 180.0f) / kPi + 360.0f, 360.0f);
    bool after_lo = deg >= lo;
    bool before_hi = hi >= deg;
    if (!(hi > lo)) {
        return after_lo || before_hi;
    }
    return after_lo && before_hi;
}

// Fraction of radiation that gets through one cell (FUN_00165370).
float CellTransmission(const SimData &sim, int x, int y) {
    const CellSOA &upd = *sim.updatedCells;
    long cell = static_cast<long>(y * sim.width + x);
    float absorb;
    if (static_cast<int8_t>(upd.properties[cell]) < 0) {
        absorb = sim.unk_0x72c;
    } else {
        absorb = (upd.mass[cell] / sim.unk_0x720) * sim.unk_0x728 + sim.unk_0x724;
    }
    absorb = gElementRadiationData[upd.elementIdx[cell]].radiationAbsorptionFactor * absorb;
    if (1.0f <= absorb) {
        absorb = 1.0f;
    }
    if (absorb <= 0.0f) {
        absorb = 0.0f;
    }
    return 1.0f - absorb;
}

// Line(float, float, float, float, std::function<void(int, int, float)>): Bresenham on floats.
template <typename F>
void Line(float x0, float y0, float x1, float y1, F &&visit) {
    float dy = y1 - y0;
    float dx = x1 - x0;
    float ady = AbsSS(dy);
    float adx = AbsSS(dx);
    bool steep = ady > adx;
    float a0, b0, a1, b1, delta, err_step;
    if (steep) {
        delta = dy;
        err_step = adx;
        a0 = y0;
        b0 = x0;
        a1 = y1;
        b1 = x1;
    } else {
        delta = dx;
        err_step = ady;
        a0 = x0;
        b0 = y0;
        a1 = x1;
        b1 = y1;
    }
    if (a0 > a1) {
        delta = a0 - a1;
        err_step = AbsSS(b0 - b1);
        std::swap(a0, a1);
        std::swap(b0, b1);
    }
    int ia = static_cast<int>(a0);
    int ia_end = static_cast<int>(a1);
    int ib = static_cast<int>(b0);
    int step = b1 > b0 ? 1 : -1;
    float err = delta * 0.5f;
    for (; ia <= ia_end; ia++) {
        if (steep) {
            visit(ib, ia);
        } else {
            visit(ia, ib);
        }
        err = err - err_step;
        if (0.0f > err) {
            err = err + delta;
            ib += step;
        }
    }
}

float LineOfSight(const SimData &sim, int cx, int cy, int x, int y) {
    float transmission = 1.0f;
    Line(static_cast<float>(cx), static_cast<float>(cy), static_cast<float>(x), static_cast<float>(y),
         [&](int lx, int ly) { transmission = CellTransmission(sim, lx, ly) * transmission; });
    return transmission;
}

bool InGridAndRegion(const SimData &sim, const Region &r, int x, int y) {
    return 0 < x && x < sim.width && 0 < y && y < sim.height && r.minX <= x && r.minY <= y && x <= r.maxX &&
           y <= r.maxY;
}

float &CellRadiation(SimData &sim, int x, int y) {
    return sim.updatedCells->unk_0xf0[static_cast<size_t>(sim.width * y + x)];
}

// Ring visitors (the std::function callbacks of FUN_001657c0).
// FUN_00165420: add, inside the cone.
void RingAdd(SimData &sim, const Region &r, const RadiationEmitterData &d, int cx, int cy, int x, int y, float w,
             float *) {
    if (!InGridAndRegion(sim, r, x, y)) {
        return;
    }
    if (d.angle != 360.0f && (x != cx || y != cy) &&
        !InCone(d, static_cast<float>(y - cy), static_cast<float>(x - cx))) {
        return;
    }
    float &cell = CellRadiation(sim, x, y);
    cell = w + cell;
}

// FUN_001655e0: absorb up to `w` from cells holding at least 0.01, inside the cone.
void RingAbsorb(SimData &sim, const Region &r, const RadiationEmitterData &d, int cx, int cy, int x, int y,
                float w, float *absorbed) {
    if (!InGridAndRegion(sim, r, x, y)) {
        return;
    }
    if (y == cy && x == cx) {
        return;
    }
    if (d.angle != 360.0f && !InCone(d, static_cast<float>(y - cy), static_cast<float>(x - cx))) {
        return;
    }
    float &cell = CellRadiation(sim, x, y);
    float have = cell;
    if (0.01f <= have) {
        if (have <= w) {
            w = have;
        }
        cell = have - w;
        if (absorbed != nullptr) {
            *absorbed = w;
        }
    }
}

// FUN_00166aa0: add through the line of sight, inside the cone.
void RingAddWithSight(SimData &sim, const Region &r, const RadiationEmitterData &d, int cx, int cy, int x, int y,
                      float w, float *) {
    if (x < 1 || !(0.0f < w) || sim.width <= x || y < 1 || sim.height <= y || x < r.minX || y < r.minY ||
        r.maxX < x || r.maxY < y) {
        return;
    }
    if (y == cy && x == cx) {
        return;
    }
    if (d.angle != 360.0f && !InCone(d, static_cast<float>(y - cy), static_cast<float>(x - cx))) {
        return;
    }
    float t = LineOfSight(sim, cx, cy, x, y);
    float v = 1.0f;
    if (t <= 1.0f) {
        v = t;
    }
    if (v <= 0.0f) {
        v = 0.0f;
    }
    float &cell = CellRadiation(sim, x, y);
    cell = v * w + cell;
}

using RingVisitor = void (*)(SimData &, const Region &, const RadiationEmitterData &, int, int, int, int, float,
                             float *);

// FUN_001657c0: anti-aliased ellipse outline with radii (a, b) around (cx, cy). Each step visits
// the four quadrants twice, splitting `intensity` between the two cells the curve passes between.
void EllipseRing(float intensity, SimData &sim, const Region &r, const RadiationEmitterData &d, int cx, int cy, int a,
                 int b, float *out, RingVisitor visit) {
    if (!(0 < a && 0 < b)) {
        return;
    }
    float a2 = static_cast<float>(a * a);
    float n = static_cast<float>(a * a + b * b);
    float y0 = ApproxRsqrt(n);
    float newton = (n * y0 * y0 + -3.0f) * (y0 * -0.5f);
    float limit = RoundSmall(newton * a2);
    if (0.0f <= limit) {
        float inv_a2 = 1.0f / a2;
        float i = 0.0f;
        float e = 1.0f;
        for (;;) {
            e = e * static_cast<float>(b);
            float w1 = RoundSmall((e - FloorSmall(e)) * intensity);
            float w2 = intensity - w1;
            int fy = FloorInt(e);
            int ix = static_cast<int>(i);
            visit(sim, r, d, cx, cy, ix + cx, fy + cy, w1, out);
            visit(sim, r, d, cx, cy, cx - ix, fy + cy, w1, out);
            visit(sim, r, d, cx, cy, ix + cx, cy - fy, w1, out);
            visit(sim, r, d, cx, cy, cx - ix, cy - fy, w1, out);
            visit(sim, r, d, cx, cy, ix + cx, fy - 1 + cy, w2, out);
            visit(sim, r, d, cx, cy, cx - ix, fy - 1 + cy, w2, out);
            visit(sim, r, d, cx, cy, ix + cx, cy - (fy - 1), w2, out);
            visit(sim, r, d, cx, cy, cx - ix, cy - (fy - 1), w2, out);
            float next = i + 1.0f;
            if (next > limit) {
                break;
            }
            i = next;
            e = std::sqrt(1.0f - (i * i) * inv_a2);
        }
    }
    float b2 = static_cast<float>(b * b);
    float limit_y = RoundSmall(newton * b2);
    if (!(0.0f <= limit_y)) {
        return;
    }
    float inv_b2 = 1.0f / b2;
    float i = 0.0f;
    float e = 1.0f;
    for (;;) {
        e = e * static_cast<float>(a);
        float w1 = RoundSmall((e - FloorSmall(e)) * intensity);
        float w2 = intensity - w1;
        int fx = FloorInt(e);
        int iy = static_cast<int>(i);
        visit(sim, r, d, cx, cy, fx + cx, iy + cy, w1, out);
        visit(sim, r, d, cx, cy, cx - fx, iy + cy, w1, out);
        visit(sim, r, d, cx, cy, fx + cx, cy - iy, w1, out);
        visit(sim, r, d, cx, cy, cx - fx, cy - iy, w1, out);
        visit(sim, r, d, cx, cy, fx - 1 + cx, iy + cy, w2, out);
        visit(sim, r, d, cx, cy, cx - (fx - 1), iy + cy, w2, out);
        visit(sim, r, d, cx, cy, fx - 1 + cx, cy - iy, w2, out);
        visit(sim, r, d, cx, cy, cx - (fx - 1), cy - iy, w2, out);
        float next = i + 1.0f;
        if (next > limit_y) {
            return;
        }
        i = next;
        e = std::sqrt(1.0f - (i * i) * inv_b2);
    }
}

int CeilRadius(float r) {
    int i = static_cast<int>(r);
    if (static_cast<float>(i) < r) {
        i++;
    }
    return i;
}

// FUN_00166780: one ring of an expanding pulse (types 1 and 2), with line of sight.
void PulseWithSight(SimData &sim, const Region &r, const RadiationEmitterData &d, int steps) {
    int cell = d.cell;
    float fraction = (static_cast<float>(d.step) + 1.0f) / static_cast<float>(steps);
    int a = CeilRadius(static_cast<float>(d.radiusX) * fraction);
    float fy = fraction * static_cast<float>(d.radiusY);
    int b = static_cast<int>(fy);
    float intensity = d.intensity;
    if (static_cast<float>(b) < fy) {
        b++;
    }
    if (d.type == 2) {
        int area = (a - 1) * (b - 1);
        if (area < 1) {
            area = 1;
        }
        intensity = intensity / static_cast<float>(area);
    }
    if (d.step != 0) {
        if (a < 2 && b < 2) {
            return;
        }
        EllipseRing(intensity, sim, r, d, cell % sim.width, cell / sim.width, a, b, nullptr, RingAddWithSight);
        return;
    }
    float &own = sim.updatedCells->unk_0xf0[static_cast<size_t>(cell)];
    own = intensity + own;
}

}  // namespace

void RadiationEmitter::Update(float dt, SimData &sim, const Region &region) {
    if (!sim.radiationEnabled) {
        return;
    }
    for (size_t i = 0; i < data.size(); i++) {
        RadiationEmitterData &d = data[i];
        int cy = d.cell / sim.width;
        int cx = d.cell % sim.width;
        if (!(region.minX <= cx && region.minY <= cy && cx <= region.maxX && cy <= region.maxY &&
              0.0f < d.intensity && (d.radiusX != 0 || d.radiusY != 0))) {
            continue;
        }
        float t = dt + d.timer;
        float per_step = d.pulseDuration;
        d.timer = t;
        int steps;
        int repeats;
        float pulse_timer;
        if (d.pulseInterval == 0.0f) {
            d.timer = 0.0f;
            steps = d.radiusY <= d.radiusX ? d.radiusX : d.radiusY;
            d.step = 0;
            per_step = per_step / static_cast<float>(steps);
            repeats = 1;
        } else {
            if (d.pulseInterval <= t) {
                d.timer = 0.0f;
                d.step = 0;
            } else if (per_step < t) {
                continue;
            }
            steps = d.radiusY <= d.radiusX ? d.radiusX : d.radiusY;
            per_step = per_step / static_cast<float>(steps);
            if (dt / per_step <= 1.0f && (per_step < d.pulseTimer || per_step == d.pulseTimer)) {
                repeats = 1;
            } else {
                repeats = static_cast<int>(dt / per_step);
            }
            if (repeats <= 0) {
                d.pulseTimer = d.pulseTimer + dt;
                continue;
            }
        }
        for (int k = 0; k < repeats;) {
            switch (d.type) {
            case 0: {
                // Continuous: every cell of the ellipse, through the line of sight.
                int ecy = d.cell / sim.width;
                int ecx = d.cell % sim.width;
                int rx = d.radiusX, ry = d.radiusY;
                float inv_rx2 = 1.0f / static_cast<float>(rx * rx);
                float inv_ry2 = 1.0f / static_cast<float>(ry * ry);
                float inv_rx = 1.0f / static_cast<float>(rx);
                float inv_ry = 1.0f / static_cast<float>(ry);
                for (int oy = -ry, y = ecy - ry;; oy++, y++) {
                    float dyf = static_cast<float>(oy);
                    for (int x = ecx - rx;; x++) {
                        if (InGridAndRegion(sim, region, x, y)) {
                            float dxf = static_cast<float>(x - ecx);
                            bool visible = true;
                            if (d.angle != 360.0f && (ecx != x || ecy != y)) {
                                visible = InCone(d, dyf, dxf);
                            }
                            if (visible && dxf * dxf * inv_rx2 + dyf * dyf * inv_ry2 <= 1.0f) {
                                float falloff = (1.0f - AbsSS(dxf) * inv_rx) * (1.0f - inv_ry * AbsSS(dyf));
                                float scale = (falloff * d.intensity) / sim.unk_0x71c;
                                float sight = LineOfSight(sim, ecx, ecy, x, y);
                                float v = MaxSS(MinSS(1.0f, sight), 0.0f) * scale;
                                float noise = 0.0f;
                                if (0.25f > falloff) {
                                    sim.rngState = sim.rngState * 0x343fdu + 0x269ec3u;
                                    float roll = static_cast<float>(static_cast<int>((sim.rngState >> 16) & 0x7fff));
                                    noise = (roll * 0x1.0002p-17f - 0.125f) * v;
                                }
                                float &cell = CellRadiation(sim, x, y);
                                cell = (v + cell) + noise;
                            }
                        }
                        if (x == rx * 2 + (ecx - rx)) {
                            break;
                        }
                    }
                    // The original ends the rows at 2 * radiusX (not radiusY) past the first row.
                    if (rx * 2 + (ecy - ry) == y) {
                        break;
                    }
                }
                break;
            }
            case 1:
            case 2:
                PulseWithSight(sim, region, d, steps);
                break;
            case 3:
            case 5: {
                float fraction = (static_cast<float>(d.step) + 1.0f) * (1.0f / static_cast<float>(steps));
                int a = CeilRadius(static_cast<float>(d.radiusX) * fraction);
                int b = CeilRadius(fraction * static_cast<float>(d.radiusY));
                if (a < 1) {
                    a = 1;
                }
                if (b < 1) {
                    b = 1;
                }
                if (d.type == 3) {
                    EllipseRing(d.intensity, sim, region, d, d.cell % sim.width, d.cell / sim.width, a, b, nullptr,
                                RingAdd);
                } else {
                    float absorbed = 0.0f;
                    EllipseRing(d.intensity, sim, region, d, d.cell % sim.width, d.cell / sim.width, a, b, &absorbed,
                                RingAbsorb);
                    if (0.0f <= absorbed) {
                        float &own = sim.updatedCells->unk_0xf0[static_cast<size_t>(d.cell)];
                        own = absorbed + own;
                    }
                }
                break;
            }
            default:
                break;
            }
            pulse_timer = d.pulseTimer - per_step;
            if (static_cast<int>(static_cast<unsigned>(d.step) + 1u) == steps) {
                d.step = 0;
            } else {
                d.step = d.step + 1;
            }
            d.pulseTimer = pulse_timer;
            k++;
        }
        d.pulseTimer = d.pulseTimer + dt;
    }
}
