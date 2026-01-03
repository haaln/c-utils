#include "pool.h"

#include <assert.h>
#include <malloc.h>
#include <new>
#include <string.h>

static size_t align_forward(size_t value, size_t align) {
  assert(align && ((align & (align - 1)) == 0) &&
         "Memory is not aligned to power of two");
  return (value + align - 1) & ~(align - 1);
}

static bool is_power_of_two(size_t x) { return (x & (x - 1)) == 0; }

static uintptr_t mem_align_forward(uintptr_t addr, uint32_t align) {
  uintptr_t p, a, modulo;
  // const uint64_t mask = align - 1;

  assert(is_power_of_two(align));

  p = addr;
  a = (uintptr_t)align;
  modulo = p & (a - 1);

  if (modulo != 0) {
    p += a - modulo;
  }
  return p;
}

uint64_t PoolAllocator::align_ptr(uint64_t value, uint64_t align) {
  assert(align && ((align & (align - 1)) == 0) &&
         "Alignment must be power of 2");
  return (value + align - 1) & ~(align - 1);
}

bool PoolAllocator::is_power_of_two(uint64_t alignment) {
  return (alignment & (alignment - 1)) == 0;
}

void PoolAllocator::init(void *memory) { clear(); }

PoolAllocator::PoolAllocator(uint64_t chunk_size, uint64_t chunk_count,
                             uint64_t alignment) {
  m_chunk_size = chunk_size;
  m_chunk_count = chunk_count;
  m_alignment = alignment;
  m_capacity = chunk_size * chunk_count;
  m_head = nullptr;
  m_chunks_in_use = 0;
}

PoolAllocator *PoolAllocator::create_on_heap(uint64_t chunk_size,
                                             uint64_t chunk_count,
                                             uint64_t alignment) {
  assert(chunk_size >= sizeof(FreeList));
  assert(chunk_count > 0);

  uint64_t header_size = align_forward(sizeof(PoolAllocator), alignment);
  uint64_t pool_size = chunk_size * chunk_count;
  uint64_t total_size = header_size + pool_size;

  void *raw = malloc(total_size);
  assert(raw);

#ifndef NDEBUG
  memset((void *)raw, 0, pool_size);
#endif

  PoolAllocator *allocator =
      new (raw) PoolAllocator(chunk_size, chunk_count, alignment);

  uint8_t *pool_memory = (uint8_t *)raw + header_size;

  allocator->m_buffer = (char *)pool_memory;
  allocator->m_raw_allocation = raw;

  allocator->clear();

  return allocator;
}

PoolAllocator *PoolAllocator::create_in_buffer(uint64_t chunk_size,
                                               uint64_t chunk_count,
                                               void *user_buffer,
                                               uint64_t buffer_length,
                                               uint64_t alignment) {
  assert(user_buffer);

  uint64_t base = (uint64_t)user_buffer;
  uint64_t aligned = mem_align_forward(base, alignment);
  uint64_t alignment_offset = aligned - base;

  uint64_t header_size = align_forward(sizeof(PoolAllocator), alignment);
  uint64_t pool_size = chunk_size * chunk_count;
  uint64_t total_required = header_size + pool_size + alignment_offset;

  assert(buffer_length >= total_required);

#ifndef NDEBUG
  memset((void *)aligned, 0, pool_size);
#endif

  PoolAllocator *allocator =
      new ((void *)aligned) PoolAllocator(chunk_size, chunk_count, alignment);
  allocator->m_buffer = (uint8_t *)user_buffer + header_size;
  allocator->m_raw_allocation = nullptr;
  allocator->m_user_buffer = user_buffer;

  allocator->clear();

  return allocator;
}

void *PoolAllocator::allocate() {
  if (!m_buffer) {
    assert(false && "pool_alloc: NULL or uninitialized pool\n");
    return nullptr;
  }

  if (!m_head) {
    assert(false && "pool_alloc: out of memory");
    return nullptr;
  }

  FreeList *block = m_head;

  m_head = m_head->next;
  m_chunks_in_use++;

  return block;
}

