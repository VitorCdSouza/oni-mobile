// Recording shim for libSimDLL.so.
//
// Installed in place of the game's libSimDLL.so (the original is renamed libSimDLL_orig.so and
// sits next to it). Every entry point the C# code uses is forwarded to the original, and the
// inputs are appended to a recording in the format described in simrec_format.h.
//
// Output: $ONI_SIMREC_DIR (default ~/oni-simrec)/rec-<unix time>-<pid>-<n>.bin, a new file each
// time the game calls SIM_Initialize.
#define _GNU_SOURCE
#include <dlfcn.h>
#include <errno.h>
#include <pthread.h>
#include <stdint.h>
#include <stdio.h>
#include <stdlib.h>
#include <string.h>
#include <sys/stat.h>
#include <time.h>
#include <unistd.h>

#include "../common/gotpatch.h"
#include "../common/simrec_format.h"

#define EXPORT __attribute__((visibility("default")))

typedef int (*GameMessageHandler)(int message_id, void *data);

static struct {
    void *handle;
    void (*SIM_Initialize)(GameMessageHandler);
    void (*SIM_Shutdown)(void);
    void *(*SIM_HandleMessage)(int, int, uint8_t *);
    void *(*SIM_HandleMessages)(int, int, int, uint8_t *);
    uint8_t *(*SIM_BeginSave)(int *, int, int);
    void (*SIM_EndSave)(void);
    void (*SIM_DebugCrash)(void);
    char *(*SYSINFO_Acquire)(void);
    void (*SYSINFO_Release)(void);
    void (*CTM_Initialize)(void);
    void (*CTM_Shutdown)(void);
    int (*CTM_Add)(float, float, int, int, float, float, int);
    int (*CTM_Set)(int, float, float, int);
    void (*CTM_Remove)(int);
    void *(*CTM_Update)(float, void *);
    void (*CTM_Clear)(void);
} orig;

static pthread_mutex_t g_lock = PTHREAD_RECURSIVE_MUTEX_INITIALIZER_NP;
static FILE *g_out;
static int g_file_index;
static GameMessageHandler g_game_handler;
static void *g_last_gdu;

static void fail(const char *what) {
    fprintf(stderr, "[simrec] %s: %s\n", what, dlerror());
    abort();
}

static void *sym(const char *name) {
    void *p = dlsym(orig.handle, name);
    if (!p) {
        fail(name);
    }
    return p;
}

static void put(const void *data, size_t size) {
    if (g_out && size && fwrite(data, 1, size, g_out) != size) {
        fprintf(stderr, "[simrec] write failed (%s), recording stopped\n", strerror(errno));
        fclose(g_out);
        g_out = NULL;
    }
}

static void put_u8(uint8_t v) { put(&v, 1); }
static void put_i32(int32_t v) { put(&v, 4); }
static void put_f32(float v) { put(&v, 4); }
static void put_i64(int64_t v) { put(&v, 8); }

static void open_recording(void) {
    if (g_out) {
        fclose(g_out);
    }
    const char *dir = getenv("ONI_SIMREC_DIR");
    char default_dir[4096];
    if (!dir) {
        snprintf(default_dir, sizeof default_dir, "%s/oni-simrec", getenv("HOME") ? getenv("HOME") : "/tmp");
        dir = default_dir;
    }
    mkdir(dir, 0755);
    char path[4352];
    snprintf(path, sizeof path, "%s/rec-%ld-%d-%d.bin", dir, (long)time(NULL), (int)getpid(), g_file_index++);
    g_out = fopen(path, "wb");
    if (!g_out) {
        fprintf(stderr, "[simrec] cannot open %s: %s\n", path, strerror(errno));
        return;
    }
    setvbuf(g_out, NULL, _IOFBF, 8 << 20);
    put(SIMREC_MAGIC, 8);
    fprintf(stderr, "[simrec] recording to %s\n", path);
}

static time_t recorded_time(time_t *out) {
    time_t now = time(NULL);
    pthread_mutex_lock(&g_lock);
    put_u8(REC_TIME);
    put_i64((int64_t)now);
    pthread_mutex_unlock(&g_lock);
    if (out) {
        *out = now;
    }
    return now;
}

