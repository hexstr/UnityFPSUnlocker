#include "hide.hh"

#include <sys/mman.h>

#include "pmparser.h"
#include "riru_hide_logger.hh"

/**
 * Magic to hide from /proc/###/maps, the idea is from Haruue Icymoon (https://github.com/haruue)
 */

struct hide_struct {
    procmaps_struct* original;
    uintptr_t backup_address;
};

static int get_prot(const procmaps_struct* procstruct) {
    int prot = 0;
    if (procstruct->is_r) {
        prot |= PROT_READ;
    }
    if (procstruct->is_w) {
        prot |= PROT_WRITE;
    }
    if (procstruct->is_x) {
        prot |= PROT_EXEC;
    }
    return prot;
}

#define FAILURE_RETURN(exp, failure_value) ({   \
    __typeof__(exp) _rc;                    \
    _rc = (exp);                            \
    if (_rc == failure_value) {             \
        PLOGE(#exp);                        \
        return 1;                           \
    }                                       \
    _rc; })

static int do_hide(hide_struct* data) {
    auto procstruct = data->original;
    auto start = (uintptr_t)procstruct->addr_start;
    auto end = (uintptr_t)procstruct->addr_end;
    auto length = end - start;
    int prot = get_prot(procstruct);

    // backup
    data->backup_address = (uintptr_t)FAILURE_RETURN(
        mmap(nullptr, length, PROT_READ | PROT_WRITE, MAP_ANONYMOUS | MAP_PRIVATE, -1, 0),
        MAP_FAILED);
    RIRU_LOGD("%" PRIxPTR "-%" PRIxPTR " %s %ld %s is backup to %" PRIxPTR, start, end, procstruct->perm,
              procstruct->offset,
              procstruct->pathname, data->backup_address);

    if (!procstruct->is_r) {
        RIRU_LOGD("mprotect +r");
        FAILURE_RETURN(mprotect((void*)start, length, prot | PROT_READ), -1);
    }
    RIRU_LOGD("memcpy -> backup");
    memcpy((void*)data->backup_address, (void*)start, length);

    // munmap original
    RIRU_LOGD("munmap original");
    FAILURE_RETURN(munmap((void*)start, length), -1);

    // restore
    RIRU_LOGD("mmap original");
    FAILURE_RETURN(mmap((void*)start, length, prot, MAP_ANONYMOUS | MAP_PRIVATE, -1, 0),
                   MAP_FAILED);
    RIRU_LOGD("mprotect +w");
    FAILURE_RETURN(mprotect((void*)start, length, prot | PROT_WRITE), -1);
    RIRU_LOGD("memcpy -> original");
    memcpy((void*)start, (void*)data->backup_address, length);
    if (!procstruct->is_w) {
        RIRU_LOGD("mprotect -w");
        FAILURE_RETURN(mprotect((void*)start, length, prot), -1);
    }
    return 0;
}

int riru_hide(const char* name) {
    procmaps_iterator* maps = pmparser_parse(-1);
    if (maps == nullptr) {
        RIRU_LOGE("cannot parse the memory map");
        return false;
    }

    char buf[PATH_MAX];
    hide_struct* data = nullptr;
    size_t data_count = 0;
    procmaps_struct* maps_tmp;
    while ((maps_tmp = pmparser_next(maps)) != nullptr) {
        if (strstr(maps_tmp->pathname, name) == 0) {
            continue;
        }

        auto start = (uintptr_t)maps_tmp->addr_start;
        auto end = (uintptr_t)maps_tmp->addr_end;
        if (maps_tmp->is_r) {
            if (data) {
                data = (hide_struct*)realloc(data, sizeof(hide_struct) * (data_count + 1));
            }
            else {
                data = (hide_struct*)malloc(sizeof(hide_struct));
            }
            data[data_count].original = maps_tmp;
            data_count += 1;
        }
        RIRU_LOGD("%" PRIxPTR "-%" PRIxPTR " %s %ld %s", start, end, maps_tmp->perm, maps_tmp->offset,
                  maps_tmp->pathname);
    }

    for (int i = 0; i < data_count; ++i) {
        do_hide(&data[i]);
    }

    if (data) {
        free(data);
    }
    pmparser_free(maps);
    return 0;
}