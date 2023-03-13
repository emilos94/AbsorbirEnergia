#include "game.h"

GameState* game_Init(MemoryArena* arena, Assets* assets)
{
	GameState* gameState = memory_AllocateStruct(arena, GameState);
	gameState->entities = memory_AllocateArray(arena, Entity, ENTITY_MAX);
	gameState->entity_active_entities = memory_AllocateArray(arena, U32, ENTITY_MAX);
	gameState->entity_active_id_to_index = memory_AllocateArray(arena, U32, ENTITY_MAX);
	gameState->entity_active_count = 0;

	gameState->entityCount = 0;
	gameState->first_free_entity = 0;
	gameState->first_free_animation = 0;

	Entity* background = _game_entity_textured_create(gameState, arena, math_vec2f(0.0f, 0.0f), math_vec2f(320.0f, 180.0f), assets->texture_background);
	Entity* player = _game_entity_textured_create(gameState, arena, math_vec2f(0.0f, 0.0f), math_vec2f(32.0f, 32.0f), assets->texture_player);
	player->entityTags = EntityTag_Player;

	// motion
	player->motion.acceleration = math_vec2f(600.0f, 5.0f);
	player->motion.friction = 0.9f;
	player->motion.velocity = math_vec2f(0.0f, 0.0f);
	player->motion.direction = math_vec2f(0.0f, 0.0f);
	player->entityFlags |= EntityFlag_HasMotion | EntityFlag_HasCollider;

	player->collision_box.top_left = math_vec2f(2.0f, 28.0f);
	player->collision_box.bottom_right = math_vec2f(28.0f, 2.0f);
	gameState->playerHandle = player->entityId;

	// player shield
	Entity* shield = _game_entity_create(gameState, player->transform.position, player->transform.scale);
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

	shield->collision_box.top_left = math_vec2f(0.0f, 32.0f);
	shield->collision_box.bottom_right = math_vec2f(32.0f, 0.0f);
	gameState->playerShieldHandle = shield->entityId;

	PlayerShieldState* playerShieldState = &gameState->playerShieldState;
	playerShieldState->duration = 1.0f;
	playerShieldState->cooldown = 1.0f;
	playerShieldState->lastExpired = 0.0f;
	playerShieldState->lastUsed = 0.0f;

	PlayerShootState* playerShootState = &gameState->playerShootState;
	playerShootState->cooldown = 0.7f;
	playerShootState->lastUsed = 0.0f;

	// Initialize enemy wave
	U32 enemyCount = 4;
	float enemyWidth = 32;
	float screenWidth = 320;
	float spacing = screenWidth / (float)enemyCount;

	for (U32 i = 0; i < enemyCount; i++)
	{
		float xPosition = spacing * (float)(i + 1) - enemyWidth / 2.0f - spacing / 2.0f;
		Entity* enemy = _game_entity_textured_create(gameState, arena, math_vec2f(xPosition, 140.0f), math_vec2f(32.0f, 32.0f), assets->texture_enemy);
		enemy->isVisible = TRUE;
		enemy->entityFlags |= EntityFlag_EnemyBaseMover | EntityFlag_HasMotion | EntityFlag_HasCollider;
		enemy->entityTags = EntityTag_Enemy;
		
		enemy->move_switch_cooldown = 0.9f;
		enemy->move_last_switch = time_now_seconds();

		enemy->motion.acceleration = math_vec2f(30.0f, 1.0f);
		enemy->motion.velocity = math_vec2f(0.0f, 0.0f);
		enemy->motion.direction = math_vec2f(-1.0f, 0.0f);
		enemy->motion.friction = 1.0f;
		enemy->collision_box.top_left = math_vec2f(0.0f, 32.0f);
		enemy->collision_box.bottom_right = math_vec2f(32.0f, 0.0f);
	}

	return gameState;
}

