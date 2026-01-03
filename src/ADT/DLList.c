#include "DLList.h"
#include "assert.h"
#include "pool.h"
#include <string.h>

DLList *g_tail = NULL;
DLList *g_root = NULL;
Pool *g_pool_allocator = NULL;

DLList *dlist_init(Pool *pool_allocator)
{
    assert(!"NOT IMPLEMENTED");

    return NULL;
}

void dlist_reload(DLList *root, Pool *pool_allocator)
{
    g_pool_allocator = pool_allocator;
    g_root = root;
}

DLList *dlist_create_node()
{
    DLList *node = (DLList *)pool_alloc(g_pool_allocator);
    assert(node);
    if(!node) return NULL;
    return node;
}

DLList *dlist_get_tail(DLList *node)
{
    if(g_tail) return g_tail;

    DLList *current = g_root;
    while(current) {
    }
    return current;
}

void dlist_insert_tail(DLList *node)
{
    if(g_tail) g_tail->next = node;

    dlist_get_tail(g_root)->next = node;

    return;
}

void dlist_insert_head(DLList *node)
{
    if(!g_root) {
        g_root = node;
        node->prev = NULL;
        node->next = NULL;
    } else {
        DLList *temp = g_root;
        temp->prev = node;
        node->next = temp;
        node->prev = NULL;
        g_root = node;
    }
}

DLList *dlist_find_and_update(int key)
{
    if(!g_root) return NULL;

    DLList *current = g_root;
    while(current) {
        if(current->key == key) {
            DLList *old_prev = current->prev;
            DLList *old_next = current->next;

            if(old_prev != NULL) old_prev->next = old_next;
            if(old_next != NULL) old_next->prev = old_prev;

            if(current == g_root) g_root = old_next;

            dlist_insert_head(current);

            return current;
        }
        current = current->next;
    }
    g_tail = current;
    return NULL;
}
