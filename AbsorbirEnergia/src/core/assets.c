#include "assets.h"


Assets* assets_load(MemoryArena* arena_permanent, MemoryArena* arena_temp)
{
	Assets* assets = memory_struct_zero_allocate(arena_permanent, Assets);

	assets->texture_player = graphics_TextureLoad(arena_temp, "res/images/player.png");
	assets->texture_background = graphics_TextureLoad(arena_temp, "res/images/background.png");
	assets->texture_electric_shield = graphics_TextureLoad(arena_temp, "res/images/electric_shield.png");
	assets->texture_explosion = graphics_TextureLoad(arena_temp, "res/images/explosion.png");
	assets->texture_bullet = graphics_TextureLoad(arena_temp, "res/images/player_bullet.png");
	assets->texture_enemy = graphics_TextureLoad(arena_temp, "res/images/base_enemy.png");

	assets->font_candara = ui_text_font_load(arena_temp, arena_permanent, "res/fonts/candara.fnt", "res/fonts/candara.png");

	assets->sound_laser_shot = sound_source_load(arena_temp, "res/sounds/laser_shot.wav");

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
	graphics_TextureDestroy(&assets->font_candara->texture);
}
