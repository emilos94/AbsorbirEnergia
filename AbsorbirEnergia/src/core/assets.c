#include "assets.h"


Assets* assets_load(MemoryArena* arena)
{
	Assets* assets = memory_struct_zero_allocate(arena, Assets);

	u32 initial_offset = arena->offset;
	assets->texture_player = graphics_TextureLoad(arena, "res/images/player.png");
	assets->texture_background = graphics_TextureLoad(arena, "res/images/background.png");
	assets->texture_electric_shield = graphics_TextureLoad(arena, "res/images/electric_shield.png");
	assets->texture_explosion = graphics_TextureLoad(arena, "res/images/explosion.png");
	assets->texture_bullet = graphics_TextureLoad(arena, "res/images/player_bullet.png");
	assets->texture_enemy = graphics_TextureLoad(arena, "res/images/base_enemy.png");

	arena->offset = initial_offset;
	return assets;
}

void assets_cleanup(Assets* assets)
{
	graphics_TextureDestroy(&assets->texture_player);
	graphics_TextureDestroy(&assets->texture_background);
	graphics_TextureDestroy(&assets->texture_electric_shield);
	graphics_TextureDestroy(&assets->texture_explosion);
	graphics_TextureDestroy(&assets->texture_bullet);
	graphics_TextureDestroy(&assets->texture_enemy);
}
