#ifndef ENTITY_H
#define ENTITY_H

#include "definitions.h"
#include "collision.h"
#include "../math/vec2f.h"
#include "../math/vec3f.h"
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
	EntityFlag_HasAnimations = 1 << 2,
	EntityFlag_HasDestroyTimer = 1 << 3,
	EntityFlag_HasCollider = 1 << 4,
	EntityFlag_MarkedForDestruction = 1 << 5,
	EntityFlag_HasTint = 1 << 6
};

enum EntityTags
{
	EntityTag_Player = 1 << 0,
	EntityTag_Enemy = 1 << 1,
	EntityTag_Bullet = 1 << 2,
	EntityTag_Shield = 1 << 3
};
typedef u32 EntityTags;

enum EnemyType {
	EnemyType_Basic,
	EnemyType_Hopper
};
typedef u32 EnemyType;

enum CollisionLayers {
	CollisionLayer_Player = 1 << 0,
	CollisionLayer_Enemy = 1 << 1
};
typedef u32 CollisionLayers;

struct Entity {
	u32 entityId;
	u32 entityFlags;
	u32 entityTags;

	Transform transform;
	Texture texture;
	Motion motion;

	// animation
	SpriteSheet spriteSheet;
	Animation* animations[3];
	u32 animationCount;
	u32 currentAnimation;
	b32 isAnimationPlaying;

	b32 isVisible;

	float createdTime;
	float timeToLive;

	// enemy properties
	Vec2f target_position;
	f32 move_last_switch;
	f32 move_switch_cooldown;
	f32 shoot_cooldown_min;
	f32 shoot_last_fire;
	f32 shoot_change_to_shoot;
	EnemyType enemy_type;
	b8 moved_yet;

	// basic enemy properties
	f32 sin_frequency;
	f32 sin_magnitude;

	// collision
	CollisionBox collision_box;
	CollisionLayers collision_layers;

	u32 health;

	// tinting
	Vec3f tint_color;
	f32 tint_strength;
	f32 tint_start;
	f32 tint_duration;
	b8 tint_active;
};
typedef struct Entity Entity;

#endif // !ENTITY_H
