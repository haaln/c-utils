#ifndef HASH_H
#define HASH_H

#define HASH_TABLE_SIZE 256

struct Arena;
struct ArenaAllocator;

#include <stdint.h>

typedef struct hash_entry_t {
    char *key;
    uint32_t value;
    hash_entry_t *next; // for collision
} hash_entry_t;

typedef struct hash_map_t {
    hash_entry_t *buckets[HASH_TABLE_SIZE];
    Arena *allocator;
    uint32_t next_handle = 1;
    uint32_t count = 0; // Total unique hash entries.
} hash_map_t;

extern "C" {

void hash_table_init(hash_map_t *table, Arena *arena);

int32_t hash_table_insert(hash_map_t *table, const char *key, uint16_t value);

int32_t hash_table_find(hash_map_t *table, const char *key, uint16_t *value);

#endif // HASH_H
