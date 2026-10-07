// Replays a SimDLL recording against a library build and prints a hash of every piece of state
// the game can read back, one line per observation point.
//
//   replay <libSimDLL.so> <recording.bin> [--dump-at N <dir>]
//
// With SIMPORT=<libsimport.so> in the environment, the port library is loaded on top of the
// original and its functions are patched in (hybrid mode, see port/detour/detour.cpp).
//
// Running it against the original library and against a reimplementation and diffing the two
// outputs (see compare.py) points at the first frame and the first array that diverge.
// --dump-at N writes the raw arrays of observation N to <dir> for a cell-level diff.
#define _GNU_SOURCE
#include <dlfcn.h>
#include <stdint.h>
#include <stdio.h>
#include <stdlib.h>
#include <string.h>
#include <time.h>

#include "../common/gotpatch.h"
#include "../common/simrec_format.h"
#include "gdu_layout.h"

typedef int (*GameMessageHandler)(int message_id, void *data);

static struct {
    void (*SIM_Initialize)(GameMessageHandler);
    void (*SIM_Shutdown)(void);
    void *(*SIM_HandleMessage)(int, int, uint8_t *);
    uint8_t *(*SIM_BeginSave)(int *, int, int);
    void (*SIM_EndSave)(void);
    void (*CTM_Initialize)(void);
    void (*CTM_Shutdown)(void);
    int (*CTM_Add)(float, float, int, int, float, float, int);
    int (*CTM_Set)(int, float, float, int);
    void (*CTM_Remove)(int);
    void *(*CTM_Update)(float, void *);
    void (*CTM_Clear)(void);
} lib;

static FILE *g_in;
static long g_observation;
static long g_dump_at = -1;
static const char *g_dump_dir;
static int g_width, g_height;
static void *g_last_gdu;

__attribute__((noreturn)) static void die(const char *msg) {
    fprintf(stderr, "replay: %s (at byte %ld)\n", msg, g_in ? ftell(g_in) : -1L);
    exit(2);
}

static void get(void *dst, size_t size) {
    if (size && fread(dst, 1, size, g_in) != size) {
        die("truncated recording");
    }
}

static uint8_t get_u8(void) { uint8_t v; get(&v, 1); return v; }
static int32_t get_i32(void) { int32_t v; get(&v, 4); return v; }
static float get_f32(void) { float v; get(&v, 4); return v; }
static int64_t get_i64(void) { int64_t v; get(&v, 8); return v; }

static void *need(void *handle, const char *name) {
    void *p = dlsym(handle, name);
    if (!p) {
        fprintf(stderr, "replay: missing symbol %s\n", name);
        exit(2);
    }
    return p;
}

// time() inside the library: the value comes from the next record, which the recorder wrote
// while the same call was in progress.
static time_t replayed_time(time_t *out) {
    int type = fgetc(g_in);
    if (type != REC_TIME) {
        die("library called time() where the recording has no REC_TIME");
    }
    time_t v = (time_t)get_i64();
    if (out) {
        *out = v;
    }
    return v;
}

static unsigned g_threads;

// std::thread::hardware_concurrency() inside the library, overridden with SIM_THREADS to check
// that results do not depend on the core count (the phone has 8, the PC 28).
static unsigned replayed_hardware_concurrency(void) { return g_threads; }

static int game_handler(int message_id, void *data) {
    (void)data;
    printf("callback %d\n", message_id);
    return 0;
}

static uint64_t hash_bytes(const void *data, size_t size) {
    // FNV-1a over 8-byte words, then the tail byte by byte. Only used for equality checks.
    const uint8_t *p = (const uint8_t *)data;
    uint64_t h = 1469598103934665603ULL;
    size_t i = 0;
    for (; i + 8 <= size; i += 8) {
        uint64_t w;
        memcpy(&w, p + i, 8);
        h = (h ^ w) * 1099511628211ULL;
    }
    for (; i < size; i++) {
        h = (h ^ p[i]) * 1099511628211ULL;
    }
    return h;
}

static void dump(const char *name, const void *data, size_t size) {
    char path[4096];
    snprintf(path, sizeof path, "%s/%s.bin", g_dump_dir, name);
    FILE *f = fopen(path, "wb");
    if (!f) {
        die("cannot write dump");
    }
    fwrite(data, 1, size, f);
    fclose(f);
}

static void observe(const char *label, const char *name, const void *data, size_t size) {
    printf(" %s=%016llx", name, (unsigned long long)(data ? hash_bytes(data, size) : 0));
    if (g_observation == g_dump_at) {
        char full[256];
        snprintf(full, sizeof full, "%s.%s", label, name);
        dump(full, data, data ? size : 0);
    }
}

