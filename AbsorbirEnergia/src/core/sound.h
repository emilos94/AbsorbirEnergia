#include "definitions.h"
#include "memory_arena.h"
#include <AL/al.h>
#include <AL/alc.h>

#ifndef SOUND_H
#define SOUND_H

typedef struct SoundSource {
	ALuint source;
	ALuint buffer;
} SoundSource;

void sound_initialize(void);
SoundSource sound_source_load(MemoryArena* arena, char* path);
void sound_volume_set(f32 volume);
void sound_play(SoundSource* source);

#endif // !SOUND_HW
