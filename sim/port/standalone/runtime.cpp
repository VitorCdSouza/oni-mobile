// Standalone build: the globals and the small runtime pieces the original keeps in the library
// (asserts, crash reports, logger, profiler and system info entry points).
#include <cstdarg>
#include <cstdio>
#include <string>
#include <unordered_map>
#include <vector>

#include "../src/sim_common.h"

// --- Globals ---

bool gAssertsDisabled = false;
SimData *gSimData = nullptr;
Disease *gDisease = nullptr;
ConduitTemperatureManager *gConduitTemperatureManager = nullptr;

std::vector<Element> gElements;
std::vector<ElementTemperatureData> gElementTemperatureData;
std::vector<ElementPostProcessData> gElementPostProcessData;
std::vector<ElementLiquidData> gElementLiquidData;
std::vector<PhysicsData> gElementPhysicsData;
std::vector<ElementPressureData> gElementPressureData;
std::vector<ElementPropertyTextureData> gElementPropertyTextureData;
std::vector<ElementLightAbsorptionData> gElementLightAbsorptionData;
std::vector<ElementRadiationData> gElementRadiationData;
std::vector<uint8_t> gElementStateData;
std::unordered_map<uint32_t, uint16_t> gElementIndices;
std::vector<std::string> gElementNames;

namespace Math {
struct FloatConstants {
    static const float PI;
};
const float FloatConstants::PI = 0x1.921fb6p+1f;
}  // namespace Math

// --- Asserts and crash reports ---
// The original logs and raises SIGINT on an assert (Unity handles it); here they are only logged,
// when SIMPORT_LOG_ASSERTS is set.

namespace {

bool LogAsserts() {
    static const bool enabled = getenv("SIMPORT_LOG_ASSERTS") != nullptr;
    return enabled;
}

}  // namespace

void AssertFunc(const char *tag, int line, const char *file) {
    if (LogAsserts()) {
        fprintf(stderr, "Assert failure '%s' at %s(%d)\n", tag, file, line);
    }
}

void KCrashReporterReportMessage(const char *file, int line, const char *format, ...) {
    if (!LogAsserts()) {
        return;
    }
    fprintf(stderr, "%s(%d): ", file, line);
    va_list args;
    va_start(args, format);
    vfprintf(stderr, format, args);
    va_end(args);
    fputc('\n', stderr);
}

// --- Logger (the original writes to a log file; the standalone build has no logger instance) ---

namespace LogType {
enum Type : int32_t {};
}
struct Logger {
    enum eLogLevel : int32_t {};
    void Log(eLogLevel level, LogType::Type type, const char *format, ...);
};
namespace Util {
template <typename T>
class cSingleton {
public:
    static T *mInstance;
};
template <>
Logger *cSingleton<Logger>::mInstance = nullptr;
}  // namespace Util

void Logger::Log(eLogLevel, LogType::Type, const char *, ...) {}

// --- C entry points the game binds that the port does not implement ---

extern "C" {

__attribute__((visibility("default"))) const char *SYSINFO_Acquire() {
    return "{}";
}

__attribute__((visibility("default"))) void SYSINFO_Release() {}

// The built-in profiler (kprofiler) is not part of the port.
__attribute__((visibility("default"))) void kprofiler_begin_section(...) {}
__attribute__((visibility("default"))) void kprofiler_end_section(...) {}
__attribute__((visibility("default"))) void kprofiler_counter(...) {}
__attribute__((visibility("default"))) uint64_t kprofile_record_string(...) { return 0; }
__attribute__((visibility("default"))) void kprofiler_flush_data_sender(...) {}
__attribute__((visibility("default"))) uint64_t kprofiler_get_thread_uid(...) { return 0; }
__attribute__((visibility("default"))) int kprofiler_load_plugin(...) { return 0; }
__attribute__((visibility("default"))) void kprofiler_ping(...) {}
__attribute__((visibility("default"))) void kprofiler_set_thread_info(...) {}
__attribute__((visibility("default"))) int kprofiler_start_file_data_sender(...) { return 0; }
__attribute__((visibility("default"))) int kprofiler_start_http_control_listener(...) { return 0; }
__attribute__((visibility("default"))) int kprofiler_start_http_data_sender(...) { return 0; }
__attribute__((visibility("default"))) void kprofiler_start_profiling(...) {}
__attribute__((visibility("default"))) void kprofiler_stop_data_sender(...) {}
__attribute__((visibility("default"))) void kprofiler_stop_profiling(...) {}
__attribute__((visibility("default"))) void kprofiler_unload_plugin(...) {}

}  // extern "C"