static void observe_gdu(const uint8_t *gdu) {
    int32_t frames;
    memcpy(&frames, gdu + GDU_NUM_FRAMES_OFFSET, 4);
    printf("%ld gdu frames=%d", g_observation, frames);
    size_t cells = (size_t)g_width * (size_t)g_height;
    for (size_t i = 0; i < sizeof kGduCellArrays / sizeof kGduCellArrays[0]; i++) {
        void *p;
        memcpy(&p, gdu + kGduCellArrays[i].ptr_offset, sizeof p);
        observe("gdu", kGduCellArrays[i].name, p, cells * (size_t)kGduCellArrays[i].elem_size);
    }
    for (size_t i = 0; i < sizeof kGduArrays / sizeof kGduArrays[0]; i++) {
        int32_t count;
        void *p;
        memcpy(&count, gdu + kGduArrays[i].count_offset, 4);
        memcpy(&p, gdu + kGduArrays[i].ptr_offset, sizeof p);
        printf(" n_%s=%d", kGduArrays[i].name, count);
        observe("gdu", kGduArrays[i].name, p, (size_t)count * (size_t)kGduArrays[i].elem_size);
    }
    printf("\n");
    g_observation++;
}

// ConduitTemperatureUpdateData, Pack = 4: three (int count, pointer) pairs.
static void observe_ctm(const uint8_t *data) {
    static const char *names[] = {"temperatures", "frozenHandles", "meltedHandles"};
    printf("%ld ctm", g_observation);
    for (int i = 0; i < 3; i++) {
        int32_t count;
        void *p;
        memcpy(&count, data + i * 12, 4);
        memcpy(&p, data + i * 12 + 4, sizeof p);
        printf(" n_%s=%d", names[i], count);
        observe("ctm", names[i], p, (size_t)count * 4);
    }
    printf("\n");
    g_observation++;
}

static void replay_message(void) {
    int32_t id = get_i32();
    int32_t length = get_i32();
    uint8_t *msg = length > 0 ? malloc((size_t)length) : NULL;
    get(msg, length > 0 ? (size_t)length : 0);
    uint8_t *interactions = NULL;
    if (id == MSG_CREATE_ELEMENT_INTERACTIONS) {
        int32_t count = get_i32();
        interactions = malloc((size_t)count * ELEMENT_INTERACTION_SIZE + 1);
        get(interactions, (size_t)count * ELEMENT_INTERACTION_SIZE);
        memcpy(msg + 4, &interactions, sizeof interactions);
    }
    if (id == MSG_ALLOCATE_CELLS && length >= 8) {
        memcpy(&g_width, msg, 4);
        memcpy(&g_height, msg + 4, 4);
    }
    void *result = lib.SIM_HandleMessage(id, length, msg);
    if (fgetc(g_in) != REC_RESULT) {
        die("expected REC_RESULT");
    }
    uint8_t recorded_non_null = get_u8();
    if (recorded_non_null != (result != NULL)) {
        printf("%ld result-mismatch msg=%d recorded=%d replayed=%d\n", g_observation, id, recorded_non_null,
               result != NULL);
    }
    if (result && (id == MSG_START || id == MSG_PREPARE_GAME_DATA)) {
        g_last_gdu = result;
        observe_gdu((const uint8_t *)result);
    }
    free(interactions);
    free(msg);
}

