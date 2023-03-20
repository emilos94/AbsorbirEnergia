#ifndef MEMORY_ARENA_H
#define MEMORY_ARENA_H

#include "definitions.h"
#include <stdlib.h>
#include <string.h>

#define memory_Kilobytes(x) x * 1024
#define memory_Megabytes(x) memory_Kilobytes(x) * 1024

struct MemoryArena
{
	char* memory;
	u32 capacity;
	u32 offset;
};
typedef struct MemoryArena MemoryArena;

MemoryArena* memory_MemoryArenaCreate(u32 capacity);
void memory_MemoryArenaReset(MemoryArena* arena);
void memory_MemoryArenaResetCount(MemoryArena* arena, u32 amount);
void memory_MemoryArenaFree(MemoryArena* arena);
void* memory_Allocate(MemoryArena* arena, u32 amount);
void* memory_allocate_zero(MemoryArena* arena, u32 amount);
void* memory_MemoryArenaCopyBuffer(MemoryArena* arena, void* buffer, u32 size);

#define memory_struct_zero_allocate(arena, type) (type*)memory_allocate_zero(arena, sizeof(type));
#define memory_AllocateArray(arena, type, count) (type*)memory_Allocate(arena, sizeof(type) * count);
#define memory_ArenaScope(arena) for(;;memory_MemoryArenaReset(arena))

#endif // !MEMORY_ARENA_H

