// CellModifications.cpp: the game's ModifyCell messages (add, remove or replace a cell's
// contents), applied by ProcessCellModifications.
#include "sim_common.h"

void AddMassAndUpdateTemperature(CellSOA *src, CellSOA *dst, int cell, float mass, float temperature,
                                 unsigned char disease_idx, int disease_count);

namespace AddSolidMassSubType {
enum Type : int32_t {};
}

namespace {

constexpr int32_t kUnobtaniumLikeId = static_cast<int32_t>(0xa9360b34u);  // never displaced or replaced

const char kCellAssert[] =
    "Assert failed: (cell.mass() == 0.0f || cell.temperature() > 0.0f) && cell.temperature() <= SIM_MAX_TEMPERATURE";

uint8_t StateOfElement(uint16_t element) {
    return gElements[element].state & 3;
}

void MarkChanged(SimData &sim, int cell) {
    int game_cell = ToGameCell(sim, cell);
    if (IsValidGameCell(sim, game_cell)) {
        PushSubstanceChange(*sim.events, game_cell);
    }
    sim.unk_0x68[cell] |= 0x1f;
}

void CheckCell(const CellSOA &cells, int cell, int line) {
    float t = cells.temperature[cell];
    if ((IsNonZero(cells.mass[cell]) && t <= 0.0f) || t > 10000.0f) {
        AssertFunc("BREAKPT:", line, "CellModifications.cpp");
        KCrashReporterReportMessage("CellModifications.cpp", line, kCellAssert);
    }
}

void SetDisease(CellSOA &cells, int cell, uint8_t disease_idx, int disease_count) {
    cells.diseaseIdx[cell] = disease_idx;
    cells.diseaseCount[cell] = disease_count;
    cells.diseaseInfestationTickCount[cell] = 0;
    cells.diseaseAccumulatedError[cell] = 0.0f;
}

// Removing mass of one state; an emptied cell becomes vacuum.
void RemoveMass(int cell, float amount, SimData &sim, uint8_t state) {
    CellAccessor at;
    at.soa = sim.updatedCells;
    at.cell = cell;
    if (StateOfElement(at.soa->elementIdx[cell]) != state) {
        return;
    }
    float &mass = at.soa->mass[cell];
    float left = amount + mass;
    if (0.0f < left) {
        mass = left;
        if (0x1p-126f < left) {
            return;
        }
    } else {
        mass = 0.0f;
    }
    sim.ClearCell(at);
    MarkChanged(sim, cell);
}

// When nothing could make room: the new mass eats into the cell's current mass, or replaces it.
// Returns false when the current mass absorbed it all.
bool Overwrite(CellSOA &cells, int cell, uint16_t element, float mass, float temperature, uint8_t disease_idx,
               int disease_count) {
    float current = cells.mass[cell];
    if (!(mass >= current)) {
        cells.mass[cell] = current - mass;
        return false;
    }
    float rest = mass - current;
    cells.elementIdx[cell] = element;
    cells.mass[cell] = 0.0f;
    cells.temperature[cell] = 0.0f;
    AddMassAndUpdateTemperature(&cells, &cells, cell, rest, temperature, disease_idx,
                                TruncToInt(rest / mass * static_cast<float>(disease_count)));
    return true;
}

}  // namespace

void RemoveGas(int cell, float amount, SimData &sim) {
    RemoveMass(cell, amount, sim, kGas);
}

void RemoveLiquid(int cell, float amount, SimData &sim) {
    RemoveMass(cell, amount, sim, kLiquid);
}

void RemoveSolid(int cell, float amount, SimData &sim) {
    RemoveMass(cell, amount, sim, kSolid);
}

void ReplaceElement(int cell, unsigned short element, float mass, float temperature, unsigned char disease_idx,
                    int disease_count, SimData &sim) {
    CellSOA &cells = *sim.updatedCells;
    cells.elementIdx[cell] = element;
    if (StateOfElement(element) == kVacuum) {
        cells.temperature[cell] = 0.0f;
        cells.mass[cell] = 0.0f;
    } else {
        cells.temperature[cell] = temperature;
        cells.mass[cell] = mass;
        CheckCell(cells, cell, 0xeb);
    }
    MarkChanged(sim, cell);
    SetDisease(cells, cell, disease_idx, disease_count);
}

