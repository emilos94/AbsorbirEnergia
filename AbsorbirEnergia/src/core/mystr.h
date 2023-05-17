#ifndef MYSTR_H
#define MYSTR_H

#include "definitions.h"
#include "memory_arena.h"
#include <stdio.h>

struct Mystr
{
	char* text;
	u32 length;
};
typedef struct Mystr Mystr;

Mystr* mystr_create(MemoryArena* arena, char* text);
Mystr* mystr_u32_to_mystr(MemoryArena* arena, u32 number);
Mystr* mystr_concat(MemoryArena* arena, Mystr* left, Mystr* right);
b8 mystr_equals(Mystr* left, Mystr* right);
b8 mystr_char_array_equals(char* either, char* other, u32 length);
b8 mystr_u32_parse(char* char_arr, u32* destination);
b8 mystr_s32_parse(char* char_arr, s32* destination);
s32 mystr_char_array_find_indexof(char* arr_to_search, u32 arr_text_length, char* search_text, u32 search_text_length);
b8 mystr_char_is_number(char c);
#endif