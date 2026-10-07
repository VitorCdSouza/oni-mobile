// Redirects one imported function of an already loaded shared library by rewriting its GOT slot.
//
// Used to hand libSimDLL our own time() (the simulation seeds its RNG with it), both when
// recording and when replaying. Symbol interposition does not work here because the library
// is dlopen'ed by Mono and libc comes first in its lookup scope.
#pragma once
#define _GNU_SOURCE
#include <elf.h>
#include <link.h>
#include <string.h>
#include <sys/mman.h>
#include <unistd.h>

typedef struct {
    const char *lib_substring;
    const char *symbol;
    void *replacement;
    int patched;
} GotPatchRequest;

static int gotpatch_visit(struct dl_phdr_info *info, size_t size, void *data) {
    (void)size;
    GotPatchRequest *req = (GotPatchRequest *)data;
    if (!info->dlpi_name || !strstr(info->dlpi_name, req->lib_substring)) {
        return 0;
    }
    const ElfW(Dyn) *dyn = NULL;
    for (int i = 0; i < info->dlpi_phnum; i++) {
        if (info->dlpi_phdr[i].p_type == PT_DYNAMIC) {
            dyn = (const ElfW(Dyn) *)(info->dlpi_addr + info->dlpi_phdr[i].p_vaddr);
        }
    }
    if (!dyn) {
        return 0;
    }
    const ElfW(Sym) *symtab = NULL;
    const char *strtab = NULL;
    const ElfW(Rela) *rel_tables[2] = {NULL, NULL};
    size_t rel_sizes[2] = {0, 0};
    for (; dyn->d_tag != DT_NULL; dyn++) {
        switch (dyn->d_tag) {
        case DT_SYMTAB: symtab = (const ElfW(Sym) *)dyn->d_un.d_ptr; break;
        case DT_STRTAB: strtab = (const char *)dyn->d_un.d_ptr; break;
        case DT_JMPREL: rel_tables[0] = (const ElfW(Rela) *)dyn->d_un.d_ptr; break;
        case DT_PLTRELSZ: rel_sizes[0] = dyn->d_un.d_val; break;
        case DT_RELA: rel_tables[1] = (const ElfW(Rela) *)dyn->d_un.d_ptr; break;
        case DT_RELASZ: rel_sizes[1] = dyn->d_un.d_val; break;
        }
    }
    long page = sysconf(_SC_PAGESIZE);
    for (int t = 0; t < 2; t++) {
        if (!rel_tables[t]) {
            continue;
        }
        size_t count = rel_sizes[t] / sizeof(ElfW(Rela));
        for (size_t i = 0; i < count; i++) {
            const ElfW(Rela) *r = &rel_tables[t][i];
            unsigned long type = ELF64_R_TYPE(r->r_info);
            if (type != R_X86_64_JUMP_SLOT && type != R_X86_64_GLOB_DAT) {
                continue;
            }
            const char *name = strtab + symtab[ELF64_R_SYM(r->r_info)].st_name;
            if (strcmp(name, req->symbol) != 0) {
                continue;
            }
            void **slot = (void **)(info->dlpi_addr + r->r_offset);
            void *page_start = (void *)((uintptr_t)slot & ~(uintptr_t)(page - 1));
            mprotect(page_start, (size_t)page, PROT_READ | PROT_WRITE);
            *slot = req->replacement;
            req->patched++;
        }
    }
    return 0;
}

// Returns how many GOT slots were rewritten (0 means the symbol was not found).
static int gotpatch(const char *lib_substring, const char *symbol, void *replacement) {
    GotPatchRequest req = {lib_substring, symbol, replacement, 0};
    dl_iterate_phdr(gotpatch_visit, &req);
    return req.patched;
}
