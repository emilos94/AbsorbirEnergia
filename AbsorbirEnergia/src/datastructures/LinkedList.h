#ifndef LINKED_LIST_H
#define LINKED_LIST_H

#include "../core/memory_arena.h"
#include "../core/definitions.h"

// NOTE: 
// this list does not take ownership, but merely stores pointers
// The lifetime should be managed by an arena. Be careful to align the arena and the list.

struct LinkedListNode {
	struct LinkedListNode* next;
	struct LinkedListNode* prev;

	void* data;
};
typedef struct LinkedListNode LinkedListNode;

struct LinkedList {
	LinkedListNode* first;
	LinkedListNode* last;

	LinkedListNode* node_free_list;

	u32 count;
};
typedef struct LinkedList LinkedList; 

LinkedList* ds_linkedlist_create(MemoryArena* arena);
u32 ds_linkedlist_count(LinkedList* list);
void ds_linkedlist_pushback(MemoryArena* arena, LinkedList* list, void* data);
void* ds_linkedlist_popback(LinkedList* list);

void ds_linkedlist_remove(LinkedList* list, void* data);// if the same pointer exists multiple times in the list, the first will be removed

#define ds_linkedlist_loop(list, type, name, node_name)\
	LinkedListNode* node_name = list->first;\
	type* name = node_name != 0 ? node_name->data : 0;\
	for (;node_name != 0 && name != 0; node_name = node_name->next, name = node_name != 0 ? node_name->data : 0)

#endif // !LINKED_LIST_H
