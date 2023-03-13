#include "spritesheet.h"

SpriteSheet graphics_spritesheet_create(Texture texture, U32 spriteCount)
{
	SpriteSheet spriteSheet;
	spriteSheet.texture = texture;
	spriteSheet.spriteCount = spriteCount;

	return spriteSheet;
}

void graphics_animation_set(Animation* animation, U32 spriteIndexMin, U32 spriteIndexMax, U32 repeat, float durationSeconds)
{
	animation->spriteIndex = 0;
	animation->spriteIndexMin = spriteIndexMin;
	animation->spriteIndexMax = spriteIndexMax;
	animation->durationSeconds = durationSeconds;
	animation->currentTime = 0.0f;
	animation->repeat = repeat;
}
