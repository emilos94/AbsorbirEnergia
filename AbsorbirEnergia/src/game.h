#ifndef GAME_H
#define GAME_H

#include "core/assets.h"
#include "core/entity.h"
#include "core/input.h"
#include "core/time.h"
#include "graphics/texture.h"
#include "graphics/shader_program.h"
#include "graphics/renderer.h"
#include "math/math_util.h"
#include "ui/ui.h"

// rand
#include <stdlib.h>
#include <time.h>

#define ENTITY_MAX 100
#define DEBUG_RENDER_COLLISION_BOXES 0

struct PlayerShieldState
{
	float lastUsed;
	float lastExpired;
	float cooldown;
	float duration;
	b32 isActive;
};
typedef struct PlayerShieldState PlayerShieldState;

struct PlayerShootState
{
	float lastUsed;
	float cooldown;
};
typedef struct PlayerShootState PlayerShootState;

enum GameStateMode {
	GameStateMode_MainMenu,
	GameStateMode_Playing,
	GameStateMode_GameOver
};

struct GameState
{
	u32 entity_active_count, entity_free_count;

	MemoryArena* arena_frame;

	Entity* entity_first_free;
	Entity* entity_first_active;
	Entity* entity_last_active;

	Animation* first_free_animation;

	u32 enemy_alive_count;
	f32 enemy_wave_cleared_time;
	f32 enemy_wave_respawn_cooldown;
	f32 enemy_spawn_y_current_offset;
	u32 enemy_wave_level;

	f32 secondsSinceStart;

	Entity* player;
	Entity* player_shield;
	u32 player_score;
	Entity* entity_background;

	PlayerShieldState playerShieldState;
	PlayerShootState playerShootState;

	u32 game_state_mode;
};
typedef struct GameState GameState;

GameState* game_Init(MemoryArena* arena, Assets* assets);

void game_Input(GameState* gameState, MemoryArena* arena, Assets* assets);
void game_Update(GameState* gameState, MemoryArena* arena, Assets* assets, float delta);
void game_render(GameState* game_state, ShaderProgram shader_default, ShaderProgram shader_quad_colored);

Entity* _game_entity_create(MemoryArena* arena, GameState* gameState, Vec2f position, Vec2f scale);
Entity* _game_entity_textured_create(MemoryArena* arena, GameState* gameState, Vec2f position, Vec2f scale, Texture texture);
Entity* _game_entity_create(MemoryArena* arena, GameState* gameState, Vec2f position, Vec2f scale);
Entity* _game_entity_player_bullet_create(GameState* gameState, MemoryArena* arena, Assets* assets, Vec2f position);
Entity* _game_entity_explosion_create(GameState* gameState, MemoryArena* arena, Assets* assets, Vec2f position);
void _game_entity_free(GameState* gameState, Entity* entity);

void _game_enemy_wave_create(GameState* game_state, MemoryArena* arena, Assets* assets, u32 enemies_per_row, u32 row_count);

void _game_playerShieldUpdate(GameState* gameState);
void _game_animation_update(Entity* entity, float delta);

void _game_graphics_animation_free(GameState* game_state, Animation* animation);
Animation* _game_graphics_animation_allocate(GameState* game_state, MemoryArena* arena);

void _game_collision_handle(GameState* gameState, MemoryArena* arena, Assets* assets, Entity* either, Entity* other);

#endif