void game_Input(GameState* gameState, MemoryArena* arena, Assets* assets)
{
	Entity* player = &gameState->entities[gameState->playerHandle];
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
		PlayerShieldState* shieldState = &gameState->playerShieldState;

		if (!shieldState->isActive)
		{
			B32 canUseShield = gameState->secondsSinceStart - shieldState->lastExpired >= shieldState->cooldown;
			if (canUseShield)
			{
				shieldState->lastUsed = time_now_seconds();
				shieldState->isActive = TRUE;
				Entity* shield = &gameState->entities[gameState->playerShieldHandle];
				shield->isVisible = TRUE;
				shield->currentAnimation = 0;
				shield->animations[shield->currentAnimation]->running = TRUE;
			}
		}
	}

	if (input_IsKeyJustPressed(GLFW_KEY_SPACE))
	{
		PlayerShootState* playerShootState = &gameState->playerShootState;
		B32 canUse = time_now_seconds() - playerShootState->lastUsed >= playerShootState->cooldown;
		if (canUse)
		{
			playerShootState->lastUsed = time_now_seconds();
			Vec2f playerMiddle = math_Vec2fAdd(player->transform.position, math_Vec2fDivScalar(player->transform.scale, 2.0f));
			float bulletHalfSize = 8.0f;
			Vec2f bulletPosition = math_Vec2fSubScalar(playerMiddle, bulletHalfSize);
			_game_entity_player_bullet_create(gameState, arena, assets, bulletPosition);
		}
	}
}

void game_Update(GameState* gameState, MemoryArena* arena, Assets* assets, float delta)
{
	for (U32 i = 0; i < gameState->entity_active_count; i++)
	{
		U32 entity_handle = gameState->entity_active_entities[i];
		Entity* e = &gameState->entities[entity_handle];

		if (e->entityFlags & EntityFlag_EnemyBaseMover)
		{
			B32 shouldSwitchDirection = time_now_seconds() - e->move_last_switch >= e->move_switch_cooldown;
			if (shouldSwitchDirection)
			{
				e->motion.direction.x *= -1.0f;
				math_vec2f_negate(&e->motion.velocity);
				math_vec2f_mul_scalar(&e->motion.velocity, 0.5f);
				e->move_last_switch = time_now_seconds();
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
			Animation* animation = &e->animations[e->currentAnimation];
			_game_animation_update(e, delta);
		}

		if (e->entityFlags & EntityFlag_HasDestroyTimer)
		{
			B32 shouldBeDestroyed = time_now_seconds() - e->createdTime >= e->timeToLive;
			if (shouldBeDestroyed)
			{
				e->entityFlags |= EntityFlag_MarkedForDestruction;
				_game_entity_free(gameState, e);
			}
		}

		if (e->entityFlags & EntityFlag_HasCollider)
		{
			// todo: replace brute force check if it ever becomes a problem.
			//		 perhaps a grid lookup could work
			for (U32 j = 0; j < gameState->entity_active_count; j++)
			{
				B32 is_self = i == j;
				if (is_self) continue;

				U32 entity_handle_other = gameState->entity_active_entities[j];
				Entity* entity_other = &gameState->entities[entity_handle_other];

				B32 collision = entity_other->entityFlags & EntityFlag_HasCollider &&
					collision_check_aabb_aabb(e->transform.position, e->collision_box, entity_other->transform.position, entity_other->collision_box);
			
				if (collision)
				{
					_game_collision_handle(gameState, arena, assets, e, entity_other);
				}
			}
		}
	}

	// clamp player position
	Entity* player = &gameState->entities[gameState->playerHandle];
	player->transform.position.x = math_MaxF(0.0f, player->transform.position.x);
	player->transform.position.x = math_MinF(320.0f - player->transform.scale.x, player->transform.position.x);

	_game_playerShieldUpdate(gameState);

	// Free entities marked for destruction
	for (U32 i = 0; i < gameState->entity_active_count; i++)
	{
		U32 entity_handle = gameState->entity_active_entities[i];
		Entity* e = &gameState->entities[entity_handle];
		
		if (e->entityFlags & EntityFlag_MarkedForDestruction)
		{
			_game_entity_free(gameState, e);
		}
	}
}

void _game_playerShieldUpdate(GameState* gameState)
{
	// set player shield position to player pos
	Entity* player = &gameState->entities[gameState->playerHandle];
	Entity* shield = &gameState->entities[gameState->playerShieldHandle];
	shield->transform.position = player->transform.position;

	if (gameState->playerShieldState.isActive)
	{
		PlayerShieldState* shieldState = &gameState->playerShieldState;
		B32 durationExpired = time_now_seconds() - shieldState->lastUsed >= shieldState->duration;
		if (durationExpired)
		{
			shieldState->isActive = FALSE;
			shieldState->lastExpired = time_now_seconds();
			shield->isVisible = FALSE;
		}
	}
}

void _game_animation_update(Entity* entity, float delta)
{
	Animation* animation = &entity->animations[entity->currentAnimation];
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
			// animation done - notify ?
			animation->currentTime = animation->durationSeconds;
			animation->running = FALSE;
		}
	}
	float timePerSprite = animation->durationSeconds / (float)entity->spriteSheet.spriteCount;
	animation->spriteIndex = (U32)floor(animation->currentTime / timePerSprite);
}


