#ifndef GAME_H
#define GAME_H

#include "core/assets.h"
#include "core/entity.h"
#include "core/input.h"
#include "core/time.h"
#include "graphics/texture.h"
#include "math/math_util.h"

#define ENTITY_MAX 100

struct PlayerShieldState
{
	float lastUsed;
	float lastExpired;
	float cooldown;
	float duration;
	B32 isActive;
};
typedef struct PlayerShieldState PlayerShieldState;

struct PlayerShootState
{
	float lastUsed;
	float cooldown;
};
typedef struct PlayerShootState PlayerShootState;

struct GameState
{
	Entity* entities;
	U32 entityCount;

	U32* entity_active_entities;
	U32* entity_active_id_to_index;
	U32 entity_active_count;

	Entity* first_free_entity;
	Animation* first_free_animation;

	float secondsSinceStart;

	U32 playerHandle, playerShieldHandle;
	PlayerShieldState playerShieldState;
	PlayerShootState playerShootState;
};
typedef struct GameState GameState;

GameState* game_Init(MemoryArena* arena, Assets* assets);

void game_Input(GameState* gameState, MemoryArena* arena, Assets* assets);
void game_Update(GameState* gameState, MemoryArena* arena, Assets* assets, float delta);

Entity* _game_entity_create(GameState* gameState, Vec2f position, Vec2f scale);
Entity* _game_entity_textured_create(GameState* gameState, MemoryArena* arena, Vec2f position, Vec2f scale, Texture texture);
Entity* _game_entity_create(GameState* gameState, Vec2f position, Vec2f scale);
Entity* _game_entity_player_bullet_create(GameState* gameState, MemoryArena* arena, Assets* assets, Vec2f position);
Entity* _game_entity_explosion_create(GameState* gameState, MemoryArena* arena, Assets* assets, Vec2f position);
void _game_entity_free(GameState* gameState, Entity* entity);

void _game_playerShieldUpdate(GameState* gameState);
void _game_animation_update(Entity* entity, float delta);

void _game_graphics_animation_free(GameState* game_state, Animation* animation);
Animation* _game_graphics_animation_allocate(GameState* game_state, MemoryArena* arena);

void _game_collision_handle(GameState* gameState, MemoryArena* arena, Assets* assets, Entity* either, Entity* other);

#endif