#include "game.h"

GameState* game_Init(MemoryArena* arena, Assets* assets)
{
	GameState* gameState = memory_struct_zero_allocate(arena, GameState);
	gameState->arena_frame = memory_MemoryArenaCreate(memory_Megabytes(1));
	gameState->entity_active_count = 0;
	gameState->entity_free_count = 0;

	gameState->enemy_alive_count = 0;
	gameState->entity_first_free = 0;
	gameState->entity_last_active = 0;
	gameState->first_free_animation = 0;

	Entity* background = _game_entity_textured_create(arena, gameState, math_vec2f(0.0f, 0.0f), math_vec2f(320.0f, 180.0f), assets->texture_background);
	Entity* player = _game_entity_textured_create(arena, gameState, math_vec2f(0.0f, 0.0f), math_vec2f(32.0f, 32.0f), assets->texture_player);
	player->entityTags = EntityTag_Player;

	// motion
	player->motion.acceleration = math_vec2f(600.0f, 5.0f);
	player->motion.friction = 0.9f;
	player->motion.velocity = math_vec2f(0.0f, 0.0f);
	player->motion.direction = math_vec2f(0.0f, 0.0f);
	player->entityFlags |= EntityFlag_HasMotion | EntityFlag_HasCollider;

	player->collision_box.bottom_left = math_vec2f(4.0f, 4.0f);
	player->collision_box.top_right = math_vec2f(28.0f, 28.0f);
	gameState->player = player;
	gameState->entity_background = background;

	// player shield
	Entity* shield = _game_entity_create(arena, gameState, player->transform.position, player->transform.scale);
	shield->entityFlags |= EntityFlag_HasAnimations | EntityFlag_HasCollider;
	shield->entityTags = EntityTag_Shield;
	shield->spriteSheet = graphics_spritesheet_create(assets->texture_electric_shield, 9);

	shield->animations[0] = _game_graphics_animation_allocate(gameState, arena);
	graphics_animation_set(shield->animations[0], 0, 5, FALSE, 1.0f);

	shield->animations[1] = _game_graphics_animation_allocate(gameState, arena);
	graphics_animation_set(shield->animations[1], 6, 8, TRUE, 0.7f);

	shield->animationCount = 2;
	shield->currentAnimation = 0;
	shield->isVisible = FALSE;
	shield->isAnimationPlaying = FALSE;

	shield->collision_box.bottom_left = math_vec2f(0.0f, 0.0f);
	shield->collision_box.top_right = math_vec2f(32.0f, 32.0f);
	gameState->player_shield = shield;

	PlayerShieldState* playerShieldState = &gameState->playerShieldState;
	playerShieldState->duration = 1.0f;
	playerShieldState->cooldown = 1.0f;
	playerShieldState->lastExpired = 0.0f;
	playerShieldState->lastUsed = 0.0f;

	PlayerShootState* playerShootState = &gameState->playerShootState;
	playerShootState->cooldown = 0.7f;
	playerShootState->lastUsed = 0.0f;

	_game_enemy_wave_create(gameState, arena, assets, 4);

	gameState->main_menu_on = TRUE;

	return gameState;
}

void game_Input(GameState* game_state, MemoryArena* arena, Assets* assets)
{
	Entity* player = game_state->player;
	float dx = 0.0f;
	if (input_IsKeyPressed(GLFW_KEY_A))
	{
		dx -= 1.0f;
	}
	if (input_IsKeyPressed(GLFW_KEY_D))
	{
		dx += 1.0f;
	}
	player->motion.direction.x = dx;

	if (input_IsKeyJustPressed(GLFW_KEY_LEFT_CONTROL))
	{
		PlayerShieldState* shieldState = &game_state->playerShieldState;

		if (!shieldState->isActive)
		{
			b32 canUseShield = game_state->secondsSinceStart - shieldState->lastExpired >= shieldState->cooldown;
			if (canUseShield)
			{
				shieldState->lastUsed = time_now_seconds();
				shieldState->isActive = TRUE;
				Entity* shield = game_state->player_shield;
				shield->isVisible = TRUE;
				shield->currentAnimation = 0;
				shield->animations[shield->currentAnimation]->running = TRUE;
			}
		}
	}

	if (input_IsKeyJustPressed(GLFW_KEY_SPACE))
	{
		PlayerShootState* playerShootState = &game_state->playerShootState;
		b32 canUse = time_now_seconds() - playerShootState->lastUsed >= playerShootState->cooldown;
		if (canUse)
		{
			playerShootState->lastUsed = time_now_seconds();
			Vec2f playerMiddle = math_Vec2fAdd(player->transform.position, math_Vec2fDivScalar(player->transform.scale, 2.0f));
			float bulletHalfSize = 8.0f;
			Vec2f bulletPosition = math_Vec2fSubScalar(playerMiddle, bulletHalfSize);
			_game_entity_player_bullet_create(game_state, arena, assets, bulletPosition);
		}
	}
}

