#ifndef BUDDY_ALLOC_H
#define BUDDY_ALLOC_H

#include <stddef.h>
#include <stdint.h>

#ifdef CXX_CLASS_IMPL
class BuddyAllocator
{
  public:
    BuddyAllocator();
    ~BuddyAllocator();

    BuddyAllocator(BudyAllocator &&other) = delete;
    BuddyAllocator &&operator=(BudyAllocator &&other) = delete;

  private:
    size_t size;
};
#endif

typedef struct buddy_t {
    ;
} buddy_t;

size_t buddy_sizeof(size_t mem_size);
size_t buddy_aloc(uint8_t *addr, uint8_t *main, size_t mem_size);

#endif // !BUDDY_ALLOC_H