int main(int argc, char **argv) {
    if (argc < 3) {
        fprintf(stderr, "usage: %s <libSimDLL.so> <recording.bin> [--dump-at N <dir>]\n", argv[0]);
        return 2;
    }
    if (argc >= 6 && strcmp(argv[3], "--dump-at") == 0) {
        g_dump_at = atol(argv[4]);
        g_dump_dir = argv[5];
    }
    const char *port = getenv("SIMPORT");
    // RTLD_GLOBAL in hybrid mode: the port's calls to functions it does not define yet must
    // resolve to the original.
    void *handle = dlopen(argv[1], RTLD_NOW | (port ? RTLD_GLOBAL : RTLD_LOCAL));
    if (!handle) {
        fprintf(stderr, "replay: %s\n", dlerror());
        return 2;
    }
    lib.SIM_Initialize = need(handle, "SIM_Initialize");
    lib.SIM_Shutdown = need(handle, "SIM_Shutdown");
    lib.SIM_HandleMessage = need(handle, "SIM_HandleMessage");
    lib.SIM_BeginSave = need(handle, "SIM_BeginSave");
    lib.SIM_EndSave = need(handle, "SIM_EndSave");
    lib.CTM_Initialize = need(handle, "ConduitTemperatureManager_Initialize");
    lib.CTM_Shutdown = need(handle, "ConduitTemperatureManager_Shutdown");
    lib.CTM_Add = need(handle, "ConduitTemperatureManager_Add");
    lib.CTM_Set = need(handle, "ConduitTemperatureManager_Set");
    lib.CTM_Remove = need(handle, "ConduitTemperatureManager_Remove");
    lib.CTM_Update = need(handle, "ConduitTemperatureManager_Update");
    lib.CTM_Clear = need(handle, "ConduitTemperatureManager_Clear");

    const char *base = strrchr(argv[1], '/') ? strrchr(argv[1], '/') + 1 : argv[1];
    if (port) {
        void *port_handle = dlopen(port, RTLD_NOW | RTLD_LOCAL);
        if (!port_handle) {
            fprintf(stderr, "replay: %s\n", dlerror());
            return 2;
        }
        int (*install)(const char *) = (int (*)(const char *))need(port_handle, "simport_install");
        if (install(base) < 0) {
            return 2;
        }
    }
    if (gotpatch(base, "time", (void *)replayed_time) == 0) {
        fprintf(stderr, "replay: note: %s does not import time()\n", base);
    }

    const char *threads = getenv("SIM_THREADS");
    if (threads) {
        g_threads = (unsigned)atoi(threads);
        if (gotpatch(base, "_ZNSt6thread20hardware_concurrencyEv", (void *)replayed_hardware_concurrency) == 0) {
            fprintf(stderr, "replay: note: %s does not import hardware_concurrency()\n", base);
        }
    }

    g_in = fopen(argv[2], "rb");
    if (!g_in) {
        perror(argv[2]);
        return 2;
    }
    char magic[8];
    get(magic, 8);
    if (memcmp(magic, SIMREC_MAGIC, 8) != 0) {
        die("not a SimDLL recording");
    }

    int type;
    while ((type = fgetc(g_in)) != EOF) {
        switch (type) {
        case REC_SIM_INITIALIZE: lib.SIM_Initialize(game_handler); break;
        case REC_SIM_SHUTDOWN: lib.SIM_Shutdown(); break;
        case REC_SIM_HANDLE_MESSAGE: replay_message(); break;
        case REC_SIM_BEGIN_SAVE: {
            int32_t x = get_i32(), y = get_i32(), recorded_size = get_i32();
            int size = 0;
            uint8_t *data = lib.SIM_BeginSave(&size, x, y);
            printf("%ld save size=%d recorded_size=%d", g_observation, size, recorded_size);
            observe("save", "blob", data, (size_t)size);
            printf("\n");
            g_observation++;
            break;
        }
        case REC_SIM_END_SAVE: lib.SIM_EndSave(); break;
        case REC_CTM_INITIALIZE: lib.CTM_Initialize(); break;
        case REC_CTM_SHUTDOWN: lib.CTM_Shutdown(); break;
        case REC_CTM_ADD: {
            float temperature = get_f32(), mass = get_f32();
            int32_t element = get_i32(), structure = get_i32();
            float capacity = get_f32(), conductivity = get_f32();
            int insulated = get_u8();
            int32_t recorded = get_i32();
            int handle = lib.CTM_Add(temperature, mass, element, structure, capacity, conductivity, insulated);
            if (handle != recorded) {
                printf("%ld ctm-add-mismatch recorded=%d replayed=%d\n", g_observation, recorded, handle);
            }
            break;
        }
        case REC_CTM_SET: {
            int32_t handle = get_i32();
            float temperature = get_f32(), mass = get_f32();
            int32_t element = get_i32(), recorded = get_i32();
            int result = lib.CTM_Set(handle, temperature, mass, element);
            if (result != recorded) {
                printf("%ld ctm-set-mismatch recorded=%d replayed=%d\n", g_observation, recorded, result);
            }
            break;
        }
        case REC_CTM_REMOVE: lib.CTM_Remove(get_i32()); break;
        case REC_CTM_UPDATE: {
            float dt = get_f32();
            uint8_t kind = get_u8();
            void *temps = NULL;
            if (kind == CTM_PTR_LAST_UPDATE && g_last_gdu) {
                memcpy(&temps, (uint8_t *)g_last_gdu + GDU_BUILDING_TEMPERATURES_OFFSET, sizeof temps);
            } else if (kind == CTM_PTR_OTHER) {
                die("ConduitTemperatureManager_Update with an unknown pointer cannot be replayed");
            }
            void *result = lib.CTM_Update(dt, temps);
            if (result) {
                observe_ctm((const uint8_t *)result);
            }
            break;
        }
        case REC_CTM_CLEAR: lib.CTM_Clear(); break;
        case REC_CALLBACK: printf("recorded-callback %d\n", get_i32()); break;
        case REC_TIME: die("REC_TIME outside of a library call");
        default: die("unknown record type");
        }
    }
    fflush(stdout);
    // Skip library destructors: the original's static teardown is not part of what is compared.
    _exit(0);
}