void ReplaceAndDisplaceElement(int cell, unsigned short element, float mass, float temperature,
                               unsigned char disease_idx, int disease_count, SimData &sim) {
    CellSOA &cells = *sim.updatedCells;
    uint16_t current = cells.elementIdx[cell];
    uint8_t state = StateOfElement(current);
    if (state == kGas) {
        DisplaceGas(sim, *sim.events, &cells, cell, current);
    } else if (state == kLiquid) {
        DisplaceLiquid(sim, *sim.events, &cells, cell, current);
    }
    cells.elementIdx[cell] = element;
    bool vacuum = StateOfElement(element) == kVacuum;
    bool report;
    if (vacuum) {
        cells.mass[cell] = 0.0f;
        report = temperature > 10000.0f;
    } else {
        cells.mass[cell] = mass;
        report = (mass > 0.0f && !(0.0f < temperature)) || temperature > 10000.0f;
    }
    if (report) {
        KCrashReporterReportMessage("CellModifications.cpp", 0x10e,
                                    "ReplaceAndDisplace resulted in cell with T=%f, M=%f. Clamping temperature.",
                                    static_cast<double>(temperature), static_cast<double>(mass));
    }
    temperature = vacuum ? 0.0f : MaxSS(MinSS(temperature, 10000.0f), 0.0f);
    cells.temperature[cell] = temperature;
    MarkChanged(sim, cell);
    SetDisease(cells, cell, disease_idx, disease_count);
}

void AddGas(int cell, unsigned short element, float mass, float temperature, unsigned char disease_idx,
            int disease_count, SimData &sim) {
    CellSOA *cells = sim.updatedCells;
    uint16_t current = cells->elementIdx[cell];
    if (current == element) {
        AddMassAndUpdateTemperature(cells, cells, cell, mass, temperature, disease_idx, disease_count);
        return;
    }
    uint8_t state = StateOfElement(current);
    bool replace = state == kVacuum;
    if (state == kGas) {
        replace = DisplaceGas(sim, *sim.events, cells, cell, current);
    } else if (state == kLiquid) {
        replace = DisplaceLiquid(sim, *sim.events, cells, cell, current);
    }
    if (replace) {
        cells->elementIdx[cell] = element;
        AddMassAndUpdateTemperature(cells, cells, cell, mass, temperature, disease_idx, disease_count);
        MarkChanged(sim, cell);
        float t = cells->temperature[cell];
        if (!((cells->mass[cell] == 0.0f || 0.0f < t) && t <= 10000.0f)) {
            AssertFunc("BREAKPT:", 0x8c, "CellModifications.cpp");
            KCrashReporterReportMessage("CellModifications.cpp", 0x8c, kCellAssert);
        }
        return;
    }
    // A solid cell, or one that could not be emptied: a vacuum neighbour takes the gas, or one
    // with the same gas gets the mass (left, right, above).
    uint16_t vacuum_element = sim.vacuumElementIdx;
    for (int n : {cell - 1, cell + 1, sim.width + cell}) {
        if (cells->elementIdx[n] == vacuum_element) {
            cells->elementIdx[n] = element;
            cells->mass[n] = mass;
            cells->temperature[n] = temperature;
            cells->diseaseIdx[n] = disease_idx;
            cells->diseaseCount[n] = disease_count;
            MarkChanged(sim, n);
            return;
        }
        if (cells->elementIdx[n] == element) {
            AddMassAndUpdateTemperature(cells, cells, n, mass, temperature, disease_idx, disease_count);
            return;
        }
    }
    if (!Overwrite(*cells, cell, element, mass, temperature, disease_idx, disease_count)) {
        return;
    }
    float m = cells->mass[cell];
    float t = cells->temperature[cell];
    bool bad = m == 0.0f ? t > 10000.0f : (0.0f >= t || t > 10000.0f);
    if (bad) {
        AssertFunc("BREAKPT:", 0xbe, "CellModifications.cpp");
        KCrashReporterReportMessage("CellModifications.cpp", 0xbe, kCellAssert);
        m = cells->mass[cell];
    }
    if (!(0x1p-126f < m)) {
        CellAccessor at;
        at.soa = cells;
        at.cell = cell;
        sim.ClearCell(at);
    }
    MarkChanged(sim, cell);
}

