#ifndef _STACK_H
#define _STACK_H

#include <stdint.h>

struct StackAllocator {
  static StackAllocator *create_in_buffer(void *buffer, uint64_t buffer_length,
                                          uint64_t alignment = sizeof(void *));
  static StackAllocator *create_on_heap(uint64_t buffer_length,
                                        uint64_t alignment = sizeof(void *));

  void *allocate(uint64_t alloc_size, uint64_t align = sizeof(void *));
  void deallocate(void *ptr);
  void clear();

  void *resize(void *ptr, uint64_t old_size, uint64_t new_size,
               uint64_t alignment);

  uint32_t get_allocated();
  uint32_t get_free();

  int32_t align_ptr();
  bool is_power_of_two(uint64_t x);
  uint64_t calc_padding_with_header(uint64_t ptr, uint64_t alignment,
                                    uint64_t header_size);
  StackAllocator(uint64_t buffer_length, uint64_t alignment = sizeof(void *));
  ~StackAllocator();

  void *m_buffer;
  uint64_t m_buffer_length;
  uint64_t m_offset;
  uint64_t m_alignment;
  uint64_t m_previous_offset;
  void *m_raw_allocation;

  struct StackHeader {
    uint64_t m_previous_offset;
    uint64_t pad;
  };
};

#endif // !_STACK_H
