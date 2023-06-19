#include "game.h"

#define PLAYER_START_HEALTH 3

GameState* game_Init(MemoryArena* arena, Assets* assets)
{
	GameState* gameState = memory_struct_zero_allocate(arena, GameState);
	gameState->arena_frame = memory_MemoryArenaCreate(memory_Megabytes(1));
	gameState->entity_active_count = 0;
	gameState->entity_free_count = 0;

	gameState->entity_activelist = ds_linkedlist_create(arena);
	gameState->entity_freelist = ds_linkedlist_create(arena);

	gameState->enemy_alive_count = 0;
	gameState->first_free_animation = 0;
	gameState->enemy_wave_level = 1;

	gameState->game_state_mode = GameStateMode_MainMenu;

	Entity* background = _game_entity_textured_create(arena, gameState, math_vec2f(0.0f, 0.0f), math_vec2f(320.0f, 180.0f), assets->texture_background);
	
	// #init player
	Entity* player = _game_entity_textured_create(arena, gameState, math_vec2f(0.0f, 0.0f), math_vec2f(32.0f, 32.0f), assets->texture_player);
	player->entityTags = EntityTag_Player;
	player->motion.acceleration = math_vec2f(900.0f, 5.0f);
	player->motion.friction = 0.95f;
	player->motion.velocity = math_vec2f(0.0f, 0.0f); 
	player->motion.direction = math_vec2f(0.0f, 0.0f);
	player->entityFlags |= EntityFlag_HasMotion | EntityFlag_HasCollider | EntityFlag_HasTint;
	player->collision_layers = CollisionLayer_Player;
	player->collision_box.bottom_left = math_vec2f(4.0f, 4.0f);
	player->collision_box.top_right = math_vec2f(28.0f, 28.0f);
	player->health = PLAYER_START_HEALTH;

	player->tint_color = math_vec3f(0.8f, 0.1f, 0.0f);
	player->tint_duration = 0.3f;
	player->tint_active = FALSE;

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

void game_Input(GameState* game_state, MemoryArena* arena, Assets* assets, f32 delta)
{
	if (game_state->game_state_mode != GameStateMode_Playing) {
		return;
	}

	Entity* player = game_state->player;
	float dx = 0.0f;
	if (input_IsKeyPressed(GLFW_KEY_A) || input_IsKeyPressed(GLFW_KEY_LEFT))
	{
		dx -= 1.0f;
	}
	if (input_IsKeyPressed(GLFW_KEY_D) || input_IsKeyPressed(GLFW_KEY_RIGHT))
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
	ds_linkedlist_loop(game_state->entity_activelist, Entity, e, node)
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
			ds_linkedlist_loop(game_state->entity_activelist, Entity, entity_other, node2)
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
			b8 will_shoot = FALSE;
			// shoot
			if (time_now_seconds() >= e->shoot_last_fire + e->shoot_cooldown_min) {
				will_shoot = e->shoot_change_to_shoot >= math_rand();
				e->shoot_last_fire = time_now_seconds();
			}

			switch (e->enemy_type)
			{
			case EnemyType_Basic:
				if (will_shoot) {
					Entity* bullet = _game_entity_player_bullet_create(game_state, arena, assets, e->transform.position);
					bullet->transform.position.x += e->transform.scale.x / 2.0f;
					bullet->motion.direction.y *= -1;
					bullet->collision_layers = CollisionLayer_Player;
					bullet->transform.rotation = 180.0f;
					sound_play(&assets->sound_laser_shot);
				}
				// movement
				{
					e->transform.position.x += sinf(game_state->secondsSinceStart * e->sin_frequency) * e->sin_magnitude;
				}
				break;

			case EnemyType_Hopper:
				// pick new target if at target
				if (time_now_seconds() >= e->move_last_switch + e->move_switch_cooldown) {
					f32 min_move_distance = 50.0f;
					f32 max_move_distance = 100.0f;
					f32 min_y_value = 80.0f;

					Vec2f new_target = math_vec2f_random_withinrange(e->transform.position, min_move_distance, max_move_distance);

					new_target.x = math_clamp(0.0f, new_target.x, graphics_window_render_width() - e->transform.scale.x);
					new_target.y = math_clamp(min_y_value, new_target.y, graphics_window_render_height() - e->transform.scale.y);

					// Avoid hopper stuck in corners
					if (new_target.x >= graphics_window_render_width() - e->transform.scale.x) {
						new_target.x -= math_rand_range(min_move_distance, max_move_distance);
					}					
					else if (new_target.x <= 0.0f) {
						new_target.x += math_rand_range(min_move_distance, max_move_distance);
					}

					if (new_target.y >= graphics_window_render_height() - e->transform.scale.y) {
						new_target.y -= math_rand_range(min_move_distance, max_move_distance);
					}
					else if (new_target.y <= min_y_value) {
						new_target.y += math_rand_range(min_move_distance, max_move_distance);
					}

					e->target_position = new_target;
					e->move_last_switch = time_now_seconds();
				}

				e->transform.position.x = math_lerp(e->transform.position.x, e->target_position.x, 0.1f);
				e->transform.position.y = math_lerp(e->transform.position.y, e->target_position.y, 0.1f);
				
				if (will_shoot) {
					f32 bullet_x = e->transform.position.x;
					for (u32 i = 0; i < 2; i++) {
						Entity* bullet = _game_entity_player_bullet_create(game_state, arena, assets, e->transform.position);
						bullet->transform.position.x = bullet_x;
						bullet->motion.direction.y *= -1;
						bullet->collision_layers = CollisionLayer_Player;
						bullet->transform.rotation = 180.0f;
						bullet_x += e->transform.scale.x;
					}
					sound_play(&assets->sound_laser_shot);
				}
				break;
			}
		}

		// #tint
		if (e->entityFlags & EntityFlag_HasTint) {
			f32 tint_time_progression = 0.0f;

			// Set tint strength if within duration
			if (e->tint_active && tint_time_progression <= e->tint_duration) {
				tint_time_progression = time_now_seconds() - e->tint_start;
				f32 tint_progress = 0.0f;
				if (e->tint_duration > 0.0f) {
					tint_progress = math_minf(1.0f, tint_time_progression / e->tint_duration);
				}

				f32 tint_strength = 0.0f;
				if (tint_progress <= 0.5f) {
					tint_strength = math_ease_out_cubic(tint_progress * 2.0f);
				}
				else {
					tint_strength = math_ease_out_cubic((1.0f - tint_progress) * 2.0f);
				}

				e->tint_strength = tint_strength;
			}
			else {
				e->tint_active = FALSE;
			}
		}
	}

	// clamp player position
	Entity* player = game_state->player;
	player->transform.position.x = math_maxf(0.0f, player->transform.position.x);
	player->transform.position.x = math_minf(320.0f - player->transform.scale.x, player->transform.position.x);

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
	
	ds_linkedlist_loop(game_state->entity_activelist, Entity, entity, listnode) {
		if (entity->entityFlags & EntityFlag_MarkedForDestruction) {
			_game_entity_free(arena, game_state, entity);
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

	// #levels
	{
		if (game_state->enemy_alive_count == 0 && 
			time_now_seconds() - game_state->enemy_wave_cleared_time >= game_state->enemy_wave_respawn_cooldown) {
			switch (game_state->enemy_wave_level) {
			case 1:
				_game_enemy_wave_create(game_state, arena, assets, 3, 1, EnemyType_Basic);
				break;
			case 2:
				_game_enemy_wave_create(game_state, arena, assets, 3, 2, EnemyType_Basic);
				break;
			case 3:
				_game_enemy_wave_create(game_state, arena, assets, 4, 2, EnemyType_Basic);
				break;
			case 4:
				_game_enemy_wave_create(game_state, arena, assets, 4, 3, EnemyType_Basic);
				break;
			case 5:
				_game_enemy_wave_create(game_state, arena, assets, 1, 2, EnemyType_Hopper);
				break;
			case 6:
				_game_enemy_wave_create(game_state, arena, assets, 2, 2, EnemyType_Hopper);
				break;
			case 7:
				_game_enemy_wave_create(game_state, arena, assets, 4, 2, EnemyType_Basic);
				_game_enemy_wave_create(game_state, arena, assets, 1, 2, EnemyType_Hopper);
				break;
			default:
				_game_enemy_wave_create(game_state, arena, assets, 4, game_state->enemy_wave_level, EnemyType_Basic);
				_game_enemy_wave_create(game_state, arena, assets, 2, game_state->enemy_wave_level / 2, EnemyType_Hopper);
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

	ds_linkedlist_loop(game_state->entity_activelist, Entity, entity, node)
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

Entity* _game_entity_textured_create(MemoryArena* arena, GameState* gameState, Vec2f position, Vec2f scale, Texture texture)
{
	Entity* entity = _game_entity_create(arena, gameState, position, scale);
	entity->texture = texture;
	entity->entityFlags = EntityFlag_HasTexture;

	return entity;
}

Entity* _game_entity_create(MemoryArena* arena, GameState* gameState, Vec2f position, Vec2f scale)
{
	Entity* entity = entity_allocate(gameState, arena);
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
	Entity* explosion = entity_allocate(gameState, arena);
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

void _game_entity_free(MemoryArena* arena, GameState* game_state, Entity* entity) {
	ds_linkedlist_remove(game_state->entity_activelist, entity);
	ds_linkedlist_pushback(arena, game_state->entity_freelist, entity);
}

// #collision
void _game_collision_handle(GameState* gameState, MemoryArena* arena, Assets* assets, Entity* either, Entity* other) {
	b8 on_same_collision_layer = either->collision_layers & other->collision_layers;
	if (!on_same_collision_layer) return;

	if (either->entityTags & EntityTag_Bullet && other->entityTags & EntityTag_Enemy) {
		other->health--;
		if (other->health == 0) {
			_game_entity_explosion_create(gameState, arena, assets, other->transform.position);
			other->entityFlags |= EntityFlag_MarkedForDestruction;
			gameState->enemy_alive_count--;
			gameState->player_score += 100;

			if (gameState->enemy_alive_count == 0)
			{
				gameState->enemy_wave_cleared_time = time_now_seconds();
				gameState->enemy_wave_level++;
			}
		}
		else {
			other->tint_active = TRUE;
			other->tint_start = time_now_seconds();
		}

		either->entityFlags |= EntityFlag_MarkedForDestruction;
	}

	if (either->entityTags & EntityTag_Bullet && other->entityTags & EntityTag_Player) {
		Entity* player = other;
		if (gameState->playerShieldState.isActive) {
			gameState->playerShieldState.isActive = FALSE;
			gameState->player_shield->isVisible = FALSE;
		}
		else {
			player->health--;
			player->tint_active = TRUE;
			player->tint_start = time_now_seconds();
			if (player->health == 0) {
				gameState->game_state_mode = GameStateMode_GameOver;
				player->health = PLAYER_START_HEALTH;
				player->motion.velocity.x = 0.0f;

				// clear everything except background and player
				ds_linkedlist_loop(gameState->entity_activelist, Entity, entity, node) {
					if (entity != gameState->player && entity != gameState->player_shield && entity != gameState->entity_background) {
						entity->entityFlags |= EntityFlag_MarkedForDestruction;
					}
				}
			}
		}

		_game_entity_explosion_create(gameState, arena, assets, either->transform.position);
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

// #waves
void _game_enemy_wave_create(GameState* game_state, MemoryArena* arena, Assets* assets, u32 enemies_per_row, u32 row_count, EnemyType enemy_type)
{
	f32 enemyWidth = 32;
	f32 spacing = graphics_window_render_width() / (float)enemies_per_row;
	f32 y_position = 140.0f;
	f32 y_spawn_offset = 0.0f;// graphics_window_render_height();
	f32 row_spacing = 42.0f;
	game_state->enemy_spawn_y_current_offset = y_spawn_offset;

	for (u32 i = 0; i < row_count; i++) {
		for (u32 j = 0; j < enemies_per_row; j++) {
			Entity* enemy = 0;
			switch (enemy_type) {
			case EnemyType_Basic:
				enemy = entity_enemy_basic_create(game_state, arena, assets);
				break;
			case EnemyType_Hopper:
				enemy = entity_enemy_hopper_create(game_state, arena, assets);
				break;
			}

			float xPosition = spacing * (float)(j + 1) - enemyWidth / 2.0f - spacing / 2.0f;
			enemy->transform.position.x = xPosition;
			enemy->transform.position.y = y_position;

			// for hopper
			enemy->target_position = math_vec2f_random_withinrange(enemy->transform.position, 50.0f, 100.0f);
			enemy->target_position.x = math_clamp(0.0f, enemy->target_position.x, graphics_window_render_width() - enemy->transform.scale.x);
			enemy->target_position.y = math_clamp(80.0f, enemy->target_position.y, graphics_window_render_height() - enemy->transform.scale.y);
		}
		y_position -= row_spacing;
	}
	game_state->enemy_alive_count = enemies_per_row * row_count;
}

Entity* entity_allocate(GameState* game_state, MemoryArena* arena) {
	Entity* result = 0;
	if (game_state->entity_freelist->count > 0) {
		result = ds_linkedlist_popback(game_state->entity_freelist);
	}
	else {
		result = memory_struct_zero_allocate(arena, Entity);
	}
	memset(result, 0, sizeof(Entity));
	ds_linkedlist_pushback(arena, game_state->entity_activelist, result);
	return result;
}

// Entity #blueprints
Entity* entity_enemy_basic_create(GameState* game_state, MemoryArena* arena, Assets* assets) {

	Entity* enemy = entity_allocate(game_state, arena);

	enemy->transform.scale = math_vec2f(32.0f, 32.0f);

	enemy->texture = assets->texture_enemy;

	enemy->isVisible = TRUE;
	enemy->entityFlags = EntityFlag_HasCollider | EntityFlag_HasTexture | EntityFlag_HasTint;
	enemy->entityTags = EntityTag_Enemy;
	enemy->enemy_type = EnemyType_Basic;
	enemy->collision_layers = CollisionLayer_Enemy;
	enemy->health = 2;

	enemy->tint_color = math_vec3f(0.8f, 0.1f, 0.0f);
	enemy->tint_duration = 0.3f;
	enemy->tint_active = FALSE;

	enemy->sin_frequency = 2.0f;
	enemy->sin_magnitude = 1.0f;
	
	enemy->motion.acceleration = math_vec2f(30.0f, 1.0f);
	enemy->motion.velocity = math_vec2f(0.0f, 0.0f);
	enemy->motion.direction = math_vec2f(-1.0f, 0.0f);
	enemy->motion.friction = 1.0f;
	enemy->collision_box.bottom_left = math_vec2f(0.0f, 0.0f);
	enemy->collision_box.top_right = math_vec2f(32.0f, 32.0f);

	enemy->shoot_change_to_shoot = 0.2f;
	enemy->shoot_cooldown_min = 2.0f;
	enemy->shoot_last_fire = 0.0f;

	return enemy;
}

Entity* entity_enemy_hopper_create(GameState* game_state, MemoryArena* arena, Assets* assets) {
	Entity* enemy = entity_allocate(game_state, arena);

	enemy->texture = assets->texture_enemy_hopper;
	enemy->transform.scale = math_vec2f(32.0f, 32.0f);

	enemy->isVisible = TRUE;
	enemy->entityFlags |= EntityFlag_HasCollider | EntityFlag_HasTexture | EntityFlag_HasTint;
	enemy->entityTags = EntityTag_Enemy;
	enemy->enemy_type = EnemyType_Hopper;
	enemy->collision_layers = CollisionLayer_Enemy;
	enemy->health = 2;

	enemy->tint_color = math_vec3f(0.8f, 0.1f, 0.0f);
	enemy->tint_duration = 0.15f;
	enemy->tint_active = FALSE;

	enemy->move_switch_cooldown = 5.0f;
	enemy->move_last_switch = time_now_seconds();

	enemy->motion.acceleration = math_vec2f(30.0f, 30.0f);
	enemy->motion.velocity = math_vec2f(0.0f, 0.0f);
	enemy->motion.friction = 1.0f;
	enemy->collision_box.bottom_left = math_vec2f(0.0f, 0.0f);
	enemy->collision_box.top_right = math_vec2f(32.0f, 32.0f);

	enemy->shoot_change_to_shoot = 0.3f;
	enemy->shoot_cooldown_min = 2.0f;
	enemy->shoot_last_fire = 0.0f;

	return enemy;
}