__attribute__((constructor)) static void load_original(void) {
    Dl_info self;
    dladdr((void *)load_original, &self);
    char path[4096];
    snprintf(path, sizeof path, "%s", self.dli_fname);
    char *slash = strrchr(path, '/');
    snprintf(slash ? slash + 1 : path, sizeof path - (size_t)(slash ? slash + 1 - path : 0), "libSimDLL_orig.so");
    orig.handle = dlopen(path, RTLD_NOW | RTLD_LOCAL);
    if (!orig.handle) {
        fail(path);
    }
    orig.SIM_Initialize = sym("SIM_Initialize");
    orig.SIM_Shutdown = sym("SIM_Shutdown");
    orig.SIM_HandleMessage = sym("SIM_HandleMessage");
    orig.SIM_HandleMessages = sym("SIM_HandleMessages");
    orig.SIM_BeginSave = sym("SIM_BeginSave");
    orig.SIM_EndSave = sym("SIM_EndSave");
    orig.SIM_DebugCrash = sym("SIM_DebugCrash");
    orig.SYSINFO_Acquire = sym("SYSINFO_Acquire");
    orig.SYSINFO_Release = sym("SYSINFO_Release");
    orig.CTM_Initialize = sym("ConduitTemperatureManager_Initialize");
    orig.CTM_Shutdown = sym("ConduitTemperatureManager_Shutdown");
    orig.CTM_Add = sym("ConduitTemperatureManager_Add");
    orig.CTM_Set = sym("ConduitTemperatureManager_Set");
    orig.CTM_Remove = sym("ConduitTemperatureManager_Remove");
    orig.CTM_Update = sym("ConduitTemperatureManager_Update");
    orig.CTM_Clear = sym("ConduitTemperatureManager_Clear");
    if (gotpatch("libSimDLL_orig.so", "time", (void *)recorded_time) == 0) {
        fprintf(stderr, "[simrec] warning: time() import not found, replays may diverge\n");
    }
    open_recording();
}

static int recorded_callback(int message_id, void *data) {
    pthread_mutex_lock(&g_lock);
    put_u8(REC_CALLBACK);
    put_i32(message_id);
    fflush(g_out);
    pthread_mutex_unlock(&g_lock);
    return g_game_handler ? g_game_handler(message_id, data) : -1;
}

EXPORT void SIM_Initialize(GameMessageHandler callback) {
    pthread_mutex_lock(&g_lock);
    if (!g_out || ftell(g_out) > 8) {
        open_recording();
    }
    g_game_handler = callback;
    put_u8(REC_SIM_INITIALIZE);
    orig.SIM_Initialize(recorded_callback);
    pthread_mutex_unlock(&g_lock);
}

EXPORT void SIM_Shutdown(void) {
    pthread_mutex_lock(&g_lock);
    put_u8(REC_SIM_SHUTDOWN);
    if (g_out) {
        fflush(g_out);
    }
    orig.SIM_Shutdown();
    pthread_mutex_unlock(&g_lock);
}

EXPORT void *SIM_HandleMessage(int msg_id, int msg_length, uint8_t *msg) {
    pthread_mutex_lock(&g_lock);
    put_u8(REC_SIM_HANDLE_MESSAGE);
    put_i32(msg_id);
    put_i32(msg_length);
    put(msg, msg && msg_length > 0 ? (size_t)msg_length : 0);
    if (msg_id == MSG_CREATE_ELEMENT_INTERACTIONS) {
        int32_t count;
        void *array;
        memcpy(&count, msg, 4);
        memcpy(&array, msg + 4, sizeof array);
        put_i32(count);
        put(array, (size_t)count * ELEMENT_INTERACTION_SIZE);
    }
    void *result = orig.SIM_HandleMessage(msg_id, msg_length, msg);
    put_u8(REC_RESULT);
    put_u8(result != NULL);
    if (msg_id == MSG_START || msg_id == MSG_PREPARE_GAME_DATA) {
        g_last_gdu = result;
    }
    if (msg_id == MSG_LOAD && g_out) {
        fflush(g_out);
    }
    pthread_mutex_unlock(&g_lock);
    return result;
}

// Not called by the game's C# code; forwarded without recording so an unexpected caller
// shows up in the log instead of silently desynchronizing the recording.
EXPORT void *SIM_HandleMessages(int msg_id, int msg_length, int msg_count, uint8_t *msg) {
    fprintf(stderr, "[simrec] warning: SIM_HandleMessages called, not recorded\n");
    return orig.SIM_HandleMessages(msg_id, msg_length, msg_count, msg);
}

EXPORT uint8_t *SIM_BeginSave(int *size, int x, int y) {
    pthread_mutex_lock(&g_lock);
    uint8_t *result = orig.SIM_BeginSave(size, x, y);
    put_u8(REC_SIM_BEGIN_SAVE);
    put_i32(x);
    put_i32(y);
    put_i32(*size);
    pthread_mutex_unlock(&g_lock);
    return result;
}

EXPORT void SIM_EndSave(void) {
    pthread_mutex_lock(&g_lock);
    put_u8(REC_SIM_END_SAVE);
    orig.SIM_EndSave();
    if (g_out) {
        fflush(g_out);
    }
    pthread_mutex_unlock(&g_lock);
}

EXPORT void SIM_DebugCrash(void) { orig.SIM_DebugCrash(); }
EXPORT char *SYSINFO_Acquire(void) { return orig.SYSINFO_Acquire(); }
EXPORT void SYSINFO_Release(void) { orig.SYSINFO_Release(); }

