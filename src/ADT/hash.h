#ifndef UTIL_HASH
#define UTIL_HASH

#include <stdint.h>

typedef struct hash_entry {
  char *key;
  uint64_t value;
  struct hash_entry *next;
} hash_entry_t;

struct arena_t;

typedef struct hash_map_t {
  arena_t *allocator;
  uint32_t size;
  uint32_t hash_map_size;
  hash_entry_t **buckets;
} hash_map_t;

uint32_t hash_string(const char *str);

void hash_table_create(hash_map_t *table, uint32_t size);
uint32_t hash_table_insert(hash_map_t *table, const char *key, uint64_t value,
                           arena_t *arena);

uint32_t hash_table_find_by_string(hash_map_t *table, const char *key,
                                   uint64_t *value_out);
uint32_t hash_table_find_by_u32(hash_map_t *table, uint32_t,
                                uint64_t *value_out);

#endif // UTIL_HASH
