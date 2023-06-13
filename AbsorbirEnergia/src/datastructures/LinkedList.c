#include "LinkedListPrivate.h"

LinkedList* ds_linkedlist_create(MemoryArena* arena) {
	LinkedList* list = memory_struct_zero_allocate(arena, LinkedList);
	list->count = 0;
	list->first = 0;
	list->last = 0;
	return list;
}

u32 ds_linkedlist_count(LinkedList* list) {
	return list->count;
}

void ds_linkedlist_pushback(MemoryArena* arena, LinkedList* list, void* data) {
	LinkedListNode* node = ds_linkedlist_node_alloc(arena, list);

	if (list->first == 0) {
		list->first = node;
		list->last = node;
	}
	else {
		node->prev = list->last;
		list->last->next = node;
		list->last = node;
	}

	node->data = data;
	list->count++;
}

void* ds_linkedlist_popback(LinkedList* list) {
	LinkedListNode* node = list->last;

	void* data_ptr = node->data;
	ds_linkedlist_node_free(list, node);
	list->count--;
	return data_ptr;
}

void ds_linkedlist_remove(LinkedList* list, void* data) {
	ASSERT(list->count > 0);

	LinkedListNode* node = list->first;
	while (node != 0) {
		if (node->data == data) {
			if (node->prev) {
				node->prev->next = node->next;
			}
			if (node->next) {
				node->next->prev = node->prev;
			}
			ds_linkedlist_node_free(list, node);
			list->count--;
			return;
		}

		node = node->next;
	}
}

// Internal functions
LinkedListNode* ds_linkedlist_node_alloc(MemoryArena* arena, LinkedList* list) {
	LinkedListNode* result = 0;
	if (list->node_free_list != 0) {
		result = list->node_free_list;
		list->node_free_list = list->node_free_list->next;
	}
	else {
		result = memory_struct_zero_allocate(arena, LinkedListNode);
	}
	return result;
}

LinkedListNode* ds_linkedlist_node_free(LinkedList* list, LinkedListNode* node) {
	if (node == list->last && node->prev) {
		list->last = node->prev;
	}
	if (node == list->first && node->next) {
		list->first = node->next;
	}

	node->prev = 0;
	node->data = 0;
	node->next = list->node_free_list;
	list->node_free_list = node;
}
