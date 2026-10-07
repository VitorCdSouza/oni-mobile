// Generated from sim/layout/layout.txt. Do not edit.
// Included by the port in hybrid builds, after its own struct definitions.
#pragma once
#include <cstddef>

static_assert(sizeof(CellSOA) == 0x108, "CellSOA size");
static_assert(offsetof(CellSOA, elementIdx) == 0x0, "CellSOA::elementIdx");
static_assert(offsetof(CellSOA, temperature) == 0x18, "CellSOA::temperature");
static_assert(offsetof(CellSOA, mass) == 0x30, "CellSOA::mass");
static_assert(offsetof(CellSOA, properties) == 0x48, "CellSOA::properties");
static_assert(offsetof(CellSOA, insulation) == 0x60, "CellSOA::insulation");
static_assert(offsetof(CellSOA, strengthInfo) == 0x78, "CellSOA::strengthInfo");
static_assert(offsetof(CellSOA, diseaseIdx) == 0x90, "CellSOA::diseaseIdx");
static_assert(offsetof(CellSOA, diseaseCount) == 0xa8, "CellSOA::diseaseCount");

static_assert(sizeof(SimData) == 0x748, "SimData size");
static_assert(offsetof(SimData, width) == 0x0, "SimData::width");
static_assert(offsetof(SimData, height) == 0x4, "SimData::height");
static_assert(offsetof(SimData, numGameCells) == 0x8, "SimData::numGameCells");
static_assert(offsetof(SimData, cells) == 0x20, "SimData::cells");
static_assert(offsetof(SimData, updatedCells) == 0x28, "SimData::updatedCells");
static_assert(offsetof(SimData, events) == 0x98, "SimData::events");
static_assert(offsetof(SimData, components) == 0xb0, "SimData::components");
static_assert(offsetof(SimData, headless) == 0x719, "SimData::headless");
static_assert(offsetof(SimData, radiationEnabled) == 0x71a, "SimData::radiationEnabled");

static_assert(sizeof(SimEvents) == 0x1e0, "SimEvents size");