void AddLiquid(int cell, unsigned short element, float mass, float temperature, unsigned char disease_idx,
               int disease_count, SimData &sim) {
    CellSOA *cells = sim.updatedCells;
    uint16_t *elements = cells->elementIdx.data();
    if (elements[cell] == element) {
        AddMassAndUpdateTemperature(cells, cells, cell, mass, temperature, disease_idx, disease_count);
        return;
    }
    // A solid cell passes the liquid to its first non-solid neighbour (left, right, below, above).
    int target = cell;
    if (StateOfElement(elements[cell]) == kSolid) {
        int found = -1;
        for (int n : {cell - 1, cell + 1, cell - sim.width, sim.width + cell}) {
            if (StateOfElement(elements[n]) != kSolid) {
                found = n;
                break;
            }
        }
        if (found < 0) {
            if (Overwrite(*cells, cell, element, mass, temperature, disease_idx, disease_count)) {
                if (cells->mass[cell] <= 0x1p-126f) {
                    cells->elementIdx[cell] = sim.vacuumElementIdx;
                }
                MarkChanged(sim, cell);
            }
            return;
        }
        target = found;
    }
    const Element &e = gElements[elements[target]];
    uint8_t state = e.state & 3;
    if (e.id != kUnobtaniumLikeId) {
        bool place = state == kVacuum;
        if (state == kGas) {
            place = DisplaceGas(sim, *sim.events, cells, target, elements[target]);
            if (!place) {
                // A liquid right above swaps places with the gas, so the new liquid lands below.
                CellSOA &c = *sim.updatedCells;
                int above = target + sim.width;
                uint16_t above_element = c.elementIdx[above];
                if (StateOfElement(above_element) == kLiquid) {
                    c.elementIdx[above] = c.elementIdx[target];
                    c.elementIdx[target] = above_element;
                    std::swap(c.mass[above], c.mass[target]);
                    std::swap(c.temperature[above], c.temperature[target]);
                    std::swap(c.diseaseIdx[above], c.diseaseIdx[target]);
                    std::swap(c.diseaseCount[above], c.diseaseCount[target]);
                    std::swap(c.diseaseInfestationTickCount[above], c.diseaseInfestationTickCount[target]);
                    std::swap(c.diseaseAccumulatedError[above], c.diseaseAccumulatedError[target]);
                    MarkChanged(sim, above);
                    MarkChanged(sim, target);
                }
            }
        } else if (state != kVacuum) {
            place = DisplaceLiquid(sim, *sim.events, cells, target, elements[target]);
        }
        if (place) {
            cells = sim.updatedCells;
            cells->elementIdx[target] = element;
            AddMassAndUpdateTemperature(cells, cells, target, mass, temperature, disease_idx, disease_count);
            MarkChanged(sim, target);
            return;
        }
        // A neighbour with the same liquid takes the mass (right, left, above, below).
        CellSOA *now = sim.updatedCells;
        const uint16_t *el = now->elementIdx.data();
        for (int n : {target + 1, target - 1, target + sim.width, target - sim.width}) {
            if (el[n] == element) {
                AddMassAndUpdateTemperature(now, now, n, mass, temperature, disease_idx, disease_count);
                return;
            }
        }
    }
    if (Overwrite(*cells, target, element, mass, temperature, disease_idx, disease_count)) {
        if (cells->mass[target] <= 0x1p-126f) {
            cells->elementIdx[target] = sim.vacuumElementIdx;
        }
        MarkChanged(sim, target);
    }
}

void AddSolid(int cell, unsigned short element, float mass, float temperature, unsigned char disease_idx,
              int disease_count, AddSolidMassSubType::Type sub_type, SimData &sim) {
    if (sub_type == 1) {
        // In place: whatever gas or liquid is there moves away; a different solid blocks it.
        CellSOA *cells = sim.updatedCells;
        CellSOA *target = cells;
        uint16_t current = cells->elementIdx[cell];
        uint8_t state = StateOfElement(current);
        if (state == kSolid) {
            if (current != element) {
                return;
            }
        } else if (state == kGas) {
            DisplaceGas(sim, *sim.events, cells, cell, current);
            target = sim.updatedCells;
        } else if (state == kLiquid) {
            DisplaceLiquid(sim, *sim.events, cells, cell, current);
            target = sim.updatedCells;
        }
        AddMassAndUpdateTemperature(target, target, cell, mass, temperature, disease_idx, disease_count);
        CheckCell(*cells, cell, 0x198);
        MarkChanged(sim, cell);
        return;
    }
    if (sub_type != 0) {
        return;
    }
    // Falling solid: top up the same solid below and in the cell (up to maxMass), then the rest
    // lands on the first non-solid cell going up.
    CellSOA *cells = sim.updatedCells;
    int w = sim.width;
    int y = cell / w;
    int x = cell % w;
    float max_mass = gElementPostProcessData[element].maxMass;
    int below = (y - 1) * w + x;
    if (cells->elementIdx[below] == element) {
        float room = max_mass - cells->mass[below];
        if (room > 0.0f) {
            float add = MinSS(room, mass);
            AddMassAndUpdateTemperature(cells, cells, below, add, temperature, disease_idx,
                                        TruncToInt(static_cast<float>(disease_count) * (add / mass)));
            cells = sim.updatedCells;
            mass = mass - add;
        }
    }
    int here = sim.width * y + x;
    if (cells->elementIdx[here] == element) {
        float room = max_mass - cells->mass[here];
        if (room > 0.0f) {
            float add = MinSS(room, mass);
            AddMassAndUpdateTemperature(cells, cells, here, add, temperature, disease_idx,
                                        TruncToInt(static_cast<float>(disease_count) * (add / mass)));
            mass = mass - add;
        }
    }
    if (mass <= 0.0f) {
        return;
    }
    int top = sim.height - 1;
    if (y >= top) {
        return;
    }
    cells = sim.updatedCells;
    w = sim.width;
    int c = x + y * w;
    while (StateOfElement(cells->elementIdx[c]) == kSolid) {
        y++;
        c += w;
        if (y == top) {
            return;
        }
    }
    const Element &e = gElements[cells->elementIdx[c]];
    if (e.id == kUnobtaniumLikeId) {
        return;
    }
    uint8_t state = e.state & 3;
    if (state == kGas) {
        DisplaceGas(sim, *sim.events, cells, c, cells->elementIdx[c]);
    } else if (state != kVacuum) {
        DisplaceLiquid(sim, *sim.events, cells, c, cells->elementIdx[c]);
    }
    cells->elementIdx[c] = element;
    cells->mass[c] = mass;
    cells->temperature[c] = temperature;
    if ((IsNonZero(cells->mass[c]) && temperature <= 0.0f) || temperature > 10000.0f) {
        AssertFunc("BREAKPT:", 0x170, "CellModifications.cpp");
        KCrashReporterReportMessage("CellModifications.cpp", 0x170, kCellAssert);
    }
    MarkChanged(sim, c);
    SetDisease(*cells, c, disease_idx, disease_count);
}

