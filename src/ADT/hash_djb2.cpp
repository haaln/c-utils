#include "hash_djb2.h"
#include "arena.h"
#include "assert.h"
#include "log.h"

#include <string.h>

/**
 * DJB2 hash, good for hashing strings (weak hash).
 *
 * Other alternatives (but not limited to): sdbm (duff-device)
 *
 */

static uint32_t hash_string(const char *str)
{
    uint32_t hash = 5381;
    int32_t c;
    while((c = *str++)) {
        hash = ((hash << 5) + hash) + c;
    }
    return hash % HASH_TABLE_SIZE;
}

void hash_table_init(hash_map_t *table, Arena *arena)
{
    memset(table->buckets, 0, sizeof(table->buckets));
    table->allocator = arena;
}

int32_t hash_table_insert(hash_map_t *table, const char *key, uint16_t value)
{
    Arena *arena = table->allocator;

    uint32_t index = hash_string(key);
    hash_entry_t *current = table->buckets[index];
    while(current) {
        if(strcmp(current->key, key) == 0) {
            LOG_CRITICAL("Duplicate key found: %s\n", key);
            assert(!"Duplicate key found");
            current->value = value;
            return -1;
        }
        current = current->next;
    }

    hash_entry_t *entry = ArenaPushStruct(arena, hash_entry_t);
    if(!entry) return -1;

    size_t key_len = strlen(key) + 1;
    entry->key = (char *)arena_alloc(arena, key_len);
    if(!entry->key) return -1;

    strcpy(entry->key, key);
    entry->value = value;
    entry->next = table->buckets[index];
    table->buckets[index] = entry;

    return 0;
}

int32_t hash_table_find(hash_map_t *table, const char *key, uint16_t *value)
{
    uint32_t index = hash_string(key);
    hash_entry_t *entry = table->buckets[index];

    while(entry) {
        if(strcmp(entry->key, key) == 0) {
            *value = entry->value;
            return 0;
        }
        entry = entry->next;
    }
    return -1;
}

int32_t hash_insert(hash_map_t *table, const char *key, uint16_t value)
{
    return 0;
}

int32_t hash_find(hash_map_t *table, const char *key, uint16_t *value)
{
    uint32_t index = hash_string(key);
    hash_entry_t *entry = table->buckets[index];

    while(entry) {
        if(strcmp(entry->key, key) == 0) {
            *value = entry->value;
            return 0;
        }
        entry = entry->next;
    }
    return -1;
}
