#ifndef LINKED_LIST_PRIVATE_H
#define LINKED_LIST_PRIVATE_H

#include "LinkedList.h"

// Internal functions
LinkedListNode* ds_linkedlist_node_alloc(MemoryArena* arena, LinkedList* list);
LinkedListNode* ds_linkedlist_node_free(LinkedList* list, LinkedListNode* node);

#endif // !LINKED_LIST_H