/**
 * clears all used memory and builds freelists.
 */
void PoolAllocator::clear() {
  assert(m_buffer);
  assert(m_chunk_size >= sizeof(FreeList));
  assert(m_chunk_count > 0);

#ifndef NDEBUG
  memset((void *)m_buffer, 0, m_capacity);
#endif

#if 0
    // using pointer arithmetics
    uint64_t chunk_count = pool->capacity / pool->chunk_size;
    for(int i = 0; i < chunk_count; i++) {
        void *ptr = &pool->buffer[i * pool->chunk_size];
        FreeList *block = (FreeList *)ptr;
        block->next = pool->head;
        pool->head = block;
    }
#else
  uint8_t *chunk = (uint8_t *)m_buffer;

  for (uint64_t i = 0; i < m_chunk_count; i++) {
    FreeList *block = (FreeList *)chunk;
    block->next = m_head;
    m_head = block;
    chunk += m_chunk_size;
  }
#endif

  m_chunks_in_use = 0;
}

void PoolAllocator::deallocate(void *ptr) {
  assert(ptr && "Invalid pointer address");

  uint64_t buffer_start = (uint64_t)m_buffer;
  uint64_t buffer_end = buffer_start + m_capacity;

  assert(buffer_start <= (uint64_t)ptr && (uint64_t)ptr < buffer_end &&
         "Memory does not belong to this pool allocator");

  uint64_t offset = (uint64_t)ptr - buffer_start;
  if (offset % m_chunk_size != 0) {
    assert(false && "pool_free: pointer is not aligned to chunk boundary "
                    "(corrupted pointer?)");
  }

  FreeList *block = (FreeList *)ptr;
  block->next = m_head;
  m_head = block;
  m_chunks_in_use--;
}

//////////////////////////////////////////////////////////////

