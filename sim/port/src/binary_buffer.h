// The original's BinaryBufferReader: a cursor over a message buffer, read without bounds checks.
// Same layout, so the port can read the readers the original creates (the reads here are
// non-virtual; the original's are virtual but do the same for this class).
#pragma once

#include <cstdint>
#include <cstring>
#include <string>

struct BinaryBufferReader {
    void *vtable;
    uint64_t pos;
    const uint8_t *data;
    uint64_t size;

    template <typename T>
    T Read() {
        T value;
        std::memcpy(&value, data + pos, sizeof value);
        pos += sizeof value;
        return value;
    }
    template <typename T>
    BinaryBufferReader &operator>>(T &value) {
        value = Read<T>();
        return *this;
    }
    void ReadBytes(uint64_t count, void *out) {
        std::memcpy(out, data + pos, count);
        pos += count;
    }
    void SkipBytes(uint64_t count) { pos += count; }
    // uint32 length, then the bytes.
    BinaryBufferReader &operator>>(std::string &value) {
        uint32_t length = Read<uint32_t>();
        value.assign(reinterpret_cast<const char *>(data + pos), length);
        pos += length;
        return *this;
    }
    const uint8_t *Current() const { return data + pos; }
};
static_assert(sizeof(BinaryBufferReader) == 0x20, "BinaryBufferReader size");
