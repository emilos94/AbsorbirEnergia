#include "game.h"

GameState* game_Init(MemoryArena* arena)
{
	GameState* gameState = memory_AllocateStruct(arena, GameState);
	gameState->entities = memory_AllocateArray(arena, Entity, ENTITY_MAX);
	gameState->entityCount = 0;
	gameState->first_free_entity = 0;

	Entity* background = _game_entity_textured_create(gameState, arena, math_Vec2f(0.0f, 0.0f), math_Vec2f(320.0f, 180.0f), "res/images/background.png");
	Entity* player = _game_entity_textured_create(gameState, arena, math_Vec2f(0.0f, 0.0f), math_Vec2f(32.0f, 32.0f), "res/images/player.png");

	// motion
	player->motion.acceleration = math_Vec2f(600.0f, 5.0f);
	player->motion.friction = 0.9f;
	player->motion.velocity = math_Vec2f(0.0f, 0.0f);
	player->motion.direction = math_Vec2f(0.0f, 0.0f);
	player->entityFlags |= EntityFlag_HasMotion;
	gameState->playerHandle = player->entityId;

	// player shield
	Entity* shield = _game_entity_create(gameState, player->transform.position, player->transform.scale);
	shield->entityFlags |= EntityFlag_HasAnimations;
	shield->spriteSheet = graphics_SpriteSheetCreate(arena, "res/images/electric_shield.png", 9);
	shield->animations = memory_AllocateArray(arena, Animation, 2);
	shield->animations[0] = graphics_AnimationCreate(0, 5, FALSE, 1.0f);
	shield->animations[1] = graphics_AnimationCreate(6, 8, TRUE, 0.7f);
	shield->animationCount = 2;
	shield->currentAnimation = 0;
	shield->isVisible = FALSE;
	shield->isAnimationPlaying = FALSE;
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
	// todo: fix why is one of the enemies not visible?
	for (U32 i = 0; i < enemyCount; i++)
	{
		float xPosition = spacing * (float)(i + 1) - enemyWidth / 2.0f - spacing / 2.0f;
		Entity* enemy = _game_entity_textured_create(gameState, arena, math_Vec2f(xPosition, 140.0f), math_Vec2f(32.0f, 32.0f), "res/images/base_enemy.png");
		enemy->isVisible = TRUE;
		enemy->entityFlags |= EntityFlag_EnemyBaseMover | EntityFlag_HasMotion;
		
		enemy->move_switch_cooldown = 0.9f;
		enemy->move_last_switch = time_now_seconds();

		enemy->motion.acceleration = math_Vec2f(30.0f, 1.0f);
		enemy->motion.velocity = math_Vec2f(0.0f, 0.0f);
		enemy->motion.direction = math_Vec2f(-1.0f, 0.0f);
		enemy->motion.friction = 1.0f;
	}

	return gameState;
}

void game_Input(GameState* gameState, MemoryArena* arena)
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
				shield->animations[shield->currentAnimation].running = TRUE;
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
			_game_entity_player_bullet_create(gameState, arena, bulletPosition);
		}
	}
}

void game_Update(GameState* gameState, MemoryArena* arena, float delta)
{
	for (U32 i = 0; i < gameState->entityCount; i++)
	{
		Entity* e = &gameState->entities[i];

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
			B32 shouldBeDestroyed = gameState->secondsSinceStart - e->createdTime >= e->timeToLive;
			if (shouldBeDestroyed)
			{
				_game_entity_free(gameState, e);
			}
		}
	}

	// clamp player position
	Entity* player = &gameState->entities[gameState->playerHandle];
	player->transform.position.x = math_MaxF(0.0f, player->transform.position.x);
	player->transform.position.x = math_MinF(320.0f - player->transform.scale.x, player->transform.position.x);

	_game_playerShieldUpdate(gameState);
}

void game_Cleanup(GameState* gameState)
{
	for (U32 i = 0; i < gameState->entityCount; i++)
	{
		Entity e = gameState->entities[i];
		if (e.entityFlags & EntityFlag_HasTexture)
		{
			graphics_TextureDestroy(&e.texture);
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
			animation->currentTime = 0.0f;
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
	return result;
}

Entity* _game_entity_textured_create(GameState* gameState, MemoryArena* arena, Vec2f position, Vec2f scale, char* texturePath)
{
	Entity* entity = _game_entity_create(gameState, position, scale);
	entity->texture = graphics_TextureLoad(arena, texturePath);
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

Entity* _game_entity_player_bullet_create(GameState* gameState, MemoryArena* arena, Vec2f position)
{
	Entity* bullet = _game_entity_textured_create(gameState, arena, position, math_Vec2f(16.0f, 16.0f), "res/images/player_bullet.png");
	bullet->entityFlags |= EntityFlag_HasMotion | EntityFlag_HasDestroyTimer;
	bullet->motion.direction = math_Vec2f(0.0f, 1.0f);
	bullet->motion.acceleration = math_Vec2f(0.0f, 1100.0f);
	bullet->motion.friction = 0.99f;

	bullet->createdTime = gameState->secondsSinceStart;
	bullet->timeToLive = 3.0f;

	return bullet;
}
	
void _game_entity_free(GameState* gameState, Entity* entity)
{
	entity->next = gameState->first_free_entity;
	gameState->first_free_entity = entity;
	entity->entityFlags = 0;
	entity->isVisible = FALSE;
}
