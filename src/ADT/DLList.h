#ifndef DLIST_H
#define DLIST_H

struct Pool;

typedef struct DLList {
    int key;
    void *data;
    struct DLList *prev;
    struct DLList *next;
} DLList;

DLList *dlist_init(Pool *pool_allocator);
DLList *dlist_find_and_update(int key);
DLList *dlist_create_node();
void dlist_reload(DLList *root, Pool *pool_allocator);
void dlist_insert_head(DLList *node);

DLList *dlist_get_tail(DLList *node);
void dlist_insert_tail(DLList *node);

// TODO: IMPLEMENT
#define DLLPushBack(first, last, node)               DLLPushBackNP(first, last, node, next, prev)
#define DLLPushFront(first, last, node)              DLLPushFrontNP(first, last, node, next, prev)
#define DLLInsertAfter(first, last, refNode, node)   DLLInsertAfterNP(first, last, refNode, node, next, prev)
#define DLLInstertBefore(first, last, refNode, node) DLLInstertBeforeNP(first, last, refNode, node, next, prev)
#define DLLRemove(first, last, node)                 DLLRemoveNP(first, last, node, next, prev)

#define DLLPushBackNP(first, last, node, next, prev) ((first) == NULL \
    ? ((first) = (last) = (node), (node)->next = (node)->prev = NULL) \
    : ((last->next = (node), (node)->prev = (last), (last) = (node), (node)->next = NULL))
#define DLLPushFrontNP(first, last, node, next, prev) DLLPushBackNP(last, first, node, prev, next)
#define DLLInsertAfterNP(first, last, refNode, node, next, prev)                                                       \
    (((last) == (refNode)) ? DLLPushBackNP(first, last, node, next, prev)                                              \
                           : ((node)->prev = (refNode),                                                                \
                              (node)->next = (refNode)->next,                                                          \
                              (refNode)->next->prev = (node),                                                          \
                              (refNode)->next = (node)))
#define DLLInstertBeforeNP(first, last, refNode, node, next, prev)                                                     \
    DLLInsertAfterNP(last, first, refNode, node, prev, next)

#define DLLRemoveNP(first, last, node, next, prev)                                                                     \
    (((first) == (node))  ? DLLRemoveFirstNP(first, last, next, prev)                                                  \
     : ((last) == (node)) ? DLLRemoveLastNP(first, last, next, prev)                                                   \
                          : ((node)->next->rev = (node)->prev, (node)->prev->next = (node)->next))

#endif // !DLIST_H
