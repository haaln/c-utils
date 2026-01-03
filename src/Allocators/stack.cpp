#include "stack.h"

#include <cassert>
#include <new>
#include <stdlib.h>
#include <string.h>

#define CLEAN_MEMORY(buffer, size) memset(buffer, 0, size)
#define POISON_MEMORY(buffer, size) memset(buffer, 0xDD, size)

static inline size_t align_forward(size_t value, size_t align) {
  assert(align && ((align & (align - 1)) == 0) &&
         "Memory is not aligned to power of two");
  return (value + align - 1) & ~(align - 1);
}

static inline bool is_power_of_two(size_t x) { return (x & (x - 1)) == 0; }

static inline uintptr_t mem_align_forward(uintptr_t addr, uint32_t align) {
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

StackAllocator *StackAllocator::create_in_buffer(void *buffer,
                                                 uint64_t bufferLength,
                                                 uint64_t alignment) {
  uint64_t base = (uint64_t)buffer;
  uint64_t aligned = mem_align_forward(base, alignment);

  uint64_t alignment_offset = aligned - base;
  uint64_t header_size = align_forward(sizeof(StackAllocator), alignment);
  uint64_t required_space = alignment_offset + header_size;

  assert(bufferLength >= required_space &&
         "Buffer too small for header + alignment");

  uint8_t *arena_start = (uint8_t *)aligned + header_size;
  uint64_t stack_capacity = bufferLength - required_space;

#ifndef NDEBUG
  CLEAN_MEMORY((uint8_t *)arena_start, stack_capacity);
#endif

  StackAllocator *allocator =
      new ((void *)aligned) StackAllocator(stack_capacity, alignment);

  allocator->m_buffer = arena_start;
  allocator->m_rawAllocation = nullptr;

  allocator->clear();

  return allocator;
}

StackAllocator *StackAllocator::create_on_heap(uint64_t bufferLength,
                                               uint64_t alignment) {
  uint64_t header_size = align_forward(sizeof(StackAllocator), alignment);
  uint64_t total_required = header_size + bufferLength;

  void *raw = malloc(total_required);
  assert(raw);

  StackAllocator *allocator = new (raw) StackAllocator(bufferLength, alignment);

  uint8_t *buffer_memory = (uint8_t *)allocator + header_size;
  allocator->m_buffer = buffer_memory;
  allocator->m_rawAllocation = raw;

  allocator->clear();

  return allocator;
}

StackAllocator::StackAllocator(uint64_t bufferLength, uint64_t alignment) {
  m_bufferLength = bufferLength;
  m_alignment = alignment;
  m_offset = 0;
  m_previousOffset = 0;
}

uint64_t StackAllocator::calc_padding_with_header(uint64_t ptr,
                                                  uint64_t alignment,
                                                  uint64_t headerSize) {
  uint64_t p;
  uint64_t a;
  uint64_t modulo;
  uint64_t padding;
  uint64_t neededSpace;

  assert(is_power_of_two(alignment));

  p = ptr;
  a = alignment;
  modulo = p & (a - 1); // (p % a) as it assumes alignment is a power of two

  padding = 0;
  neededSpace = 0;

  if (modulo != 0) { // Same logic as 'align_forward'
    padding = a - modulo;
  }

  neededSpace = (uint64_t)headerSize;

  if (padding < neededSpace) {
    neededSpace -= padding;

    if ((neededSpace & (a - 1)) != 0) {
      padding += a * (1 + (neededSpace / a));
    } else {
      padding += a * (neededSpace / a);
    }
  }

  return (uint64_t)padding;
}

void *StackAllocator::allocate(uint64_t alloc_size, uint64_t align) {
  uint64_t currentAddress;
  uint64_t nextAddress;
  uint64_t padding;
  StackHeader *header;

  assert(is_power_of_two(align));

  if (align > 128) {
    // As the padding is 8 bits (1 byte), the largest alignment that can
    // be used is 128 bytes
    align = 128;
  }

  currentAddress = (uint64_t)m_buffer + (uint64_t)m_offset;
  padding = calc_padding_with_header(currentAddress, (uint64_t)align,
                                     sizeof(StackHeader));
  if (m_offset + padding + alloc_size > m_bufferLength) {
    // Stack allocator is out of memory
    return NULL;
  }
  m_offset += padding;

  nextAddress = currentAddress + (uint64_t)padding;
  header = (StackHeader *)(nextAddress - sizeof(StackHeader));
  header->pad = (uint8_t)padding;

  m_offset += alloc_size;

  return memset((void *)nextAddress, 0, alloc_size);
}

void StackAllocator::deallocate(void *ptr) {
  if (ptr != NULL) {
    uint64_t start, end, currentAddress;
    StackHeader *header;
    uint64_t previousOffset;

    start = (uint64_t)m_buffer;
    end = start + (uint64_t)m_bufferLength;
    currentAddress = (uint64_t)ptr;

    if (!(start <= currentAddress && currentAddress < end)) {
      assert(0 &&
             "Out of bounds memory address passed to stack allocator (free)");
      return;
    }

    if (currentAddress >= start + (uint64_t)m_offset) {
      // Allow double frees
      return;
    }

    header = (StackHeader *)(currentAddress - sizeof(StackHeader));
    previousOffset = (uint64_t)(currentAddress - (uint64_t)header->pad - start);

    m_offset = previousOffset;
  }
}

uint32_t StackAllocator::get_allocated() { return m_offset; }
uint32_t StackAllocator::get_free() { return -1; }

void StackAllocator::clear() { m_offset = 0; }

int32_t StackAllocator::align_ptr() { return 0; }

bool StackAllocator::is_power_of_two(uint64_t x) { return (x & (x - 1)) == 0; }

void *StackAllocator::resize(void *ptr, uint64_t oldSize, uint64_t newSize,
                             uint64_t align) {
  if (ptr == NULL) {
    return allocate(newSize, align);
  } else if (newSize == 0) {
    deallocate(ptr);
    return NULL;
  } else {
    uint64_t start;
    uint64_t end;
    uint64_t currentAddress;

    uint64_t minimumSize = oldSize < newSize ? oldSize : newSize;
    void *newPtr;

    start = (uint64_t)m_buffer;
    end = start + (uint64_t)m_bufferLength;
    currentAddress = (uint64_t)ptr;
    if (!(start <= currentAddress && currentAddress < end)) {
      assert(0 &&
             "Out of bounds memory address passed to stack allocator (resize)");
      return NULL;
    }

    if (currentAddress >= start + (uint64_t)m_offset) {
      // Treat as a double free
      return NULL;
    }

    if (oldSize == newSize) {
      return ptr;
    }

    newPtr = allocate(newSize, align);
    memmove(newPtr, ptr, minimumSize);
    return newPtr;
  }
}
