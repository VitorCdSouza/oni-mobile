// SimEvents helpers (inline in the original, exported as weak symbols).
#include "sim_common.h"

void SimEvents::ChangeSubstance(const SimData &sim, unsigned long cell) {
    int game_cell = ToGameCell(sim, static_cast<int>(cell));
    if (IsValidGameCell(sim, game_cell)) {
        PushSubstanceChange(*this, game_cell);
    }
    sim.unk_0x68[cell] |= 0x1f;
}

void SimEvents::SpawnFX(const SimData &sim, unsigned long cell, SpawnFXHashes::Type fx, float rotation) {
    int game_cell = ToGameCell(sim, static_cast<int>(cell));
    if (IsValidGameCell(sim, game_cell)) {
        PushSpawnFX(*this, game_cell, fx, rotation);
    }
}
