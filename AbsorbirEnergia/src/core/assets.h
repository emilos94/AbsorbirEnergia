#ifndef ASSETS_H
#define ASSETS_H

#include "../graphics/texture.h"

struct Assets
{
	Texture texture_background;
	Texture texture_enemy;
	Texture texture_electric_shield;
	Texture texture_player;
	Texture texture_bullet;
	Texture texture_explosion;
};
typedef struct Assets Assets;

Assets* assets_load(MemoryArena* arena);
void assets_cleanup(Assets* assets);

#endif