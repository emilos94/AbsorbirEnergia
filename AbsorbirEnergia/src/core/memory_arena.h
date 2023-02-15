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
	U32 capacity;
	U32 offset;
};
typedef struct MemoryArena MemoryArena;

MemoryArena* memory_MemoryArenaCreate(U32 capacity);
void memory_MemoryArenaReset(MemoryArena* arena);
void memory_MemoryArenaResetCount(MemoryArena* arena, U32 amount);
void memory_MemoryArenaFree(MemoryArena* arena);
void* memory_Allocate(MemoryArena* arena, U32 amount);
void* memory_MemoryArenaCopyBuffer(MemoryArena* arena, void* buffer, U32 size);

#define memory_AllocateStruct(arena, type) (type*)memory_Allocate(arena, sizeof(type));
#define memory_AllocateArray(arena, type, count) (type*)memory_Allocate(arena, sizeof(type) * count);
#define memory_ArenaScope(arena) for(;;memory_MemoryArenaReset(arena))

#endif // !MEMORY_ARENA_H

