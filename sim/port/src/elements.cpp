// Element lookups by name hash.
#include <unordered_map>

#include "sim_common.h"

// Element name hash to element index, filled when the element table is created.
extern std::unordered_map<uint32_t, uint16_t> gElementIndices;

uint16_t GetElementIndex(uint32_t hash) {
    auto it = gElementIndices.find(hash);
    return it == gElementIndices.end() ? kInvalidElement : it->second;
}
