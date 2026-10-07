// Generated from sim/layout/layout.txt. Do not edit.
typedef unsigned char uint8_t;
typedef unsigned short uint16_t;
typedef short int16_t;
typedef unsigned int uint32_t;
typedef int int32_t;
typedef unsigned long long uint64_t;
typedef long long int64_t;

struct CellSOA;
struct SimData;
struct SimEvents;

struct Vec_f32 { float *begin; float *end; float *cap; };
struct Vec_i32 { int32_t *begin; int32_t *end; int32_t *cap; };
struct Vec_ptr { void * *begin; void * *end; void * *cap; };
struct Vec_u16 { uint16_t *begin; uint16_t *end; uint16_t *cap; };
struct Vec_u8 { uint8_t *begin; uint8_t *end; uint8_t *cap; };

struct CellSOA {
    struct Vec_u16 elementIdx;
    struct Vec_f32 temperature;
    struct Vec_f32 mass;
    struct Vec_u8 properties;
    struct Vec_u8 insulation;
    struct Vec_u8 strengthInfo;
    struct Vec_u8 diseaseIdx;
    struct Vec_i32 diseaseCount;
    struct Vec_u8 unk_0xc0;
    struct Vec_f32 unk_0xd8;
    struct Vec_f32 unk_0xf0;
};

struct SimData {
    int32_t width;
    int32_t height;
    int32_t numGameCells;
    uint8_t pad_0xc[12];
    uint8_t unk_0x18;
    uint8_t pad_0x19[7];
    struct CellSOA * cells;
    struct CellSOA * updatedCells;
    uint8_t pad_0x30[96];
    uint16_t unk_0x90;
    uint8_t pad_0x92[6];
    struct SimEvents * events;
    float unk_0xa0;
    float unk_0xa4;
    uint8_t pad_0xa8[8];
    struct Vec_ptr components;
    uint8_t pad_0xc8[1616];
    uint8_t unk_0x718;
    uint8_t headless;
    uint8_t radiationEnabled;
    uint8_t pad_0x71b[1];
    float unk_0x71c;
    float unk_0x720;
    float unk_0x724;
    float unk_0x728;
    float unk_0x72c;
    uint8_t pad_0x730[24];
};

struct SimEvents {
    uint8_t pad_0x0[480];
};