extern "C" {

Pool *pool_init(uint64_t chunk_size, uint64_t chunk_count, void *user_buffer,
                uint64_t buffer_length, uint64_t align) {
  assert((chunk_size && chunk_count) &&
         "Chunk size is zero || chunk_count is zero");
  assert((chunk_size >= sizeof(FreeList)) &&
         "Chunk size is smaller than FreeList");
  assert((chunk_count <= SIZE_MAX / chunk_size) && "Integer overflow");

  Pool *pool = NULL;
  bool is_heap_based = false;

  if (user_buffer && buffer_length > 0) {
    // User provided buffer case

    uint64_t buffer_start = (uint64_t)user_buffer;
    // uint64_t aligned_start = align_forward(buffer_start, align);
    uint64_t aligned_start = mem_align_forward(buffer_start, align);
    uint64_t alignment_offset = aligned_start - buffer_start;

    uint64_t header_size = align_forward(sizeof(Pool), align);
    // uint64_t header_size = mem_align_forward(sizeof(Pool), align);
    uint64_t pool_size = chunk_size * chunk_count;
    uint64_t total_required = header_size + pool_size + alignment_offset;

    assert((buffer_length >= total_required) && "Not enough space");

#ifndef NDEBUG
    memset((void *)aligned_start, 0, pool_size);
#endif

    pool = (Pool *)aligned_start;
    pool->buffer = (uint8_t *)pool + header_size;
    pool->raw_allocation = user_buffer;
  } else {
    // Dynamically allocate buffer

    uint64_t header_size = align_forward(sizeof(Pool), align);
    uint64_t buffer_size = chunk_size * chunk_count;
    uint64_t total_size =
        header_size + buffer_size + align - 1; // Extra for alignment

    uint8_t *memory = (uint8_t *)malloc(total_size);
    assert(memory && "malloc returned NULL");
    memset(memory, 0, total_size);

    uint64_t aligned_start = align_forward((uint64_t)memory, align);

    pool = (Pool *)aligned_start;
    pool->raw_allocation = memory;
    pool->buffer = (uint8_t *)pool + header_size;
    is_heap_based = true;
  }

  // Initialize pool structure
  pool->chunk_count = chunk_count;
  pool->capacity = chunk_size * chunk_count;
  pool->chunk_size = chunk_size;
  pool->head = NULL;
  pool->is_heap_based = is_heap_based;
  pool->chunks_in_use = 0;

  // Build the initial free list
  pool_free_all(pool);

  return pool;
}

void pool_free_all(Pool *pool) {
  assert(pool);
  assert(pool->buffer);
  assert(pool->chunk_size >= sizeof(FreeList));
  assert(pool->chunk_count > 0);

#ifndef NDEBUG
  memset((void *)pool->buffer, 0, pool->capacity);
#endif

#if 0
    // using pointer arithmetics
    uint64_t chunk_count = pool->capacity / pool->chunk_size;
    for(int i = 0; i < chunk_count; i++) {
        void *ptr = &pool->buffer[i * pool->chunk_size];
        FreeList *block = (FreeList *)ptr;
        block->next = pool->head;
        pool->head = block;
    }
#else
  uint8_t *chunk = pool->buffer;

  for (uint64_t i = 0; i < pool->chunk_count; i++) {
    FreeList *block = (FreeList *)chunk;
    block->next = pool->head;
    pool->head = block;
    chunk += pool->chunk_size;
  }
#endif

  pool->chunks_in_use = 0;
}

void pool_free(Pool *pool, void *ptr) {
  if (!ptr || !pool) {
    assert(false && "pool_free: pool or pointer is NULL");
    return;
  }

  uint64_t buffer_start = (uint64_t)pool->buffer;
  uint64_t buffer_end = buffer_start + pool->capacity;
  assert(buffer_start <= (uint64_t)ptr && (uint64_t)ptr < buffer_end &&
         "Memory does not belong to this pool allocator");

  uint64_t offset = (uint64_t)ptr - buffer_start;
  if (offset % pool->chunk_size != 0) {
    assert(false && "pool_free: pointer is not aligned to chunk boundary "
                    "(corrupted pointer?)");
    return;
  }

  FreeList *block = (FreeList *)ptr;
  block->next = pool->head;
  pool->head = block;
  pool->chunks_in_use--;

  // #ifndef NDEBUG
  //     POISON_MEMORY(block, pool->chunk_size);
  // #endif

#if 0
    // TODO: later. (or maybe have two versions, one mt_, and one is thread-local)
    // threading version
    do {

    } while(!CAS)

#endif
}

void *pool_alloc(Pool *pool) {
  if (!pool || !pool->buffer) {
    assert(false && "pool_alloc: NULL or uninitialized pool\n");
    return nullptr;
  }

  if (!pool->head) {
    assert(false && "pool_alloc: out of memory");
    return nullptr;
  }

  FreeList *block = pool->head;

  pool->head = pool->head->next;
  pool->chunks_in_use++;

  return block;
}

void pool_destroy(Pool *pool) {
  if (!pool)
    return;

#ifndef NDEBUG
  if (pool->chunks_in_use != 0) {
    assert(false && "pool_destroy: memory leak! Not every chunk is free");
  }
  if (pool->head && pool->chunk_count > 0) {
    // walk the freelist and count, if it doesnt match, do something
    // or poison/clean the memory
  }
#endif

  if (pool->is_heap_based) {
#if 0 // pointer arithmetic
      // uint64_t pool_addr = (uint64_t)pool;
      // uint64_t header_size = align_forward_size(sizeof(Pool), sizeof(void
      // *));
#ifndef NDEBUG
        POISON_MEMORY(pool->raw_allocation, pool->capacity);
#endif
      SDL_free(void*(pool_addr + header_size)); // FIXME: wrong.
#else

    free(pool->raw_allocation);
#endif
  } else {
    memset(pool->raw_allocation, 0xDD, pool->capacity);
  }
}
} // extern "C"