void ProcessCellModifications(SimFrameInfo &frame, SimData &sim) {
    for (const CellModification &m : frame.cellModifications) {
        int w = sim.width - 2;
        int c = m.cellIdx % w + 1 + (m.cellIdx / w + 1) * sim.width;
        float mass = m.mass;
        float temperature = m.temperature;
        bool report = false;
        bool clamp = false;
        if (mass > 0.0f && 0.0f >= temperature) {
            report = true;
            clamp = true;
        } else if (temperature > 10000.0f) {
            report = true;
            if (0.0f < mass) {
                clamp = true;
            } else {
                mass = 0.0f;
                temperature = 0.0f;
            }
        }
        if (report) {
            KCrashReporterReportMessage(
                "CellModifications.cpp", 0x1f,
                "CLAMPING TEMPERATURE: Invalid cell modification msg: GameCell=%d T=%f M=%f EIdx=%d, DIdx=%d, DC=%d",
                m.cellIdx, static_cast<double>(m.temperature), static_cast<double>(m.mass),
                static_cast<int>(m.elementIdx), static_cast<int>(m.diseaseIdx), m.diseaseCount);
        }
        if (clamp) {
            temperature = MaxSS(MinSS(temperature, 10000.0f), 0.0f);
        }
        if (m.replaceType == 1) {
            ReplaceElement(c, m.elementIdx, mass, temperature, m.diseaseIdx, m.diseaseCount, sim);
        } else if (m.replaceType == 2) {
            ReplaceAndDisplaceElement(c, m.elementIdx, mass, temperature, m.diseaseIdx, m.diseaseCount, sim);
        } else if (m.replaceType == 0) {
            uint8_t state = StateOfElement(m.elementIdx);
            if (state == kGas) {
                if (mass > 0.0f) {
                    AddGas(c, m.elementIdx, mass, temperature, m.diseaseIdx, m.diseaseCount, sim);
                } else {
                    RemoveGas(c, mass, sim);
                }
            } else if (state == kSolid) {
                if (mass > 0.0f) {
                    AddSolid(c, m.elementIdx, mass, temperature, m.diseaseIdx, m.diseaseCount,
                             static_cast<AddSolidMassSubType::Type>(m.addSubType), sim);
                } else if (0.0f > mass) {
                    RemoveSolid(c, mass, sim);
                }
            } else if (state == kLiquid) {
                if (mass > 0.0f) {
                    AddLiquid(c, m.elementIdx, mass, temperature, m.diseaseIdx, m.diseaseCount, sim);
                } else if (0.0f > mass) {
                    RemoveLiquid(c, mass, sim);
                }
            } else {
                AssertFunc("BREAKPT:", 0x59, "CellModifications.cpp");
                KCrashReporterReportMessage("CellModifications.cpp", 0x59, "Invalid replacement type");
            }
        }
        if (m.callbackIdx != -1) {
            sim.events->callbacks.push_back(CallbackInfo{m.callbackIdx});
        }
    }
    frame.cellModifications.clear();
}