void game_Update(GameState* game_state, MemoryArena* arena, Assets* assets, float delta)
{
	entity_loopback(game_state->entity_last_active, e)
	{
		if (game_state->main_menu_on)
		{
			break;
		}

		if (e->entityFlags & EntityFlag_EnemyBaseMover)
		{
			f32 distance = math_vec2f_distance(e->transform.position, e->target_position);
			e->transform.position.x += distance * 0.1f * e->motion.direction.x * delta;
			
			f32 x_direction = e->motion.direction.x;
			if (x_direction > 0.0f && e->transform.position.x >= e->target_position.x)
			{
				e->motion.direction.x *= -1.0f;
				e->target_position.x -= 64.0f;
			}
			if (x_direction < 0.0f && e->transform.position.x <= e->target_position.x)
			{
				e->motion.direction.x *= -1.0f;
				e->target_position.x += 64.0f;
			}
		}

		if (e->entityFlags & EntityFlag_HasMotion)
		{
			// p' = vt + p
			Vec2f velocity = math_Vec2fMulScalar(e->motion.velocity, delta);
			e->transform.position = math_Vec2fAdd(e->transform.position, velocity);
			
			// v' = at * f + v 
			Vec2f acceleration = math_Vec2fMul(e->motion.direction, e->motion.acceleration);
			Vec2f deltaAcceleration = math_Vec2fMulScalar(acceleration, delta);
			Vec2f newVelocity = math_Vec2fAdd(e->motion.velocity, deltaAcceleration);
			newVelocity = math_Vec2fMulScalar(newVelocity, e->motion.friction);
			e->motion.velocity = newVelocity;
		}


		if (e->entityFlags & EntityFlag_HasAnimations)
		{
			_game_animation_update(e, delta);
		}

		if (e->entityFlags & EntityFlag_HasDestroyTimer)
		{
			b32 shouldBeDestroyed = time_now_seconds() - e->createdTime >= e->timeToLive;
			if (shouldBeDestroyed)
			{
				e->entityFlags |= EntityFlag_MarkedForDestruction;
			}
		}

		if (e->entityFlags & EntityFlag_HasCollider)
		{
			// todo: replace brute force check if it ever becomes a problem.
			//		 perhaps a grid lookup could work
			entity_loopback(game_state->entity_last_active, entity_other)
			{
				b32 is_self = entity_other == e;
				if (is_self)
				{
					entity_other = entity_other->prev;
					continue;
				}

				b32 collision = entity_other->entityFlags & EntityFlag_HasCollider &&
					collision_check_aabb_aabb(e->transform.position, e->collision_box, entity_other->transform.position, entity_other->collision_box);
			
				if (collision)
				{
					_game_collision_handle(game_state, arena, assets, e, entity_other);
				}
			}
		}
	}

	// clamp player position
	Entity* player = game_state->player;
	player->transform.position.x = math_MaxF(0.0f, player->transform.position.x);
	player->transform.position.x = math_MinF(320.0f - player->transform.scale.x, player->transform.position.x);

	_game_playerShieldUpdate(game_state);

	entity_loopback(game_state->entity_last_active, entity)
	{
		if (entity->entityFlags & EntityFlag_MarkedForDestruction)
		{
			_game_entity_free(game_state, entity);
		}
	}

	if (game_state->main_menu_on)
	{
		f32 button_width = 80.0f;
		f32 center_x = graphics_window_render_width() / 2.0f - button_width / 2.0f;
		UI_Info* button_play = ui_button(center_x, 120.0f, button_width, 32.0f);

		Mystr* str = mystr_create(game_state->arena_frame, "play", 4);
		UI_Text* text = ui_text_create(str, assets->font_candara, center_x + button_width * 0.25f, 120.0f + 10.0f, 0.2f, button_width);
		if (button_play->hot)
		{
			math_vec3f_set(1.0f, 0.0f, 0.0f, &button_play->widget->color);
		}
		else
		{
			math_vec3f_set(1.0f, 0.0f, 1.0f, &button_play->widget->color);
		}

		if (button_play->active)
		{
			game_state->main_menu_on = FALSE;
		}
	}

	memory_MemoryArenaReset(game_state->arena_frame);
}


