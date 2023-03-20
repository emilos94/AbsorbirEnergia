#ifndef SPRITE_SHEET_H
#define SPRITE_SHEET_H

#include "../core/definitions.h"
#include "texture.h"

struct SpriteSheet
{
	Texture texture;
	u32 spriteCount;
};
typedef struct SpriteSheet SpriteSheet;

struct Animation
{
	// To facilitate free list
	struct Animation* next;

	u32 spriteIndex, spriteIndexMin, spriteIndexMax;
	b32 repeat, running;
	float durationSeconds;
	float currentTime;
};
typedef struct Animation Animation;

SpriteSheet graphics_spritesheet_create(Texture texture, u32 spriteCount);
void graphics_animation_set(Animation* animation, u32 spriteIndexMin, u32 spriteIndexMax, u32 repeat, float durationSeconds);

#endif