#include "hash.h"
#include <cassert>

// TODO: custom allocator macro
#ifndef UTIL_HASH_ALLOCATOR
#include "arena.h"
#endif

#include "hash.h"
#include <string.h>

uint32_t hash_string(const char *str)
{
    uint32_t hash = 5381;
    int c;
    while((c = *str++)) {
        hash = ((hash << 5) + hash) + c;
    }
    return hash % HASH_TABLE_SIZE;
}

void hash_table_init(hash_map_t *table)
{
    memset(table->buckets, 0, sizeof(table->buckets));
}

int hash_table_insert(hash_map_t *table, const char *key, uint16_t value, arena_t *arena)
{
    uint32_t index = hash_string(key);
    hash_entry_t *current = table->buckets[index];
    while(current) {
        if(strcmp(current->key, key) == 0) {
            assert(false); // Duplicate key found
            current->value = value;
            return -1;
        }
        current = current->next;
    }

    hash_entry_t *entry = (hash_entry_t *)arena_alloc(arena, sizeof(hash_entry_t));
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

int hash_table_find(hash_map_t *table, const char *key, uint16_t *value)
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