Entity* _game_entity_allocate(GameState* gameState)
{
	Entity* result = gameState->first_free_entity;
	if (result)
	{
		gameState->first_free_entity = gameState->first_free_entity->next;
	}
	else
	{
		result = &gameState->entities[gameState->entityCount];
		result->entityId = gameState->entityCount++;
	}

	gameState->entity_active_entities[gameState->entity_active_count] = result->entityId;
	gameState->entity_active_id_to_index[result->entityId] = gameState->entity_active_count;
	gameState->entity_active_count++;
	return result;
}

Entity* _game_entity_textured_create(GameState* gameState, MemoryArena* arena, Vec2f position, Vec2f scale, Texture texture)
{
	Entity* entity = _game_entity_create(gameState, position, scale);
	entity->texture = texture;
	entity->entityFlags = EntityFlag_HasTexture;

	return entity;
}

Entity* _game_entity_create(GameState* gameState, Vec2f position, Vec2f scale)
{
	Entity* entity = _game_entity_allocate(gameState);
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
	Entity* bullet = _game_entity_textured_create(gameState, arena, position, math_vec2f(16.0f, 16.0f), assets->texture_bullet);
	bullet->entityFlags |= EntityFlag_HasMotion | EntityFlag_HasDestroyTimer | EntityFlag_HasCollider;
	bullet->entityTags = EntityTag_Bullet;
	bullet->motion.direction = math_vec2f(0.0f, 1.0f);
	bullet->motion.acceleration = math_vec2f(0.0f, 1100.0f);
	bullet->motion.friction = 0.99f;

	bullet->createdTime = gameState->secondsSinceStart;
	bullet->timeToLive = 3.0f;

	bullet->collision_box.top_left = math_vec2f(0.0f, 16.0f);
	bullet->collision_box.bottom_right = math_vec2f(16.0f, 0.0f);

	return bullet;
}

Entity* _game_entity_explosion_create(GameState* gameState, MemoryArena* arena, Assets* assets, Vec2f position)
{
	Entity* explosion = _game_entity_allocate(gameState);
	explosion->transform.position = position;
	explosion->transform.scale = math_vec2f(32.0f, 32.0f);
	explosion->isVisible = TRUE;

	explosion->entityFlags |= EntityFlag_HasAnimations | EntityFlag_HasDestroyTimer;
	explosion->entityTags = 0;

	explosion->animationCount = 1;
	explosion->currentAnimation = 0;

	explosion->spriteSheet = graphics_spritesheet_create(assets->texture_explosion, 8);
	explosion->animations[0] = _game_graphics_animation_allocate(gameState, arena);
	graphics_animation_set(explosion->animations[0], 0, 7, FALSE, 1.0f);

	explosion->timeToLive = 1.0f;
	explosion->createdTime = time_now_seconds();

	return explosion;
}

void _game_entity_free(GameState* gameState, Entity* entity)
{
	entity->next = gameState->first_free_entity;
	gameState->first_free_entity = entity;
	if (entity->entityFlags & EntityFlag_HasAnimations)
	{
		for (U32 i = 0; i < entity->animationCount; i++)
		{
			_game_graphics_animation_free(gameState, entity->animations[i]);
		}
	}
	entity_set_zero(entity);

	U32 index_to_fill = gameState->entity_active_id_to_index[entity->entityId];
	U32 latest_entity_id = gameState->entity_active_entities[gameState->entity_active_count - 1];
	gameState->entity_active_entities[index_to_fill] = latest_entity_id;
	gameState->entity_active_id_to_index[latest_entity_id] = index_to_fill;
	gameState->entity_active_count--;
}

void _game_collision_handle(GameState* gameState, MemoryArena* arena, Assets* assets, Entity* either, Entity* other)
{
	if (either->entityTags & EntityTag_Bullet && other->entityTags & EntityTag_Enemy)
	{
		_game_entity_explosion_create(gameState, arena, assets, either->transform.position);

		either->entityFlags |= EntityFlag_MarkedForDestruction;
		other->entityFlags |= EntityFlag_MarkedForDestruction;
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
		result = memory_AllocateStruct(arena, Animation);
		result->next = 0;
	}
	return result;
}
