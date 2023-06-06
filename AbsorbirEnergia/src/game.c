#include "game.h"

#define PLAYER_START_HEALTH 3

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
	gameState->enemy_wave_level = 1;

	gameState->game_state_mode = GameStateMode_MainMenu;

	Entity* background = _game_entity_textured_create(arena, gameState, math_vec2f(0.0f, 0.0f), math_vec2f(320.0f, 180.0f), assets->texture_background);
	
	// #init player
	Entity* player = _game_entity_textured_create(arena, gameState, math_vec2f(0.0f, 0.0f), math_vec2f(32.0f, 32.0f), assets->texture_player);
	player->entityTags = EntityTag_Player;
	player->motion.acceleration = math_vec2f(600.0f, 5.0f);
	player->motion.friction = 0.9f;
	player->motion.velocity = math_vec2f(0.0f, 0.0f);
	player->motion.direction = math_vec2f(0.0f, 0.0f);
	player->entityFlags |= EntityFlag_HasMotion | EntityFlag_HasCollider;
	player->collision_layers = CollisionLayer_Player;
	player->collision_box.bottom_left = math_vec2f(4.0f, 4.0f);
	player->collision_box.top_right = math_vec2f(28.0f, 28.0f);
	player->health = PLAYER_START_HEALTH;

	gameState->player = player;
	gameState->entity_background = background;
	gameState->player_score = 0;

	// #init player shield
	Entity* shield = _game_entity_create(arena, gameState, player->transform.position, player->transform.scale);
	shield->entityFlags |= EntityFlag_HasAnimations | EntityFlag_HasCollider;
	shield->entityTags = EntityTag_Shield;
	shield->spriteSheet = graphics_spritesheet_create(assets->texture_electric_shield, 9);
	shield->collision_layers = CollisionLayer_Player;

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

	gameState->enemy_wave_cleared_time = time_now_seconds();
	gameState->enemy_wave_respawn_cooldown = 1.0f;
	gameState->game_state_mode = GameStateMode_MainMenu;

	time_t t;
	srand((s32)time(&t));

	return gameState;
}

void game_Input(GameState* game_state, MemoryArena* arena, Assets* assets)
{
	if (game_state->game_state_mode != GameStateMode_Playing) {
		return;
	}

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
			sound_play(&assets->sound_laser_shot);
		}
	}
}

