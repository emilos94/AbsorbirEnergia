#ifndef ENTITY_H
#define ENTITY_H

#include "definitions.h"
#include "../math/vec2f.h"
#include "../graphics/texture.h"
#include "../graphics/spritesheet.h"

struct Transform
{
	Vec2f position;
	Vec2f scale;
	float rotation;
};
typedef struct Transform Transform;

struct Motion
{
	Vec2f direction;
	Vec2f velocity;
	Vec2f acceleration;
	float friction;
};
typedef struct Motion Motion;

enum EntityFlags
{
	EntityFlag_HasTexture = 1 << 0,
	EntityFlag_HasMotion = 1 << 1,
	EntityFlag_EnemyBaseMover = 1 << 2,
	EntityFlag_HasAnimations = 1 << 3,
	EntityFlag_HasDestroyTimer = 1 << 4,
};

struct Entity
{
	struct Entity* next; // to enable free list

	U32 entityId;
	U32 entityFlags;

	Transform transform;
	Texture texture;
	Motion motion;

	SpriteSheet spriteSheet;
	Animation* animations;
	U32 animationCount;
	U32 currentAnimation;
	B32 isAnimationPlaying;

	B32 isVisible;

	float createdTime;
	float timeToLive;

	float move_last_switch;
	float move_switch_cooldown;
};
typedef struct Entity Entity;

#endif // !ENTITY_H
