// Hybrid mode: patches the original libSimDLL so that every function this port library defines
// runs the port's version instead.
//
// The port is written with the same class names and signatures as the original, so its exported
// symbols carry the same mangled names. simport_install() walks the port's own dynamic symbol
// table, looks each defined function up in the original, and writes a jump at the start of the
// original's copy. Functions the port does not define yet keep running the original code, and
// the port's calls to them resolve to the original through the global symbol scope (the replay
// loads the original with RTLD_GLOBAL).
//
// Environment:
//   SIMPORT_ONLY=a,b   patch only symbols whose name contains one of these substrings
//   SIMPORT_SKIP=a,b   do not patch symbols whose name contains one of these substrings
//   SIMPORT_VERBOSE=1  list every patched symbol
#include <dlfcn.h>
#include <elf.h>
#include <link.h>
#include <sys/mman.h>
#include <unistd.h>

#include <cstdint>
#include <cstdio>
#include <cstdlib>
#include <cstring>
#include <string>
#include <vector>

namespace {

struct LoadedLib {
    uintptr_t base = 0;
    const ElfW(Dyn) *dynamic = nullptr;
    std::string path;
};

struct FindRequest {
    const char *substring;
    LoadedLib *out;
};

int find_visit(struct dl_phdr_info *info, size_t, void *data) {
    auto *req = static_cast<FindRequest *>(data);
    if (!info->dlpi_name || !strstr(info->dlpi_name, req->substring)) {
        return 0;
    }
    req->out->base = info->dlpi_addr;
    req->out->path = info->dlpi_name;
    for (int i = 0; i < info->dlpi_phnum; i++) {
        if (info->dlpi_phdr[i].p_type == PT_DYNAMIC) {
            req->out->dynamic = reinterpret_cast<const ElfW(Dyn) *>(info->dlpi_addr + info->dlpi_phdr[i].p_vaddr);
        }
    }
    return 1;
}

bool find_loaded(const char *substring, LoadedLib *out) {
    FindRequest req{substring, out};
    dl_iterate_phdr(find_visit, &req);
    return out->base != 0;
}

// Defined function symbols of a loaded library, read from its file's .dynsym section
// (the section table gives the symbol count directly, unlike the GNU hash table).
struct Symbol {
    std::string name;
    uintptr_t value;
    size_t size;
};

std::vector<Symbol> defined_functions(const std::string &path) {
    std::vector<Symbol> result;
    FILE *f = fopen(path.c_str(), "rb");
    if (!f) {
        return result;
    }
    fseek(f, 0, SEEK_END);
    std::vector<uint8_t> file(static_cast<size_t>(ftell(f)));
    fseek(f, 0, SEEK_SET);
    size_t read = fread(file.data(), 1, file.size(), f);
    fclose(f);
    if (read != file.size()) {
        return result;
    }
    auto *eh = reinterpret_cast<const ElfW(Ehdr) *>(file.data());
    auto *sh = reinterpret_cast<const ElfW(Shdr) *>(file.data() + eh->e_shoff);
    for (int i = 0; i < eh->e_shnum; i++) {
        if (sh[i].sh_type != SHT_DYNSYM) {
            continue;
        }
        auto *syms = reinterpret_cast<const ElfW(Sym) *>(file.data() + sh[i].sh_offset);
        const char *strtab = reinterpret_cast<const char *>(file.data() + sh[sh[i].sh_link].sh_offset);
        size_t count = sh[i].sh_size / sizeof(ElfW(Sym));
        for (size_t s = 0; s < count; s++) {
            if (ELF64_ST_TYPE(syms[s].st_info) == STT_FUNC && syms[s].st_shndx != SHN_UNDEF) {
                result.push_back({strtab + syms[s].st_name, syms[s].st_value, syms[s].st_size});
            }
        }
    }
    return result;
}

std::vector<std::string> split_env(const char *name) {
    std::vector<std::string> parts;
    const char *v = getenv(name);
    if (!v) {
        return parts;
    }
    std::string s(v);
    size_t start = 0;
    while (start <= s.size()) {
        size_t comma = s.find(',', start);
        std::string part = s.substr(start, comma == std::string::npos ? std::string::npos : comma - start);
        if (!part.empty()) {
            parts.push_back(part);
        }
        if (comma == std::string::npos) {
            break;
        }
        start = comma + 1;
    }
    return parts;
}

// Template instantiations from the standard library end up in both libraries; they are not
// part of the port.
bool is_runtime_symbol(const std::string &name) {
    static const char *prefixes[] = {"_ZNSt", "_ZNKSt", "_ZSt", "_ZN9__gnu_cxx", "_ZNK9__gnu_cxx", "__"};
    for (const char *p : prefixes) {
        if (name.compare(0, strlen(p), p) == 0) {
            return true;
        }
    }
    return false;
}

bool matches_any(const std::string &name, const std::vector<std::string> &patterns) {
    for (const auto &p : patterns) {
        if (name.find(p) != std::string::npos) {
            return true;
        }
    }
    return false;
}

// Trampolines live in a page mapped within +-2 GB of the original's code so every patch can be
// a 5-byte relative jump, which fits even the smallest functions. Each trampoline is
// "jmp [rip+0]" followed by the absolute target address.
struct TrampolinePage {
    uint8_t *page = nullptr;
    size_t used = 0;
    size_t size = 0;
};

uint8_t *trampoline_for(TrampolinePage &tp, uintptr_t near, void *target) {
    if (!tp.page) {
        long pagesize = sysconf(_SC_PAGESIZE);
        tp.size = static_cast<size_t>(pagesize) * 16;
        for (uintptr_t hint = (near & ~static_cast<uintptr_t>(pagesize - 1)) - (64u << 20); hint > (1u << 20);
             hint -= (64u << 20)) {
            void *p = mmap(reinterpret_cast<void *>(hint), tp.size, PROT_READ | PROT_WRITE | PROT_EXEC,
                           MAP_PRIVATE | MAP_ANONYMOUS | MAP_FIXED_NOREPLACE, -1, 0);
            if (p != MAP_FAILED) {
                tp.page = static_cast<uint8_t *>(p);
                break;
            }
        }
        if (!tp.page) {
            fprintf(stderr, "[simport] cannot map a trampoline page near the original\n");
            abort();
        }
    }
    if (tp.used + 14 > tp.size) {
        fprintf(stderr, "[simport] trampoline page full\n");
        abort();
    }
    uint8_t *t = tp.page + tp.used;
    t[0] = 0xff;
    t[1] = 0x25;
    memset(t + 2, 0, 4);
    memcpy(t + 6, &target, 8);
    tp.used += 14;
    return t;
}

void write_jump(uint8_t *from, uint8_t *to) {
    long pagesize = sysconf(_SC_PAGESIZE);
    auto start = reinterpret_cast<uintptr_t>(from) & ~static_cast<uintptr_t>(pagesize - 1);
    size_t len = static_cast<size_t>(reinterpret_cast<uintptr_t>(from) + 5 - start);
    mprotect(reinterpret_cast<void *>(start), len, PROT_READ | PROT_WRITE | PROT_EXEC);
    int64_t rel = reinterpret_cast<intptr_t>(to) - reinterpret_cast<intptr_t>(from + 5);
    if (rel > INT32_MAX || rel < INT32_MIN) {
        fprintf(stderr, "[simport] trampoline out of jump range\n");
        abort();
    }
    auto rel32 = static_cast<int32_t>(rel);
    from[0] = 0xe9;
    memcpy(from + 1, &rel32, 4);
    mprotect(reinterpret_cast<void *>(start), len, PROT_READ | PROT_EXEC);
    __builtin___clear_cache(reinterpret_cast<char *>(from), reinterpret_cast<char *>(from + 5));
}

}  // namespace

