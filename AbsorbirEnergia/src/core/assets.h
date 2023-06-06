#ifndef ASSETS_H
#define ASSETS_H

#include "../graphics/texture.h"
#include "../ui/ui.h"
#include "../core/sound.h"

struct Assets
{
	Texture texture_background;
	Texture texture_enemy;
	Texture texture_electric_shield;
	Texture texture_player;
	Texture texture_bullet;
	Texture texture_explosion;

	UI_Font* font_candara;

	SoundSource sound_laser_shot;
};
typedef struct Assets Assets;

Assets* assets_load(MemoryArena* arena_permanent, MemoryArena* arena_temp);
void assets_cleanup(Assets* assets);

#endif