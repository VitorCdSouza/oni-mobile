// Property textures the game's shaders read, rebuilt from the sim grid every frame.
#include <math.h>

#include <cstring>
#include <vector>

#include "sim_common.h"

uint16_t GetElementIndex(uint32_t hash);

// Per game cell: the flow of the cell (x: first minus second component, y: fourth minus third),
// divided by the mass above 1 kg, and zero when the cell changed element this tick.
void UpdateFlowTexture(const SimData &sim, GameData &game) {
    const int w = sim.width;
    float *out = static_cast<float *>(game.propertyTextureFlow);
    const CellSOA &upd = *sim.updatedCells;
    const uint16_t *before = sim.cells->elementIdx.data();
    for (int y = 0; y < game.height; y++) {
        int first = (y + 1) * w + 1;
        for (int c = first; c < first + game.width; c++) {
            float scale = 0.0f;
            if (upd.elementIdx[c] == before[c]) {
                scale = 1.0f;
                float mass = upd.mass[c];
                if (mass > 1.0f) {
                    scale = 1.0f / mass;
                }
            }
            const Vector4f &f = sim.unk_0x60[c];
            out[0] = (f.x - f.y) * scale;
            out[1] = (f.w - f.z) * scale;
            out += 2;
        }
    }
}

// Per game cell, how much sunlight reaches it (0..255): each world's top row gets all of it,
// and every cell below gets what the cell above let through.
void UpdateExposedToSunPropertyTexture(const SimData &sim, GameData &game) {
    uint8_t *out = game.propertyTextureExposedToSunlight;
    const CellSOA &upd = *sim.updatedCells;
    for (const WorldRect &world : sim.worldRects) {
        const int w = sim.width;
        const int columns = world.width;
        if (columns > 0) {
            std::memset(out + ToGameCell(sim, (world.y + world.height) * w + world.x + 1), 0xff,
                        static_cast<size_t>(columns));
        }
        std::vector<float> light(columns > 0 ? static_cast<size_t>(columns) : 0, 1.0f);
        for (int row = world.height - 2; row >= 0; row--) {
            int first = world.x + (row + world.y + 1) * w + 1;
            for (int i = 0; i < columns; i++) {
                int c = first + i;
                int above = c + w;
                uint8_t props = upd.properties[c];
                float through;
                if ((props & 0x30) == 0x30) {
                    const ElementLightAbsorptionData &a = gElementLightAbsorptionData[GetElementIndex(0x2531469c)];
                    float amount = MinSS(upd.mass[above] * a.inverseDefaultMass, 1.0f);
                    through = MaxSS(light[i] - a.lightAbsorptionFactor * amount, 0.0f);
                    light[i] = through;
                } else {
                    const ElementLightAbsorptionData &a = gElementLightAbsorptionData[upd.elementIdx[above]];
                    float amount = MinSS(upd.mass[above] * a.inverseDefaultMass, 1.0f);
                    through = MaxSS(light[i] - a.lightAbsorptionFactor * amount, 0.0f);
                    light[i] = (upd.properties[c] & 0x20) != 0 ? 0.0f : through;
                }
                out[ToGameCell(sim, c)] = static_cast<uint8_t>(static_cast<int>(through * 255.0f));
            }
        }
    }
}

int LerpInt(int a, int b, float t) {
    return a + static_cast<int>(static_cast<float>(b - a) * t);
}

// Per channel, alpha in the top byte (shifted as a signed int, like the original).
uint32_t LerpColor(int a, int b, float t) {
    int alpha = LerpInt(a >> 24, b >> 24, t);
    int red = LerpInt((a >> 16) & 0xff, (b >> 16) & 0xff, t);
    int green = LerpInt((static_cast<uint32_t>(a) >> 8) & 0xff, (static_cast<uint32_t>(b) >> 8) & 0xff, t);
    int blue = LerpInt(a & 0xff, b & 0xff, t);
    return static_cast<uint32_t>(green << 8 | alpha << 24 | red << 16 | blue);
}

uint32_t AddMaterialPropertyToEncodedColor(uint32_t color, uint32_t properties, uint32_t bit) {
    return ((1u << (bit & 0x1f)) & properties) | color;
}