EXPORT void ConduitTemperatureManager_Initialize(void) {
    pthread_mutex_lock(&g_lock);
    put_u8(REC_CTM_INITIALIZE);
    orig.CTM_Initialize();
    pthread_mutex_unlock(&g_lock);
}

EXPORT void ConduitTemperatureManager_Shutdown(void) {
    pthread_mutex_lock(&g_lock);
    put_u8(REC_CTM_SHUTDOWN);
    orig.CTM_Shutdown();
    pthread_mutex_unlock(&g_lock);
}

EXPORT int ConduitTemperatureManager_Add(float temperature, float mass, int element_hash, int structure_handle,
                                         float heat_capacity, float conductivity, int insulated) {
    pthread_mutex_lock(&g_lock);
    int handle = orig.CTM_Add(temperature, mass, element_hash, structure_handle, heat_capacity, conductivity, insulated);
    put_u8(REC_CTM_ADD);
    put_f32(temperature);
    put_f32(mass);
    put_i32(element_hash);
    put_i32(structure_handle);
    put_f32(heat_capacity);
    put_f32(conductivity);
    put_u8((uint8_t)(insulated != 0));
    put_i32(handle);
    pthread_mutex_unlock(&g_lock);
    return handle;
}

EXPORT int ConduitTemperatureManager_Set(int handle, float temperature, float mass, int element_hash) {
    pthread_mutex_lock(&g_lock);
    int result = orig.CTM_Set(handle, temperature, mass, element_hash);
    put_u8(REC_CTM_SET);
    put_i32(handle);
    put_f32(temperature);
    put_f32(mass);
    put_i32(element_hash);
    put_i32(result);
    pthread_mutex_unlock(&g_lock);
    return result;
}

EXPORT void ConduitTemperatureManager_Remove(int handle) {
    pthread_mutex_lock(&g_lock);
    put_u8(REC_CTM_REMOVE);
    put_i32(handle);
    orig.CTM_Remove(handle);
    pthread_mutex_unlock(&g_lock);
}

EXPORT void *ConduitTemperatureManager_Update(float dt, void *building_temperatures) {
    pthread_mutex_lock(&g_lock);
    uint8_t kind = CTM_PTR_OTHER;
    if (!building_temperatures) {
        kind = CTM_PTR_NULL;
    } else if (g_last_gdu) {
        void *latest;
        memcpy(&latest, (uint8_t *)g_last_gdu + GDU_BUILDING_TEMPERATURES_OFFSET, sizeof latest);
        if (latest == building_temperatures) {
            kind = CTM_PTR_LAST_UPDATE;
        }
    }
    put_u8(REC_CTM_UPDATE);
    put_f32(dt);
    put_u8(kind);
    void *result = orig.CTM_Update(dt, building_temperatures);
    pthread_mutex_unlock(&g_lock);
    return result;
}

EXPORT void ConduitTemperatureManager_Clear(void) {
    pthread_mutex_lock(&g_lock);
    put_u8(REC_CTM_CLEAR);
    orig.CTM_Clear();
    pthread_mutex_unlock(&g_lock);
}

// kprofiler_* entry points: forwarded untouched. Generated by a macro to keep them in sync with
// the declarations in KProfilerPlugin.cs.
#define FORWARD(ret, name, params, args)                       \
    EXPORT ret name params {                                   \
        static ret(*fn) params;                                \
        if (!fn) fn = (ret(*) params)sym(#name);               \
        return fn args;                                        \
    }
FORWARD(void, kprofiler_load_plugin, (void), ())
FORWARD(void, kprofiler_unload_plugin, (void), ())
FORWARD(void, kprofiler_start_http_control_listener, (int port), (port))
FORWARD(void, kprofiler_start_http_data_sender, (int port), (port))
FORWARD(void, kprofiler_start_file_data_sender, (const char *filename), (filename))
FORWARD(void, kprofiler_flush_data_sender, (void), ())
FORWARD(void, kprofiler_stop_data_sender, (void), ())
FORWARD(void, kprofiler_start_profiling, (void), ())
FORWARD(void, kprofiler_stop_profiling, (int broadcast_info), (broadcast_info))
FORWARD(uint64_t, kprofile_record_string, (const char *str), (str))
FORWARD(uint64_t, kprofiler_get_thread_uid, (void), ())
FORWARD(void, kprofiler_set_thread_info, (uint64_t thread_id, uint64_t name, uint64_t category), (thread_id, name, category))
FORWARD(void, kprofiler_begin_section, (uint64_t name, uint64_t category, int64_t gc_alloc_count), (name, category, gc_alloc_count))
FORWARD(void, kprofiler_end_section, (int64_t gc_alloc_count), (gc_alloc_count))
FORWARD(void, kprofiler_ping, (uint64_t name, uint64_t category, double value), (name, category, value))
FORWARD(void, kprofiler_counter, (uint64_t name, double value), (name, value))
