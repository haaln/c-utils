#ifndef UTIL_LINKED_LIST_H
#define UTIL_LINKED_LIST_H

#include <stdint.h>

typedef struct node_t {
  uint64_t data;
  node_t *next;
} node_t;

struct allocator_t;
typedef struct linked_list_t {
  allocator_t *allocator;
  uint32_t count;
  node_t *head;
} linked_list_t;

linked_list_t *node_init(allocator_t *allocator, uint32_t size);
int32_t list_add(linked_list_t *list, uint64_t data);
int32_t list_delete_front(linked_list_t *list);
int32_t list_delete_index(linked_list_t *list, uint32_t index);
int32_t list_delete_back(linked_list_t *list);
int32_t list_insert_front(linked_list_t *list, uint64_t data);
int32_t list_insert_index(linked_list_t *list, uint64_t data, uint32_t index);
int32_t list_insert_back(linked_list_t *list, uint64_t data);

#endif // UTIL_LINKED_LIST_H
