#ifndef UTIL_ARENA_H
#define UTIL_ARENA_H

#include <stdint.h>

#ifndef __cplusplus
#error "CXX Necessary for default paramaters"
#endif

typedef struct arena_t {
  char *buffer = nullptr;
  uint64_t capacity = 0;
  uint64_t offset = 0;
  uint64_t prev_offset = 0;
  uint64_t alignment = 0;
} arena_t;

void *arena_alloc(arena_t *arena, uint64_t size,
                  uint64_t align = (sizeof(void *)));
arena_t *arena_init(uint64_t size, arena_t *arena = nullptr,
                    char *user_buffer = nullptr,
                    uint64_t align = (sizeof(void *)));
void arena_clear(arena_t *arena);
void arena_destroy(arena_t *arena);

#define ArenaPush(arena, size) arena_alloc(arena, size, sizeof(void *))
#define ArenaPushStruct(arena, type)                                           \
  (type *)arena_alloc(arena, sizeof(type), alignof(type))
#define ArenaPushArray(arena, count, type)                                     \
  arena_alloc(arena, (count) * sizeof(type), alignof(type[1]))

#endif // UTIL_ARENA_H
