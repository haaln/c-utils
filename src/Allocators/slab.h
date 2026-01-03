#ifndef UTIL_BUDDY_SLAB
#define UTIL_BUDDY_SLAB

#include "malloc.h"
#include "mem.h"
#include "types.h"

/*
 * Aim for 12.5% waste
 */

typedef struct slab_t {
  u32 padding;
  slab_t *next;
} slab_t;

typedef struct cache_t {
  u32 cache_size;
  slab_t *slabs[];
} cache_t;

static slab_t *slab_create(u32 slab_size);
void *slab_alloc(slab_t *slab);
void slab_free(slab_t *slab, void *ptr);

// BUDDY ALLOCATOR

typedef struct block_t {
  u32 size;
  bool is_free;
} block_t;

typedef struct buddy_t {
  block_t *head;
  block_t *tail;
  u32 align;
} buddy_t;

block_t *buddy_get_next(block_t *block);
block_t *block_split(block_t *block, u32 size);
buddy_t *buddy_init(u32 size, buddy_t *buddy = NULL, void *data = NULL,
                    u32 align = 2);
u32 buddy_block_size_required(buddy_t *block, u32 size);
block_t *buddy_block_find_best(block_t *head, block_t *tail, size_t size);
void buddy_allocator_free(buddy_t *buddy, void *data);

#endif // UTIL_BUDDY_SLAB
#ifdef UTIL_BUDDY_SLAB_IMPLEMENTATION
#undef UTIL_BUDDY_SLAB_IMPLEMENTATION

#define SLAB_SLOTS 10

static slab_t *slab_create(u32 slab_size) {
  slab_t *mem = (slab_t *)malloc((SLAB_SLOTS * slab_size) + sizeof(slab_t));
  return mem;
}

void *slab_create(u32 cache_capacity, u32 align) {
  cache_t *cache = (cache_t *)malloc(sizeof(cache_t));
  cache->slabs[0] = slab_create(cache_capacity);
  return cache;
}

void *slab_alloc(slab_t *slab) { return NULL; }
void slab_free(slab_t *slab, void *ptr) {}

// BUDDY ALLOCATOR

block_t *buddy_get_next(block_t *block) {
  return (block_t *)(uint8_t *)block + block->size;
}

buddy_t *buddy_init(u32 size, buddy_t *buddy, void *data, u32 align) {
  if (buddy && data) {
    // stack allocated

  } else {
    // dynamic allocated
    void *mem = malloc(sizeof(buddy_t) + size);
    buddy_t *buddy = (buddy_t *)malloc(sizeof(buddy_t) + size);
    block_t *data = (block_t *)(buddy + 1);

    assert((uintptr_t)data % align == 0 &&
           "data is not aligned to minimum alignment");

    buddy->align = align;
    buddy->head = data;
    buddy->head->size = size;
    buddy->head->is_free = true;
    buddy->tail = buddy_get_next(buddy->head);
  }
  return buddy;
}

block_t *block_split(block_t *block, u32 size) {
  if (!block && !size) {
    while (size < block->size) {
      u32 split = block->size >> 1;
      block->size = split;
      block = buddy_get_next(block);
      block->size = split;
      block->is_free = true;
    }
    if (size <= block->size) {
      return block;
    }
  }
  return NULL;
}

u32 buddy_block_size_required(buddy_t *block, u32 size) {
  u32 actual_size = block->align;

  size += sizeof(buddy_t);
  size = align_forward_size(size, block->align);

  while (size > actual_size) {
    actual_size <<= 1;
  }

  return actual_size;
}

void buddy_block_coalescence(block_t *head, block_t *tail) {
  for (;;) {
    // Keep looping until there are no more buddies to coalesce

    block_t *block = head;
    block_t *buddy = buddy_get_next(block);

    bool no_coalescence = true;
    while (block < tail &&
           buddy < tail) { // make sure the buddies are within the range
      if (block->is_free && buddy->is_free && block->size == buddy->size) {
        // Coalesce buddies into one
        block->size <<= 1;
        block = buddy_get_next(block);
        if (block < tail) {
          buddy = buddy_get_next(block);
          no_coalescence = false;
        }
      } else if (block->size < buddy->size) {
        // The buddy block is split into smaller blocks
        block = buddy;
        buddy = buddy_get_next(buddy);
      } else {
        block = buddy_get_next(buddy);
        if (block < tail) {
          // Leave the buddy block for the next iteration
          buddy = buddy_get_next(block);
        }
      }
    }

    if (no_coalescence) {
      return;
    }
  }
}

void *buddy_allocator_alloc(buddy_t *buddy, size_t size) {
  if (size != 0) {
    size_t actual_size = buddy_block_size_required(buddy, size);

    block_t *found =
        buddy_block_find_best(buddy->head, buddy->tail, actual_size);
    if (found == NULL) {
      // Try to coalesce all the free buddy blocks and then search again
      buddy_block_coalescence(buddy->head, buddy->tail);
      found = buddy_block_find_best(buddy->head, buddy->tail, actual_size);
    }

    if (found != NULL) {
      found->is_free = false;
      return (void *)((char *)found + buddy->align);
    }

    // Out of memory (possibly due to too much internal fragmentation)
  }

  return NULL;
}

void buddy_allocator_free(buddy_t *buddy, void *data) {
  if (data != NULL) {
    block_t *block;

    assert(buddy->head <= data);
    assert(data < buddy->tail);

    block = (block_t *)((char *)data - buddy->align);
    block->is_free = true;

    // NOTE: Coalescence could be done now but it is optional
    // buddy_block_coalescence(b->head, b->tail);
  }
}

block_t *buddy_block_find_best(block_t *head, block_t *tail, size_t size) {
  // Assumes size != 0

  block_t *best_block = NULL;
  block_t *block = head;                  // Left Buddy
  block_t *buddy = buddy_get_next(block); // Right Buddy

  // The entire memory section between head and tail is free,
  // just call 'buddy_block_split' to get the allocation
  if (buddy == tail && block->is_free) {
    return block_split(block, size);
  }

  // Find the block which is the 'best_block' to requested allocation sized
  while (block < tail &&
         buddy < tail) { // make sure the buddies are within the range
    // If both buddies are free, coalesce them together
    // NOTE: this is an optimization to reduce fragmentation
    //       this could be completely ignored
    if (block->is_free && buddy->is_free && block->size == buddy->size) {
      block->size <<= 1;
      if (size <= block->size &&
          (best_block == NULL || block->size <= best_block->size)) {
        best_block = block;
      }

      block = buddy_get_next(buddy);
      if (block < tail) {
        // Delay the buddy block for the next iteration
        buddy = buddy_get_next(block);
      }
      continue;
    }

    if (block->is_free && size <= block->size &&
        (best_block == NULL || block->size <= best_block->size)) {
      best_block = block;
    }

    if (buddy->is_free && size <= buddy->size &&
        (best_block == NULL || buddy->size < best_block->size)) {
      // If each buddy are the same size, then it makes more sense
      // to pick the buddy as it "bounces around" less
      best_block = buddy;
    }

    if (block->size <= buddy->size) {
      block = buddy_get_next(buddy);
      if (block < tail) {
        // Delay the buddy block for the next iteration
        buddy = buddy_get_next(block);
      }
    } else {
      // Buddy was split into smaller blocks
      block = buddy;
      buddy = buddy_get_next(buddy);
    }
  }

  if (best_block != NULL) {
    // This will handle the case if the 'best_block' is also the perfect fit
    return block_split(best_block, size);
  }

  // Maybe out of memory
  return NULL;
}

#endif // UTIL_BUDDY_SLAB_IMPLEMENTATION
