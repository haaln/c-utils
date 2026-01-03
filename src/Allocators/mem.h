#ifndef UTIL_MEM
#define UTIL_MEM

#include "assert.h"

#include "../types.h"

#ifdef __linux__
#include <sys/resource.h>
#endif

#define KB(value) (value * 1024ULL)
#define MB(value) (value * 1024ULL * 1024ULL)
#define GB(value) (value * 1024ULL * 1024ULL * 1024ULL)

#define ALIGN8(value)  ((value + 7) & ~7)
#define ALIGN16(value) ((value + 15) & ~15)

static u32 align_block_size(u32 size)
{
    u32 alignment = sizeof(void *);
    return (size + alignment - 1) & ~(alignment - 1);
}

static inline u32 align_forward_size(u32 size, u32 align)
{
    return (size + align - 1) & ~(align - 1);
}

static inline uintptr_t align_forward_uintptr(uintptr_t ptr, uintptr_t align)
{
    return (ptr + align - 1) & ~(align - 1);
}

static inline bool is_power_of_two(uintptr_t x)
{
    return (x & (x - 1)) == 0;
}

static inline uintptr_t mem_align_forward(uintptr_t addr, u32 align)
{
    uintptr_t p, a, modulo;
    // const uint64_t mask = align - 1;

    assert(is_power_of_two(align));

    p = addr;
    a = (uintptr_t)align;
    modulo = p & (a - 1);

    if(modulo != 0) {
        p += a - modulo;
    }
    return p;
}

#endif // UTIL_MEM