void game_Update(GameState* game_state, MemoryArena* arena, Assets* assets, float delta)
{
	entity_loopback(game_state->entity_last_active, e)
	{
		if (game_state->game_state_mode == GameStateMode_MainMenu)
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
				if (is_self) {
					continue;
				}

				b32 collision = entity_other->entityFlags & EntityFlag_HasCollider &&
					collision_check_aabb_aabb(e->transform.position, e->collision_box, entity_other->transform.position, entity_other->collision_box);
			
				if (collision) {
					_game_collision_handle(game_state, arena, assets, e, entity_other);
				}
			}
		}

		// #logic enemy
		if (e->entityTags & EntityTag_Enemy) {
			// lerp onto screen
			f32 offset_difference = math_lerp(0.0f, game_state->enemy_spawn_y_current_offset, 0.2f) * delta;
			game_state->enemy_spawn_y_current_offset -= offset_difference;
			game_state->enemy_spawn_y_current_offset = math_MaxF(game_state->enemy_spawn_y_current_offset, 0.0f);

			e->transform.position.y -= offset_difference;

			// shoot
			if (time_now_seconds() >= e->shoot_last_fire + e->shoot_cooldown_min) {
				b8 will_shoot = e->shoot_change_to_shoot >= math_rand();
				e->shoot_last_fire = time_now_seconds();
				if (will_shoot) {
					e->shoot_last_fire = time_now_seconds();
					Entity* bullet = _game_entity_player_bullet_create(game_state, arena, assets, e->transform.position);
					bullet->transform.position.x += e->transform.scale.x / 2.0f;
					bullet->motion.direction.y *= -1;
					bullet->collision_layers = CollisionLayer_Player;
					bullet->transform.rotation = 180.0f;
					sound_play(&assets->sound_laser_shot);
				}
			}
		}
	}

	// clamp player position
	Entity* player = game_state->player;
	player->transform.position.x = math_MaxF(0.0f, player->transform.position.x);
	player->transform.position.x = math_MinF(320.0f - player->transform.scale.x, player->transform.position.x);

	_game_playerShieldUpdate(game_state);

	// in game ui
	if (game_state->game_state_mode == GameStateMode_Playing) {
		f32 score_label_x = 10.0f;
		Mystr* score = mystr_create(game_state->arena_frame, "score: ");
		UI_Text* text = ui_text_create(
			score,
			assets->font_candara,
			score_label_x,
			graphics_window_render_height() - 10.0f,
			0.1f,
			graphics_window_render_width(),
			FALSE
		);
		score_label_x += text->width;

		Mystr* score_number_str = mystr_u32_to_mystr(game_state->arena_frame, game_state->player_score);
		ui_text_create(
			score_number_str,
			assets->font_candara,
			score_label_x,
			graphics_window_render_height() - 10.0f,
			0.1f,
			graphics_window_render_width(),
			FALSE
		);

		f32 level_label_x = graphics_window_render_width() - 40.0f;
		Mystr* level_label = mystr_create(game_state->arena_frame, "level: ");
		UI_Text* level_label_text = ui_text_create(
			level_label,
			assets->font_candara,
			level_label_x,
			graphics_window_render_height() - 10.0f,
			0.1f,
			graphics_window_render_width(),
			FALSE
		);
		level_label_x += level_label_text->width;

		Mystr* level_number = mystr_u32_to_mystr(game_state->arena_frame, game_state->enemy_wave_level);
		ui_text_create(
			level_number,
			assets->font_candara,
			level_label_x,
			graphics_window_render_height() - 10.0f,
			0.1f,
			graphics_window_render_width(),
			FALSE
		);

		f32 health_block_width = 15.0f;
		f32 health_block_x = graphics_window_render_width() - health_block_width - 2.0f;
		for (u32 i = 0; i < game_state->player->health; i++) {
			ui_block(health_block_x, 5.0f, health_block_width, 6.0f, math_vec3f(0.2f, 1.0f, 0.2f));
			health_block_x -= health_block_width + 2.0f;
		}
	}


	entity_loopback(game_state->entity_last_active, entity)
	{
		if (entity->entityFlags & EntityFlag_MarkedForDestruction)
		{
			_game_entity_free(game_state, entity);
		}
	}

	// #ui main menu
	if (game_state->game_state_mode == GameStateMode_MainMenu)
	{
		Mystr* str = mystr_create(game_state->arena_frame, "Press space to begin!");
		UI_Text* text = ui_text_create(
			str,
			assets->font_candara,
			0.0f, 
			120.0f, 
			0.3f,
			graphics_window_render_width(), 
			TRUE
		);

		if (input_IsKeyPressed(GLFW_KEY_SPACE)) {
			game_state->game_state_mode = GameStateMode_Playing;
		}
	}

	// #ui game over
	if (game_state->game_state_mode == GameStateMode_GameOver) {
		f32 y_placement = 120.0f;

		Mystr* str = mystr_create(game_state->arena_frame, "Game over!");
		UI_Text* text_game_over = ui_text_create(
			str,
			assets->font_candara,
			0.0f,
			y_placement,
			0.3f,
			graphics_window_render_width(),
			TRUE
		);

		y_placement -= text_game_over->height;
		y_placement -= 2.0f;

		Mystr* str_score = mystr_create(game_state->arena_frame, "Final score: ");
		Mystr* score_number_str = mystr_u32_to_mystr(game_state->arena_frame, game_state->player_score);
		Mystr* score_str_concat = mystr_concat(game_state->arena_frame, str_score, score_number_str);
		UI_Text* final_score_text = ui_text_create(
			score_str_concat,
			assets->font_candara,
			0.0f,
			y_placement,
			0.1f,
			graphics_window_render_width(),
			TRUE
		);

		y_placement -= final_score_text->height;
		y_placement -= 2.0f;

		Mystr* str_try_again = mystr_create(game_state->arena_frame, "Press space to try again!");
		UI_Text* text = ui_text_create(
			str_try_again,
			assets->font_candara,
			0.0f,
			y_placement,
			0.1f,
			graphics_window_render_width(),
			TRUE
		);

		if (input_IsKeyJustPressed(GLFW_KEY_SPACE)) {
			game_state->game_state_mode = GameStateMode_Playing;
			game_state->enemy_wave_level = 1;
			game_state->player_score = 0;
			game_state->enemy_alive_count = 0;
		}
	}

	// Wave respawn
	{
		if (game_state->enemy_alive_count == 0 && 
			time_now_seconds() - game_state->enemy_wave_cleared_time >= game_state->enemy_wave_respawn_cooldown) {
			switch (game_state->enemy_wave_level) {
			case 1:
				_game_enemy_wave_create(game_state, arena, assets, 3, 1);
				break;
			case 2:
				_game_enemy_wave_create(game_state, arena, assets, 3, 2);
				break;
			case 3:
				_game_enemy_wave_create(game_state, arena, assets, 4, 2);
				break;
			default:
				_game_enemy_wave_create(game_state, arena, assets, 4, 3);
				break;
			}
		}
	}

	memory_MemoryArenaReset(game_state->arena_frame);
}


