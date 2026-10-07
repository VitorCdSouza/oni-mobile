// Binary format of a SimDLL call recording.
//
// A recording is the exact sequence of calls the game made into SimDLL, with every input the
// library can observe (message bytes, data behind pointers, the value of time()). Replaying it
// against two builds of the library and comparing the GameDataUpdate after each frame tells
// whether a reimplementation behaves like the original.
//
// File layout: the 8-byte magic, then records. Each record is a 1-byte type followed by its
// payload; all integers are little-endian.
#pragma once
#include <stdint.h>

#define SIMREC_MAGIC "ONISIMR1"

enum SimRecType {
    // payload: nothing. The game's callback is not recorded, the replay installs its own.
    REC_SIM_INITIALIZE = 1,
    REC_SIM_SHUTDOWN = 2,
    // payload: i32 msg_id, i32 msg_length, msg bytes.
    // For MSG_CREATE_ELEMENT_INTERACTIONS the bytes are followed by i32 count and
    // count * ELEMENT_INTERACTION_SIZE bytes, the array the message points to.
    // Always followed (after any REC_TIME made during the call) by REC_RESULT.
    REC_SIM_HANDLE_MESSAGE = 3,
    // payload: i32 x, i32 y, i32 returned_size.
    REC_SIM_BEGIN_SAVE = 4,
    REC_SIM_END_SAVE = 5,
    // payload: i64 value returned by time() inside the library. Written while the call that
    // triggered it is in progress, so it sits between that call's record and its REC_RESULT.
    REC_TIME = 6,
    // payload: u8 1 when SIM_HandleMessage returned a non-null pointer.
    REC_RESULT = 7,
    REC_CTM_INITIALIZE = 10,
    REC_CTM_SHUTDOWN = 11,
    // payload: f32 temperature, f32 mass, i32 element_hash, i32 structure_temperature_handle,
    //          f32 heat_capacity, f32 thermal_conductivity, u8 insulated, i32 returned_handle.
    REC_CTM_ADD = 12,
    // payload: i32 handle, f32 temperature, f32 mass, i32 element_hash, i32 returned.
    REC_CTM_SET = 13,
    // payload: i32 handle.
    REC_CTM_REMOVE = 14,
    // payload: f32 dt, u8 pointer_kind (CTM_PTR_*). With CTM_PTR_OTHER the array length is not
    // knowable, so the replay cannot reproduce that call and stops there.
    REC_CTM_UPDATE = 15,
    REC_CTM_CLEAR = 16,
    // payload: i32 message_id. A message the library sent back to the game (errors, crashes).
    REC_CALLBACK = 20,
};

enum CtmPointerKind {
    CTM_PTR_NULL = 0,
    // The pointer was buildingTemperatures of the latest GameDataUpdate, which is what the game
    // always passes. The replay passes its own library's buildingTemperatures.
    CTM_PTR_LAST_UPDATE = 1,
    CTM_PTR_OTHER = 2,
};

#define MSG_CREATE_ELEMENT_INTERACTIONS (-930289787)
#define MSG_LOAD (-672538170)
#define MSG_ALLOCATE_CELLS 1092408308
// Both return a pointer to the library-owned GameDataUpdate.
#define MSG_START (-931446686)
#define MSG_PREPARE_GAME_DATA 1078620451
#define GDU_BUILDING_TEMPERATURES_OFFSET 236
#define ELEMENT_INTERACTION_SIZE 32
