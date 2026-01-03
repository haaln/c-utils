#ifndef UTIL_POOL_H
#define UTIL_POOL_H

#include <stdint.h>

typedef struct FreeList {
  struct FreeList *next;
} FreeList;

struct PoolAllocator {
  static PoolAllocator *
  create_in_buffer(uint64_t chunk_size, uint64_t chunk_count, void *buffer,
                   uint64_t buffer_length,
                   uint64_t alignment = alignof(PoolAllocator));
  static PoolAllocator *
  create_on_heap(uint64_t chunk_size, uint64_t chunk_count,
                 uint64_t alignment = alignof(PoolAllocator));

  void *allocate();
  void deallocate(void *ptr);
  void clear();

  // NOTE: Because C++ is such a good language,
  // we have to set m_buffer to void* and cast it to u8* to make things work.
  void *m_buffer; // Cast to u8* to perform pointer arithmetics.
  uint64_t m_capacity = 0;
  uint64_t m_peak = 0;
  uint64_t m_previous_allocation = 0;
  uint64_t m_chunk_size = 0;
  uint64_t m_alignment = 0;
  uint64_t m_chunk_count = 0;
  uint64_t m_chunks_in_use = 0;
  uint64_t m_buffer_length = 0;
  void *m_user_buffer = nullptr;
  void *m_raw_allocation = nullptr;
  FreeList *m_head = nullptr;

  uint64_t align_ptr(uint64_t value, uint64_t align);
  bool is_power_of_two(uint64_t x);
  PoolAllocator(uint64_t size, uint64_t chunk_size,
                uint64_t align = sizeof(void *));
  void init(void *pool_memory); // wrapper around clear();
};

extern "C" {

typedef struct Pool {
  uint8_t *buffer;      // memory &
  uint64_t capacity;    // pool size
  uint64_t chunk_size;  // size of one chunk
  uint64_t chunk_count; // number of chunks

  FreeList *head; // first free block;

  int16_t chunks_in_use;
  bool is_heap_based; // check if we are stack allocated or dynamic
  void *raw_allocation;
} Pool;

void pool_free_all(Pool *pool);
void pool_free(Pool *pool, void *ptr);
void *pool_alloc(Pool *pool);
void pool_destroy(Pool *pool);
Pool *pool_init(uint64_t chunk_size, uint64_t chunk_count,
                void *user_buffer = nullptr, uint64_t buffer_length = 0,
                uint64_t align = sizeof(void *));
} // extern "C"

#endif
