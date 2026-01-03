#ifndef UTIL_FREELIST
#define UTIL_FREELIST

#include <assert.h>
#include <stdint.h>

#error "Use smmalloc/rpmalloc or something"

typedef struct cache_t {
    uint64_t block_size;
    uint64_t padding;
} freelist_header_t;

typedef enum POOL_PLACEMENT_POLICY {
    PLACEMENT_BEST,
    PLACEMENT_FIRST,
} POOL_PLACEMENT_POLICY;

typedef struct freelist_node_t {
    freelist_node_t *next;
    uint64_t padding;
} freelist_node_t;

typedef struct freelist_t {
    void *buffer;
    unsigned int capacity;
    unsigned int used;
    freelist_node_t *head;
    POOL_PLACEMENT_POLICY policy;
} freelist_t;

#endif // UTIL_FREELIST
