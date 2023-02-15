#include "memory_arena.h"

MemoryArena* memory_MemoryArenaCreate(U32 capacity)
{
	MemoryArena* arena = (MemoryArena*)calloc(1, sizeof(MemoryArena));
	ASSERT(arena);

	arena->capacity = capacity;
	arena->offset = 0;
	arena->memory = calloc(1, capacity * sizeof(char));
	ASSERT(arena->memory);

	return arena;
}

void memory_MemoryArenaReset(MemoryArena* arena)
{
	arena->offset = 0;
}

void memory_MemoryArenaResetCount(MemoryArena* arena, U32 amount)
{
	ASSERT(arena->offset > amount && arena->capacity > amount);

	arena->offset -= amount;
}

void memory_MemoryArenaFree(MemoryArena* arena)
{
	free(arena->memory);
	free(arena);
}

void* memory_Allocate(MemoryArena* arena, U32 amount)
{
	ASSERT(arena->capacity >= arena->offset + amount);

	void* ptr = arena->memory + arena->offset;
	arena->offset += amount;
	return ptr;
}

void* memory_MemoryArenaCopyBuffer(MemoryArena* arena, void* buffer, U32 size)
{
	ASSERT(arena->capacity >= arena->offset + size);

	void* ptr = arena->memory + arena->offset;
	memcpy(ptr, buffer, size);
	arena->offset += size;

	return ptr;
}
