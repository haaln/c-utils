#ifndef UTIL_HASH
#define UTIL_HASH

#include <stdint.h>

#ifndef HASH_TABLE_SIZE
#define HASH_TABLE_SIZE 256
#endif

typedef struct hash_entry {
    char *key;
    uint16_t value;
    struct hash_entry *next;
} hash_entry_t;

typedef struct hash_map_t {
    hash_entry_t *buckets[HASH_TABLE_SIZE];
} hash_map_t;

struct arena_t;

extern "C" {

// TODO: custom allocator as parameter (not just arena allocator)
// i.e. alloc_free && alloc_mem
uint32_t hash_string(const char *str);
void hash_table_init(hash_map_t *table);
int hash_table_insert(hash_map_t *table, const char *key, uint16_t value, arena_t *arena);
int hash_table_find(hash_map_t *table, const char *key, uint16_t *value);
}
#endif // UTIL_HASH
