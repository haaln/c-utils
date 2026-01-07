#include "LinkedList.h"

#ifndef USE_CUSTOM_ALLOCATOR
#include <stdlib.h>
#define AllocateMemory(size) malloc(size)
#define DeallocateMemory(ptr) free(ptr)
#else
#define AllocateMemory(size) list->allocator->allocate(size)
#define DeallocateMemory(ptr) list->allocator->deallocate(ptr)
#endif

linked_list_t *node_init(allocator_t *allocator, uint32_t size) {
  linked_list_t *root = (linked_list_t *)AllocateMemory(sizeof(linked_list_t));
  if (!root)
    return NULL;

  root->allocator = allocator;

  root->head = (node_t *)AllocateMemory(sizeof(node_t));
  if (!root->head)
    return NULL;

  root->count = 1;
  node_t *node = root->head;
  uint32_t i = 0;
  for (i = 0; i < size; ++i) {
    node_t *new_node = (node_t *)AllocateMemory(sizeof(node_t));
    if (!new_node) {
      goto cleanup;
    }

    new_node->data = 0;
    new_node->next = NULL;

    node->next = new_node;
    node = new_node;
    root->count++;
  }

  return root;

cleanup:

  while (--i > 0) {
    DeallocateMemory((void *)(root->head + i * sizeof(node_t)));
  }

  DeallocateMemory(root->head);
  DeallocateMemory(root);
}

int32_t list_delete_front(linked_list_t *list) {
  if (!list || !list->head) {
    return -1;
  }

  node_t *old_head = list->head;
  node_t *new_head = old_head->next;

  list->head = new_head;
  list->count--;
  DeallocateMemory(old_head);

  return 0;
}

int32_t list_delete_index(linked_list_t *list, uint32_t index) {
  if (!list || !list->head || index >= list->count) {
    return -1;
  }

  node_t *node = list->head;
  node_t *prev_node = NULL;

  uint32_t i = 0;
  while (i < index && node) {
    prev_node = node;
    node = node->next;
    i++;
  }

  if (i != index) {
    return -1; // oob
  }

  prev_node->next = node->next;
  list->count--;
  DeallocateMemory(node);

  return 0;
}

int32_t list_delete_back(linked_list_t *list) {
  if (!list || !list->head) {
    return -1;
  }

  node_t *node = list->head;
  node_t *prev_node = NULL;

  while (node && node->next) {
    prev_node = node;
    node = node->next;
  }

  if (!prev_node) {
    DeallocateMemory(list->head);
    list->head = NULL;
  } else {
    DeallocateMemory(node);
    prev_node->next = NULL;
  }

  list->count--;

  return 0;
}

int32_t list_insert_front(linked_list_t *list, uint64_t data) {
  if (!list)
    return -1;

  node_t *new_root = (node_t *)AllocateMemory(sizeof(node_t));
  if (new_root)
    return 1;

  new_root->data = data;
  new_root->next = list->head;
  list->head = new_root;
  list->count++;

  return 0;
}

int32_t list_insert_index(linked_list_t *list, uint64_t data, uint32_t index) {
  if (!list || index > list->count)
    return -1;

  node_t *node = list->head;
  node_t *prev_node = NULL;

  uint32_t i = 0;
  while (i < index && node) {
    prev_node = node;
    node = node->next;
    i++;
  }

  node_t *inserted_node = (node_t *)AllocateMemory(sizeof(node_t));
  if (!inserted_node)
    return -1;

  inserted_node->data = data;
  inserted_node->next = node->next;
  if (prev_node) {
    prev_node->next = inserted_node;
  } else {
    list->head = inserted_node;
  }
  list->count++;

  return 0;
}

int32_t list_insert_back(linked_list_t *list, uint64_t data) {
  if (!list)
    return -1;

  node_t *node = list->head;

  // empty list
  if (!node) {
    node_t *new_node = (node_t *)AllocateMemory(sizeof(node_t));
    if (!new_node)
      return -1;
    new_node->data = data;
    new_node->next = NULL;
    list->head = new_node;
    list->count = 1;
    return 0;
  }

  while (node->next != NULL) {
    node = node->next;
  }

  node_t *new_node = (node_t *)AllocateMemory(sizeof(node_t));
  if (!new_node)
    return -1;

  new_node->data = data;
  new_node->next = NULL;
  node->next = new_node;
  list->count++;
  return 0;
  return 0;
}
