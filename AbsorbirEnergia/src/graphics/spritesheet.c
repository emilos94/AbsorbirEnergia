#include "spritesheet.h"

SpriteSheet graphics_SpriteSheetCreate(MemoryArena* arena, char* path, U32 spriteCount)
{
	SpriteSheet spriteSheet;
	spriteSheet.texture = graphics_TextureLoad(arena, path);
	spriteSheet.spriteCount = spriteCount;

	return spriteSheet;
}


Animation graphics_AnimationCreate(U32 spriteIndexMin, U32 spriteIndexMax, U32 repeat, float durationSeconds)
{
	Animation animation;
	animation.spriteIndex = 0;
	animation.spriteIndexMin = spriteIndexMin;
	animation.spriteIndexMax = spriteIndexMax;
	animation.durationSeconds = durationSeconds;
	animation.currentTime = 0.0f;
	animation.repeat = repeat;

	return animation;
}