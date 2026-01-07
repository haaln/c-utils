#include "arena.h"

#ifndef USE_CUSTOM_MALLOC
#include "malloc.h"
#define AllocateMemory(size) malloc(size)
#else
#define AllocateMemory(size) MY_CUSTOM_ALLOCATOR_FUNCTION(size)
#endif

#include <assert.h>

static uint64_t align_forward(uint64_t value, uint64_t align) {
  assert(align && ((align & (align - 1)) == 0) &&
         "Memory is not aligned to power of two");
  return (value + align - 1) & ~(align - 1);
}

void *arena_alloc(arena_t *arena, uint64_t size, uint64_t align) {
  uintptr_t curr_ptr = (uintptr_t)arena->buffer + (uintptr_t)arena->offset;

  uintptr_t offset = align_forward(curr_ptr, align);
  offset -= (uintptr_t)arena->buffer;

  if (offset + size <= arena->capacity) {
    void *ptr = &arena->buffer[offset];
    arena->prev_offset = offset;
    arena->offset = offset + size;

    return ptr;
  }
  return nullptr;
}

static bool is_power_of_two(uint64_t x) { return (x & (x - 1)) == 0; }

/*
 * Initializes an arena allocator, optionally using a stack buffer.
 */
arena_t *arena_init(uint64_t size, arena_t *arena, char *user_buffer,
                    uint64_t align) {
  assert(size > 0 && "Capacity must be > 0");
  assert(is_power_of_two(align) && "align must be power of 2");

  if (arena && user_buffer) {
    // User provided buffer.

    arena->buffer = user_buffer;
    arena->offset = 0;
    arena->prev_offset = 0;
    arena->capacity = size;
    arena->alignment = align;
    return arena;

  } else if (!arena && !user_buffer) {
    // Dynamically allocate.

    uint64_t total_size = size + sizeof(arena_t);
    uint8_t *mem = (uint8_t *)AllocateMemory(total_size);
    if (!mem)
      return nullptr;

    arena_t *arena = (arena_t *)mem;
    arena->buffer = (char *)(arena + 1);
    arena->offset = 0;
    arena->prev_offset = 0;
    arena->capacity = size;
    arena->alignment = align;
    return arena;
  } else {
    assert(0 && "Invalid parameter combination");
    return nullptr;
  }
}

void arena_clear(arena_t *arena) {
  assert(arena);

  arena->offset = 0;
}
