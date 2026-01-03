#include "LinkedList.h"

#ifdef CXX_CLASS_IMPL
#else

treenode_t *node_add(int val)
{
    treenode_t *node = (treenode_t *)malloc(sizeof(treenode_t));
    node->val = val;
    node->prev = nullptr;
    node->next = nullptr;
    return node;
}

int node_delete_front()
{
    return 0;
}

int node_delete_index(int index)
{
    return 0;
}

int node_delete_back()
{
    return 0;
}

int node_insert_front(int val)
{
    return 0;
}

int node_insert_index(int val, int index)
{
    return 0;
}

int node_insert_back(int val)
{
    return 0;
}
#endif // CXX_CLASS_IMPL