void game_render(GameState* game_state, ShaderProgram shader_default, ShaderProgram shader_quad_colored)
{
	if (game_state->game_state_mode == GameStateMode_MainMenu)
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
	bullet->collision_layers = CollisionLayer_Enemy;

	bullet->createdTime = time_now_seconds();
	bullet->timeToLive = 3.0f;

	bullet->collision_box.bottom_left = math_vec2f(0.0f, 0.0f);
	bullet->collision_box.top_right = math_vec2f(16.0f, 16.0f);

	return bullet;
}

// #init explosion
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

// #collision
void _game_collision_handle(GameState* gameState, MemoryArena* arena, Assets* assets, Entity* either, Entity* other) {
	b8 on_same_collision_layer = either->collision_layers & other->collision_layers;
	if (!on_same_collision_layer) return;

	if (either->entityTags & EntityTag_Bullet && other->entityTags & EntityTag_Enemy) {
		_game_entity_explosion_create(gameState, arena, assets, other->transform.position);

		either->entityFlags |= EntityFlag_MarkedForDestruction;
		other->entityFlags |= EntityFlag_MarkedForDestruction;
		gameState->enemy_alive_count--;
		gameState->player_score += 100;

		if (gameState->enemy_alive_count == 0)
		{
			gameState->enemy_wave_cleared_time = time_now_seconds();
			gameState->enemy_wave_level++;
		}
	}

	if (either->entityTags & EntityTag_Bullet && other->entityTags & EntityTag_Player) {
		Entity* player = other;
		player->health--;
		if (player->health == 0) {
			gameState->game_state_mode = GameStateMode_GameOver;
			player->health = PLAYER_START_HEALTH;
			player->motion.velocity.x = 0.0f;

			// clear everything except background and player
			entity_loop(gameState->entity_first_active->next, entity) {
				if (entity != gameState->player && entity != gameState->player_shield) {
					entity->entityFlags |= EntityFlag_MarkedForDestruction;
				}
			}
		}
		either->entityFlags |= EntityFlag_MarkedForDestruction;
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

// #init enemy wave
void _game_enemy_wave_create(GameState* game_state, MemoryArena* arena, Assets* assets, u32 enemies_per_row, u32 row_count)
{
	f32 enemyWidth = 32;
	f32 screenWidth = 320;
	f32 spacing = graphics_window_render_width() / (float)enemies_per_row;
	f32 y_position = 140.0f;
	f32 y_spawn_offset = 0.0f;// graphics_window_render_height();
	f32 row_spacing = 42.0f;
	game_state->enemy_spawn_y_current_offset = y_spawn_offset;

	for (u32 i = 0; i < row_count; i++) {
		for (u32 j = 0; j < enemies_per_row; j++) {
			float xPosition = spacing * (float)(j + 1) - enemyWidth / 2.0f - spacing / 2.0f;
			Entity* enemy = _game_entity_textured_create(arena, game_state, math_vec2f(xPosition, y_position + y_spawn_offset), math_vec2f(32.0f, 32.0f), assets->texture_enemy);
			enemy->isVisible = TRUE;
			enemy->entityFlags |= EntityFlag_EnemyBaseMover | EntityFlag_HasMotion | EntityFlag_HasCollider;
			enemy->entityTags = EntityTag_Enemy;
			enemy->collision_layers = CollisionLayer_Enemy;

			enemy->move_switch_cooldown = 0.9f;
			enemy->move_last_switch = time_now_seconds();
			enemy->target_position = math_vec2f(xPosition - 64.0f, y_position);

			enemy->motion.acceleration = math_vec2f(30.0f, 1.0f);
			enemy->motion.velocity = math_vec2f(0.0f, 0.0f);
			enemy->motion.direction = math_vec2f(-1.0f, 0.0f);
			enemy->motion.friction = 1.0f;
			enemy->collision_box.bottom_left = math_vec2f(0.0f, 0.0f);
			enemy->collision_box.top_right = math_vec2f(32.0f, 32.0f);

			enemy->shoot_change_to_shoot = 0.2f;
			enemy->shoot_cooldown_min = 1.0f;
			enemy->shoot_last_fire = 0.0f;
		}
		y_position -= row_spacing;
	}
	game_state->enemy_alive_count = enemies_per_row * row_count;
}