void game_render(GameState* game_state, ShaderProgram shader_default, ShaderProgram shader_quad_colored)
{
	if (game_state->main_menu_on)
	{
		graphics_entity_render(shader_default, game_state->entity_background);
		return;
	}

	entity_loop(game_state->entity_first_active, entity)
	{
		if (!entity->isVisible)
		{
			continue;
		}

		graphics_entity_render(shader_default, entity);
		if (entity->entityFlags & EntityFlag_HasCollider && DEBUG_RENDER_COLLISION_BOXES)
		{
			Vec2f bottom_left = math_Vec2fAdd(entity->transform.position, entity->collision_box.bottom_left);
			Vec2f top_right = math_Vec2fAdd(entity->transform.position, entity->collision_box.top_right);
			graphics_render_quad_color(shader_quad_colored, bottom_left, top_right, 1.0f, 0.0f, 0.0f);
		}
	}
}

void _game_playerShieldUpdate(GameState* game_state)
{
	game_state->player_shield->transform.position = game_state->player->transform.position;

	if (game_state->playerShieldState.isActive)
	{
		PlayerShieldState* shieldState = &game_state->playerShieldState;
		b32 durationExpired = time_now_seconds() - shieldState->lastUsed >= shieldState->duration;
		if (durationExpired)
		{
			shieldState->isActive = FALSE;
			shieldState->lastExpired = time_now_seconds();
			game_state->player_shield->isVisible = FALSE;
		}
	}
}

void _game_animation_update(Entity* entity, float delta)
{
	Animation* animation = entity->animations[entity->currentAnimation];
	if (!animation->running) return;
	
	animation->currentTime += delta;
	if (animation->currentTime > animation->durationSeconds)
	{
		if (animation->repeat)
		{
			animation->currentTime = 0.0f;
		}
		else
		{
			animation->currentTime = 0.0f;
			animation->running = FALSE;
		}
	}
	float timePerSprite = animation->durationSeconds / (float)entity->spriteSheet.spriteCount;
	animation->spriteIndex = (u32)floor(animation->currentTime / timePerSprite);
}


Entity* _game_entity_allocate(MemoryArena* arena, GameState* game_state)
{
	Entity* result = game_state->entity_first_free;
	if (result)
	{
		game_state->entity_first_free = game_state->entity_first_free->next;
		game_state->entity_free_count--;
	}
	else
	{
		result = memory_struct_zero_allocate(arena, Entity);
	}

	if (!game_state->entity_first_active)
	{
		game_state->entity_first_active = result;
	}

	if (game_state->entity_last_active)
	{
		game_state->entity_last_active->next = result;
	}

	entity_set_zero(result);
	result->prev = game_state->entity_last_active;
	game_state->entity_last_active = result;
	game_state->entity_active_count++;
	return result;
}

Entity* _game_entity_textured_create(MemoryArena* arena, GameState* gameState, Vec2f position, Vec2f scale, Texture texture)
{
	Entity* entity = _game_entity_create(arena, gameState, position, scale);
	entity->texture = texture;
	entity->entityFlags = EntityFlag_HasTexture;

	return entity;
}

Entity* _game_entity_create(MemoryArena* arena, GameState* gameState, Vec2f position, Vec2f scale)
{
	Entity* entity = _game_entity_allocate(arena, gameState);
	entity->transform.position = position;
	entity->transform.scale = scale;
	entity->transform.rotation = 0.0f;
	entity->entityFlags = 0;
	entity->isVisible = TRUE;
	entity->isAnimationPlaying = FALSE;
	return entity;
}

Entity* _game_entity_player_bullet_create(GameState* gameState, MemoryArena* arena, Assets* assets, Vec2f position)
{
	Entity* bullet = _game_entity_textured_create(arena, gameState, position, math_vec2f(16.0f, 16.0f), assets->texture_bullet);
	bullet->entityFlags |= EntityFlag_HasMotion | EntityFlag_HasDestroyTimer | EntityFlag_HasCollider;
	bullet->entityTags = EntityTag_Bullet;
	bullet->motion.direction = math_vec2f(0.0f, 1.0f);
	bullet->motion.acceleration = math_vec2f(0.0f, 1100.0f);
	bullet->motion.friction = 0.99f;

	bullet->createdTime = time_now_seconds();
	bullet->timeToLive = 3.0f;

	bullet->collision_box.bottom_left = math_vec2f(0.0f, 0.0f);
	bullet->collision_box.top_right = math_vec2f(16.0f, 16.0f);

	return bullet;
}

