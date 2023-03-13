#ifndef SPRITE_SHEET_H
#define SPRITE_SHEET_H

#include "../core/definitions.h"
#include "texture.h"

struct SpriteSheet
{
	Texture texture;
	U32 spriteCount;
};
typedef struct SpriteSheet SpriteSheet;

struct Animation
{
	// To facilitate free list
	struct Animation* next;

	U32 spriteIndex, spriteIndexMin, spriteIndexMax;
	B32 repeat, running;
	float durationSeconds;
	float currentTime;
};
typedef struct Animation Animation;

SpriteSheet graphics_spritesheet_create(Texture texture, U32 spriteCount);
void graphics_animation_set(Animation* animation, U32 spriteIndexMin, U32 spriteIndexMax, U32 repeat, float durationSeconds);

#endif