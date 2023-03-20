#ifndef ENTITY_H
#define ENTITY_H

#include "definitions.h"
#include "collision.h"
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
	EntityFlag_HasCollider = 1 << 5,
	EntityFlag_MarkedForDestruction = 1 << 6,
};

enum EntityTags
{
	EntityTag_Player = 1 << 0,
	EntityTag_Enemy = 1 << 1,
	EntityTag_Bullet = 1 << 2,
	EntityTag_Shield = 1 << 3,
};

struct Entity
{
	struct Entity* next; // to enable free list
	struct Entity* prev; // to enable free list

	u32 entityId;
	u32 entityFlags;
	u32 entityTags;

	Transform transform;
	Texture texture;
	Motion motion;

	SpriteSheet spriteSheet;
	Animation* animations[3];
	u32 animationCount;
	u32 currentAnimation;
	b32 isAnimationPlaying;

	b32 isVisible;

	float createdTime;
	float timeToLive;

	Vec2f target_position;
	float move_last_switch;
	float move_switch_cooldown;

	CollisionBox collision_box;
};
typedef struct Entity Entity;

#define entity_loopback(e, name)\
	Entity* name = e;\
	for(name; name != 0; name = name->prev)

#define entity_loop(e, name)\
	Entity* name = e;\
	for(name; name != 0; name = name->next)


void entity_set_zero(Entity* entity);

#endif // !ENTITY_H
