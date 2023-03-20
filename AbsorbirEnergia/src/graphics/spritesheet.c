#include "spritesheet.h"

SpriteSheet graphics_spritesheet_create(Texture texture, u32 spriteCount)
{
	SpriteSheet spriteSheet;
	spriteSheet.texture = texture;
	spriteSheet.spriteCount = spriteCount;

	return spriteSheet;
}

void graphics_animation_set(Animation* animation, u32 spriteIndexMin, u32 spriteIndexMax, u32 repeat, float durationSeconds)
{
	animation->spriteIndex = 0;
	animation->spriteIndexMin = spriteIndexMin;
	animation->spriteIndexMax = spriteIndexMax;
	animation->durationSeconds = durationSeconds;
	animation->currentTime = 0.0f;
	animation->repeat = repeat;
}