// How far down a liquid column the cell is, 0 at the surface: the liquid's fill fraction (mass
// over max_mass, gamma 0.45) dimmed by the sunlight reaching the cell; with `follow`, never more
// than the same value for the cell above. Non-liquid cells: 1 for solids, 0 otherwise.
float GetEstimatedLiquidGradientValue(int cell, const float *mass, const uint16_t *elements, uint8_t *exposed,
                                      const SimData &sim, float max_mass, bool follow) {
    uint8_t state = gElementPropertyTextureData[elements[cell]].state;
    if (StateOf(state) != kLiquid) {
        return (state & 2) != 0 ? 1.0f : 0.0f;
    }
    int above = sim.width + cell;
    if ((gElementPropertyTextureData[elements[above]].state & 2) == 0) {
        return 0.0f;
    }
    unsigned long u = static_cast<unsigned long>(cell);
    unsigned long w = static_cast<unsigned long>(static_cast<long>(sim.width));
    uint8_t sun = exposed[(static_cast<int>(u / w) - 1) * (sim.width - 2) - 1 + static_cast<int>(u % w)];
    float shade = 1.0f - static_cast<float>(sun) * 0x1.010102p-8f;
    float value = 0.0f;
    if (sun != 0xff) {
        float fill = MinSS(mass[cell] / max_mass, 1.0f);
        value = shade * powf(fill, 0x1.ccccccp-2f);
        if (value > 0.0f && follow) {
            float next = GetEstimatedLiquidGradientValue(above, mass, elements, exposed, sim, max_mass, false);
            return MinSS(next, value);
        }
    }
    return MinSS(1.0f, value);
}

// Liquid rendering data per game cell (the last game row is left as it was, like the original):
// - liquid: alpha from the fill level at the surface, rgb from the element's colour gradient;
// - liquid data: alpha how close the liquid is to a state change, rgb the colour of the element
//   it would turn into;
// - material data: the element's material property bits.
void UpdateLiquidPropertyTexture(const SimData &sim, GameData &game) {
    uint32_t *liquid = static_cast<uint32_t *>(game.propertyTextureLiquid);
    uint32_t *liquid_data = static_cast<uint32_t *>(game.propertyTextureLiquidData);
    uint32_t *material = static_cast<uint32_t *>(game.propertyTextureMaterialData);
    uint8_t *exposed = game.propertyTextureExposedToSunlight;
    const uint16_t *elements = sim.updatedCells->elementIdx.data();
    const float *mass = sim.updatedCells->mass.data();
    const float *temperature = sim.updatedCells->temperature.data();
    const int gw = game.width;
    for (int y = 1; y < game.height; y++) {
        int first = y * sim.width + 1;
        size_t out = static_cast<size_t>(y - 1) * static_cast<size_t>(gw);
        for (int c = first; c < first + gw; c++, out++) {
            uint16_t e = elements[c];
            const ElementPropertyTextureData &tex = gElementPropertyTextureData[e];
            if (StateOf(tex.state) != kLiquid) {
                liquid[out] = 0;
                liquid_data[out] = 0;
                material[out] = 0;
                continue;
            }
            int above = c + sim.width;
            uint32_t alpha = 0xff000000u;
            if (((sim.cells->properties[above] | gElementPropertyTextureData[elements[above]].state) & 2) == 0 &&
                !(mass[c] > 0x1.f3fffep+9f)) {
                float level = powf(mass[c] * 0.001f, 0x1.ccccccp-2f);
                alpha = static_cast<uint32_t>(static_cast<int64_t>(level * 255.0f)) << 24;
            }
            float gradient = GetEstimatedLiquidGradientValue(c, mass, elements, exposed, sim, 1000.0f, true);
            uint32_t colour = tex.colour;
            for (int i = 0; i < tex.numberOfGradientColors; i++) {
                uint32_t from = tex.gradientColours[i];
                float stop = static_cast<float>(from >> 24) * 0x1.010102p-8f;
                if (!(gradient >= stop)) {
                    continue;
                }
                uint32_t to;
                float span;
                if (i < tex.numberOfGradientColors - 1) {
                    to = tex.gradientColours[i + 1];
                    span = static_cast<float>(to >> 24) * 0x1.010102p-8f - stop;
                } else {
                    to = from;
                    span = 0.0f;
                }
                float t = MaxSS(MinSS(1.0f, (gradient - stop) / span), 0.0f);
                colour = LerpColor(static_cast<int>(from), static_cast<int>(to), t);
            }
            liquid[out] = alpha | (colour & 0xffffff);

            const ElementTemperatureData &td = gElementTemperatureData[e];
            float closeness = (3.0f + temperature[c] - td.lowTemp) / (6.0f + td.highTemp - td.lowTemp);
            uint32_t data_alpha;
            uint16_t target;
            if (0.0f > closeness) {
                data_alpha = 0;
                target = td.lowTempTransitionIdx;
            } else if (closeness > 1.0f) {
                data_alpha = 0xff000000u;
                target = td.highTempTransitionIdx;
            } else {
                data_alpha = static_cast<uint32_t>(static_cast<int64_t>(closeness * 255.0f)) << 24;
                target = !(closeness >= 0.5f) ? td.lowTempTransitionIdx : td.highTempTransitionIdx;
            }
            uint32_t target_colour = target == kInvalidElement ? 0xffffffu
                                                               : gElementPropertyTextureData[target].colour & 0xffffff;
            liquid_data[out] = data_alpha | target_colour;

            uint32_t bits = 0;
            for (uint32_t bit = 0; bit < 4; bit++) {
                bits = AddMaterialPropertyToEncodedColor(bits, tex.materialProperties, bit);
            }
            material[out] = (tex.materialProperties & 0xff000000u) | bits;
        }
    }
}