// Returns the number of patched functions, or -1 when the original library is not loaded.
extern "C" __attribute__((visibility("default"))) int simport_install(const char *original_substring) {
    LoadedLib original, self;
    if (!find_loaded(original_substring, &original)) {
        fprintf(stderr, "[simport] original library '%s' not loaded\n", original_substring);
        return -1;
    }
    Dl_info info;
    dladdr(reinterpret_cast<void *>(&simport_install), &info);
    self.path = info.dli_fname;
    self.base = reinterpret_cast<uintptr_t>(info.dli_fbase);

    auto originals = defined_functions(original.path);
    auto ours = defined_functions(self.path);
    auto only = split_env("SIMPORT_ONLY");
    auto skip = split_env("SIMPORT_SKIP");
    bool verbose = getenv("SIMPORT_VERBOSE") != nullptr;

    TrampolinePage tp;
    int patched = 0;
    for (const auto &mine : ours) {
        if (mine.name.rfind("simport_", 0) == 0 || is_runtime_symbol(mine.name) || (!only.empty() && !matches_any(mine.name, only)) ||
            matches_any(mine.name, skip)) {
            continue;
        }
        for (const auto &theirs : originals) {
            if (theirs.name != mine.name) {
                continue;
            }
            if (theirs.size < 5) {
                fprintf(stderr, "[simport] %s is %zu bytes, too small to patch\n", theirs.name.c_str(), theirs.size);
                break;
            }
            auto *target = reinterpret_cast<void *>(self.base + mine.value);
            auto *site = reinterpret_cast<uint8_t *>(original.base + theirs.value);
            write_jump(site, trampoline_for(tp, original.base, target));
            patched++;
            if (verbose) {
                fprintf(stderr, "[simport] %s\n", mine.name.c_str());
            }
            break;
        }
    }
    fprintf(stderr, "[simport] %d functions patched\n", patched);
    return patched;
}

// Coverage builds (make coverage): writes the gcov counters. The replay exits with _exit() to skip
// the original library's static destructors, which would also skip gcov's own exit handler.
#ifdef SIMPORT_COVERAGE
extern "C" void __gcov_dump(void);
#endif
extern "C" __attribute__((visibility("default"))) void simport_flush_coverage() {
#ifdef SIMPORT_COVERAGE
    __gcov_dump();
#endif
}
