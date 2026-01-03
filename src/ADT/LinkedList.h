#ifndef UTIL_LINKED_LIST_H
#define UTIL_LINKED_LIST_H

#include <stdlib.h>

#ifdef CXX_CLASS_IMPL
#error "NOT FINISHED"
template<class T> class LinkedList
{
  public:
    struct Node {
        T data;
        Node *next;
    };

    Node *head;

  public:
    LinkedList() = default;
    LinkedList(LinkedList &stackLinkedList) = delete;
    void push(Node *newNode);
    Node *pop();
};

template<class T> void LinkedList<T>::push(Node *newNode)
{
    newNode->next = head;
    head = newNode;
}

template<class T> typename LinkedList<T>::Node *LinkedList<T>::pop()
{
    Node *top = head;
    head = head->next;
    return top;
}

#else

typedef struct treenode_t {
    int val;
    treenode_t *prev;
    treenode_t *next;
} treenode_t;

treenode_t *node_add(int val);
int node_delete_front();
int node_delete_index(int index);
int node_delete_back();
int node_insert_front(int val);
int node_insert_index(int val, int index);
int node_insert_back(int val);

#endif // CXX_CLASS_IMPL

#endif // UTIL_LINKED_LIST_H