Entity* _game_entity_explosion_create(GameState* gameState, MemoryArena* arena, Assets* assets, Vec2f position)
{
	Entity* explosion = _game_entity_allocate(arena, gameState);
	explosion->transform.position = position;
	explosion->transform.scale = math_vec2f(32.0f, 32.0f);
	explosion->isVisible = TRUE;

	explosion->entityFlags |= EntityFlag_HasAnimations | EntityFlag_HasDestroyTimer;
	explosion->entityTags = 0;

	explosion->animationCount = 1;
	explosion->currentAnimation = 0;

	explosion->spriteSheet = graphics_spritesheet_create(assets->texture_explosion, 8);
	explosion->animations[0] = _game_graphics_animation_allocate(gameState, arena);
	explosion->animations[0]->currentTime = 0.0f;
	explosion->animations[0]->running = TRUE;
	graphics_animation_set(explosion->animations[0], 0, 7, FALSE, 1.0f);

	explosion->timeToLive = 1.0f;
	explosion->createdTime = time_now_seconds();

	return explosion;
}

void _game_entity_free(GameState* game_state, Entity* entity)
{
	if (entity->prev)
	{
		entity->prev->next = entity->next;
	}

	if (entity->next)
	{
		entity->next->prev = entity->prev;
	}
	entity->next = game_state->entity_first_free;
	game_state->entity_first_free = entity;
	game_state->entity_free_count++;
	game_state->entity_active_count--;

	if (game_state->entity_last_active == entity)
	{
		game_state->entity_last_active = entity->prev;
	}

	if (game_state->entity_first_active == entity)
	{
		game_state->entity_first_active = entity->next;
	}
}

void _game_collision_handle(GameState* gameState, MemoryArena* arena, Assets* assets, Entity* either, Entity* other)
{
	if (either->entityTags & EntityTag_Bullet && other->entityTags & EntityTag_Enemy)
	{
		_game_entity_explosion_create(gameState, arena, assets, other->transform.position);

		either->entityFlags |= EntityFlag_MarkedForDestruction;
		other->entityFlags |= EntityFlag_MarkedForDestruction;
		gameState->enemy_alive_count--;

		if (gameState->enemy_alive_count == 0)
		{
			_game_enemy_wave_create(gameState, arena, assets, 4);
		}
	}
}

void _game_graphics_animation_free(GameState* game_state, Animation* animation)
{
	animation->next = game_state->first_free_animation;
	game_state->first_free_animation = animation;

	animation->repeat = FALSE;
	animation->currentTime = 0.0f;
	animation->durationSeconds = 0.0f;
	animation->running = FALSE;
	animation->spriteIndex = 0;
	animation->spriteIndexMax = 0;
	animation->spriteIndexMin = 0;
}

Animation* _game_graphics_animation_allocate(GameState* game_state, MemoryArena* arena)
{
	Animation* result = game_state->first_free_animation;
	if (result != 0)
	{
		game_state->first_free_animation = game_state->first_free_animation->next;
	}
	else
	{
		result = memory_struct_zero_allocate(arena, Animation);
		result->next = 0;
	}
	return result;
}

void _game_enemy_wave_create(GameState* game_state, MemoryArena* arena, Assets* assets, u32 count)
{
	float enemyWidth = 32;
	float screenWidth = 320;
	float spacing = screenWidth / (float)count;

	for (u32 i = 0; i < count; i++)
	{
		float xPosition = spacing * (float)(i + 1) - enemyWidth / 2.0f - spacing / 2.0f;
		Entity* enemy = _game_entity_textured_create(arena, game_state, math_vec2f(xPosition, 140.0f), math_vec2f(32.0f, 32.0f), assets->texture_enemy);
		enemy->isVisible = TRUE;
		enemy->entityFlags |= EntityFlag_EnemyBaseMover | EntityFlag_HasMotion | EntityFlag_HasCollider;
		enemy->entityTags = EntityTag_Enemy;

		enemy->move_switch_cooldown = 0.9f;
		enemy->move_last_switch = time_now_seconds();
		enemy->target_position = math_vec2f(xPosition - 64.0f, 140.0f);

		enemy->motion.acceleration = math_vec2f(30.0f, 1.0f);
		enemy->motion.velocity = math_vec2f(0.0f, 0.0f);
		enemy->motion.direction = math_vec2f(-1.0f, 0.0f);
		enemy->motion.friction = 1.0f;
		enemy->collision_box.bottom_left = math_vec2f(0.0f, 0.0f);
		enemy->collision_box.top_right = math_vec2f(32.0f, 32.0f);
	}
	game_state->enemy_alive_count = count;
}
